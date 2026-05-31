#pragma once

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

// ST ULD driver — files must be placed in driver/ before building
// (see driver/DRIVER_README.md)
extern "C" {
#include "driver/vl53l5cx_api.h"
}

#include <array>
#include <string>
#include <vector>

namespace esphome {
namespace vl53l5cx {

static const uint8_t NUM_ZONES_MAX = 64;  // 8x8
static const uint8_t CAL_AVERAGE_FRAMES = 10;

// Persisted to NVS via ESPHome Preferences
struct CalibrationData {
  bool valid;
  uint16_t baseline_mm[NUM_ZONES_MAX];
};

// Forward declarations for sub-platform sensor classes
class VL53L5CXZoneSensor;
class VL53L5CXGridTextSensor;
class VL53L5CXCalBinarySensor;

class VL53L5CXComponent : public PollingComponent, public i2c::I2CDevice {
 public:
  // ---- ESPHome lifecycle ------------------------------------------------
  void setup() override;
  void loop() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void dump_config() override;

  // ---- Configuration setters (called by generated code) ----------------
  void set_resolution(uint8_t res) { resolution_ = res; }
  void set_ranging_mode(uint8_t mode) { ranging_mode_ = mode; }
  void set_target_order(uint8_t order) { target_order_ = order; }

  // ---- Runtime controls (called from YAML lambdas) ---------------------
  void set_averaging_window(uint8_t frames) { avg_window_ = frames; }
  void set_operating_mode(bool averaged) { averaged_mode_ = averaged; }

  // ---- Calibration -----------------------------------------------------
  void capture_baseline();
  void clear_baseline();
  bool is_calibrated() const { return cal_data_.valid; }

  // ---- Sub-sensor registration -----------------------------------------
  void register_zone_sensor(VL53L5CXZoneSensor *s) { zone_sensors_.push_back(s); }
  void set_grid_text_sensor(VL53L5CXGridTextSensor *s) { grid_sensor_ = s; }
  void set_cal_binary_sensor(VL53L5CXCalBinarySensor *s) { cal_sensor_ = s; }

  // ---- Data accessors (used by sub-sensors) ----------------------------
  uint16_t get_distance_mm(uint8_t row, uint8_t col) const;
  int16_t  get_delta_mm(uint8_t row, uint8_t col) const;
  uint32_t get_signal_kcps(uint8_t row, uint8_t col) const;

 protected:
  bool init_sensor_();
  bool read_frame_();
  void publish_all_();
  std::string build_json_() const;
  void accumulate_frame_for_cal_();
  void finalise_baseline_capture_();
  void save_calibration_();
  void load_calibration_();
  uint8_t zone_index_(uint8_t row, uint8_t col) const { return row * resolution_ + col; }

  // ---- ST ULD device handle --------------------------------------------
  VL53L5CX_Configuration dev_{};

  // ---- Config ----------------------------------------------------------
  uint8_t resolution_{8};
  uint8_t ranging_mode_{1};  // continuous
  uint8_t target_order_{1};  // closest
  uint8_t avg_window_{10};
  bool    averaged_mode_{false};

  // ---- Live frame buffers ----------------------------------------------
  VL53L5CX_ResultsData results_{};
  uint16_t distance_mm_[NUM_ZONES_MAX]{};
  uint32_t signal_kcps_[NUM_ZONES_MAX]{};
  uint8_t  nb_targets_[NUM_ZONES_MAX]{};
  int16_t  delta_mm_[NUM_ZONES_MAX]{};

  // ---- Averaging accumulator -------------------------------------------
  uint32_t avg_acc_[NUM_ZONES_MAX]{};
  uint8_t  avg_count_{0};

  // ---- Calibration capture accumulator ---------------------------------
  bool     capturing_baseline_{false};
  uint32_t cal_acc_[NUM_ZONES_MAX]{};
  uint8_t  cal_frames_captured_{0};

  // ---- Persisted calibration -------------------------------------------
  CalibrationData cal_data_{};
  ESPPreferenceObject pref_;

  // ---- Sub-sensor references -------------------------------------------
  std::vector<VL53L5CXZoneSensor *> zone_sensors_;
  VL53L5CXGridTextSensor  *grid_sensor_{nullptr};
  VL53L5CXCalBinarySensor *cal_sensor_{nullptr};

  bool sensor_ready_{false};
};

// ---------------------------------------------------------------------------
// Sub-platform: per-zone numeric sensor
// ---------------------------------------------------------------------------
class VL53L5CXZoneSensor : public sensor::Sensor, public Component {
 public:
  void set_parent(VL53L5CXComponent *parent) { parent_ = parent; }
  void set_zone(uint8_t row, uint8_t col) { row_ = row; col_ = col; }
  void set_data_type(uint8_t type) { data_type_ = type; }
  void publish_zone_value();

 protected:
  VL53L5CXComponent *parent_{nullptr};
  uint8_t row_{0}, col_{0};
  uint8_t data_type_{0};  // 0=distance, 1=delta, 2=signal
};

// ---------------------------------------------------------------------------
// Sub-platform: full-grid JSON text sensor
// ---------------------------------------------------------------------------
class VL53L5CXGridTextSensor : public text_sensor::TextSensor, public Component {
 public:
  void set_parent(VL53L5CXComponent *parent) { parent_ = parent; }

 protected:
  VL53L5CXComponent *parent_{nullptr};
};

// ---------------------------------------------------------------------------
// Sub-platform: calibration-valid binary sensor
// ---------------------------------------------------------------------------
class VL53L5CXCalBinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void set_parent(VL53L5CXComponent *parent) { parent_ = parent; }

 protected:
  VL53L5CXComponent *parent_{nullptr};
};

}  // namespace vl53l5cx
}  // namespace esphome
