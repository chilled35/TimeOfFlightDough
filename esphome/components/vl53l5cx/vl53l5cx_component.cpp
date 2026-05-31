#include "vl53l5cx_component.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace vl53l5cx {

static const char *const TAG = "vl53l5cx";

void VL53L5CXComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up VL53L5CX...");
  load_calibration_();
  if (!init_sensor_()) {
    ESP_LOGE(TAG, "Sensor init failed — check wiring and I²C address (expected 0x29)");
    mark_failed();
    return;
  }
  sensor_ready_ = true;
  ESP_LOGCONFIG(TAG, "VL53L5CX ready. Resolution: %dx%d, calibrated: %s",
                resolution_, resolution_, cal_data_.valid ? "yes" : "no");
  if (cal_sensor_) cal_sensor_->publish_state(cal_data_.valid);
}

void VL53L5CXComponent::loop() {
  if (!sensor_ready_) return;
  if (averaged_mode_ && avg_count_ < avg_window_) {
    uint8_t is_ready = 0;
    vl53l5cx_check_data_ready(&dev_, &is_ready);
    if (is_ready) { read_frame_(); avg_count_++; }
  }
}

void VL53L5CXComponent::update() {
  if (!sensor_ready_) return;
  if (averaged_mode_) {
    if (avg_count_ >= avg_window_) {
      uint8_t n = resolution_ * resolution_;
      for (uint8_t i = 0; i < n; i++)
        distance_mm_[i] = static_cast<uint16_t>(avg_acc_[i] / avg_window_);
      avg_count_ = 0;
      memset(avg_acc_, 0, sizeof(avg_acc_));
      for (uint8_t i = 0; i < n; i++) {
        delta_mm_[i] = cal_data_.valid
            ? static_cast<int16_t>(cal_data_.baseline_mm[i]) - static_cast<int16_t>(distance_mm_[i])
            : 0;
      }
      publish_all_();
    }
  } else {
    if (read_frame_()) publish_all_();
  }
}

void VL53L5CXComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "VL53L5CX:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Resolution : %dx%d", resolution_, resolution_);
  ESP_LOGCONFIG(TAG, "  Ranging    : %s", ranging_mode_ == VL53L5CX_RANGING_MODE_CONTINUOUS ? "continuous" : "autonomous");
  ESP_LOGCONFIG(TAG, "  Target     : %s", target_order_ == VL53L5CX_TARGET_ORDER_CLOSEST ? "closest" : "strongest");
  ESP_LOGCONFIG(TAG, "  Avg window : %d frames", avg_window_);
  ESP_LOGCONFIG(TAG, "  Calibrated : %s", cal_data_.valid ? "yes" : "no");
}

bool VL53L5CXComponent::init_sensor_() {
  dev_.platform.address    = static_cast<uint16_t>(get_i2c_address() << 1);
  dev_.platform.i2c_handle = this;

  uint8_t status = vl53l5cx_init(&dev_);
  if (status != VL53L5CX_STATUS_OK) { ESP_LOGE(TAG, "vl53l5cx_init failed (%u)", status); return false; }

  uint8_t res_const = (resolution_ == 8) ? VL53L5CX_RESOLUTION_8X8 : VL53L5CX_RESOLUTION_4X4;
  status = vl53l5cx_set_resolution(&dev_, res_const);
  if (status != VL53L5CX_STATUS_OK) { ESP_LOGE(TAG, "set_resolution failed (%u)", status); return false; }

  status = vl53l5cx_set_ranging_mode(&dev_, ranging_mode_);
  if (status != VL53L5CX_STATUS_OK) { ESP_LOGE(TAG, "set_ranging_mode failed (%u)", status); return false; }

  status = vl53l5cx_set_target_order(&dev_, target_order_);
  if (status != VL53L5CX_STATUS_OK) { ESP_LOGE(TAG, "set_target_order failed (%u)", status); return false; }

  status = vl53l5cx_set_ranging_frequency_hz(&dev_, 10);
  if (status != VL53L5CX_STATUS_OK) { ESP_LOGE(TAG, "set_ranging_frequency_hz failed (%u)", status); return false; }

  status = vl53l5cx_start_ranging(&dev_);
  if (status != VL53L5CX_STATUS_OK) { ESP_LOGE(TAG, "start_ranging failed (%u)", status); return false; }

  return true;
}

bool VL53L5CXComponent::read_frame_() {
  uint8_t is_ready = 0;
  vl53l5cx_check_data_ready(&dev_, &is_ready);
  if (!is_ready) return false;

  uint8_t status = vl53l5cx_get_ranging_data(&dev_, &results_);
  if (status != VL53L5CX_STATUS_OK) { ESP_LOGW(TAG, "get_ranging_data failed (%u)", status); return false; }

  uint8_t n = resolution_ * resolution_;
  for (uint8_t i = 0; i < n; i++) {
    distance_mm_[i] = results_.distance_mm[VL53L5CX_NB_TARGET_PER_ZONE * i];
    signal_kcps_[i] = results_.signal_per_spad[VL53L5CX_NB_TARGET_PER_ZONE * i];
    nb_targets_[i]  = results_.nb_target_detected[i];
  }
  for (uint8_t i = 0; i < n; i++) {
    delta_mm_[i] = cal_data_.valid
        ? static_cast<int16_t>(cal_data_.baseline_mm[i]) - static_cast<int16_t>(distance_mm_[i])
        : 0;
  }
  if (capturing_baseline_) accumulate_frame_for_cal_();
  if (averaged_mode_) {
    for (uint8_t i = 0; i < n; i++) avg_acc_[i] += distance_mm_[i];
  }
  return true;
}

void VL53L5CXComponent::publish_all_() {
  if (grid_sensor_) grid_sensor_->publish_state(build_json_());
  for (auto *zs : zone_sensors_) zs->publish_zone_value();
}

std::string VL53L5CXComponent::build_json_() const {
  uint8_t n = resolution_ * resolution_;

  int64_t ts = 0;
  if (time_ != nullptr) {
    auto now = time_->now();
    if (now.is_valid()) ts = now.timestamp;
  }

  std::string out;
  out.reserve(768);
  out += "{\"ts\":";
  out += std::to_string(ts);
  out += ",\"res\":";
  out += std::to_string(resolution_);
  out += ",\"mode\":\"";
  out += averaged_mode_ ? "averaged" : "live";
  out += '"';

  auto append_array = [&](const char *key, auto *arr) {
    out += ",\"";
    out += key;
    out += \"\\":[\"";
    for (uint8_t i = 0; i < n; i++) {
      if (i) out += ',';
      out += std::to_string(arr[i]);
    }
    out += ']';
  };

  append_array("distances_mm", distance_mm_);
  append_array("signal_kcps",  signal_kcps_);
  append_array("nb_targets",   nb_targets_);
  append_array("delta_mm",     delta_mm_);

  out += ",\"cal_valid\":";
  out += cal_data_.valid ? "true" : "false";
  out += '}';
  return out;
}

void VL53L5CXComponent::capture_baseline() {
  ESP_LOGI(TAG, "Starting baseline capture (%d frames)", CAL_AVERAGE_FRAMES);
  capturing_baseline_  = true;
  cal_frames_captured_ = 0;
  memset(cal_acc_, 0, sizeof(cal_acc_));
}

void VL53L5CXComponent::accumulate_frame_for_cal_() {
  uint8_t n = resolution_ * resolution_;
  for (uint8_t i = 0; i < n; i++) cal_acc_[i] += distance_mm_[i];
  cal_frames_captured_++;
  ESP_LOGD(TAG, "Cal frame %d/%d", cal_frames_captured_, CAL_AVERAGE_FRAMES);
  if (cal_frames_captured_ >= CAL_AVERAGE_FRAMES) finalise_baseline_capture_();
}

void VL53L5CXComponent::finalise_baseline_capture_() {
  capturing_baseline_ = false;
  uint8_t n = resolution_ * resolution_;
  for (uint8_t i = 0; i < n; i++)
    cal_data_.baseline_mm[i] = static_cast<uint16_t>(cal_acc_[i] / CAL_AVERAGE_FRAMES);
  cal_data_.valid = true;
  save_calibration_();
  if (cal_sensor_) cal_sensor_->publish_state(true);
  ESP_LOGI(TAG, "Baseline captured and saved to NVS");
}

void VL53L5CXComponent::clear_baseline() {
  cal_data_.valid = false;
  memset(cal_data_.baseline_mm, 0, sizeof(cal_data_.baseline_mm));
  save_calibration_();
  if (cal_sensor_) cal_sensor_->publish_state(false);
  ESP_LOGI(TAG, "Calibration cleared");
}

void VL53L5CXComponent::save_calibration_() {
  pref_ = global_preferences->make_preference<CalibrationData>(
      this->get_object_id_hash() ^ 0xCAL1B00U);
  pref_.save(&cal_data_);
}

void VL53L5CXComponent::load_calibration_() {
  pref_ = global_preferences->make_preference<CalibrationData>(
      this->get_object_id_hash() ^ 0xCAL1B00U);
  if (!pref_.load(&cal_data_)) {
    cal_data_.valid = false;
    ESP_LOGD(TAG, "No saved calibration found");
  } else {
    ESP_LOGI(TAG, "Loaded calibration from NVS (valid=%s)", cal_data_.valid ? "yes" : "no");
  }
}

uint16_t VL53L5CXComponent::get_distance_mm(uint8_t row, uint8_t col) const {
  return distance_mm_[zone_index_(row, col)];
}
int16_t VL53L5CXComponent::get_delta_mm(uint8_t row, uint8_t col) const {
  return delta_mm_[zone_index_(row, col)];
}
uint32_t VL53L5CXComponent::get_signal_kcps(uint8_t row, uint8_t col) const {
  return signal_kcps_[zone_index_(row, col)];
}

void VL53L5CXZoneSensor::publish_zone_value() {
  if (!parent_) return;
  float val = 0.0f;
  switch (data_type_) {
    case 0: val = static_cast<float>(parent_->get_distance_mm(row_, col_)); break;
    case 1: val = static_cast<float>(parent_->get_delta_mm(row_, col_));    break;
    case 2: val = static_cast<float>(parent_->get_signal_kcps(row_, col_)); break;
  }
  publish_state(val);
}

}  // namespace vl53l5cx
}  // namespace esphome
