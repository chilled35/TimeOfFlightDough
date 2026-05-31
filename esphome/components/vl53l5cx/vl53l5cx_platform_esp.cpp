/*
 * VL53L5CX Platform Abstraction Layer — ESPHome / ESP32 implementation.
 */

#include "platform.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esp_task_wdt.h"

#include <vector>

using namespace esphome::i2c;

static const char *const TAG_PAL = "vl53l5cx_pal";

static inline I2CDevice *dev(VL53L5CX_Platform *p) {
  return reinterpret_cast<I2CDevice *>(p->i2c_handle);
}

static void write_reg_addr(uint8_t *buf, uint16_t reg) {
  buf[0] = static_cast<uint8_t>(reg >> 8);
  buf[1] = static_cast<uint8_t>(reg & 0xFF);
}

uint8_t VL53L5CX_WrByte(VL53L5CX_Platform *p, uint16_t reg, uint8_t value) {
  uint8_t buf[3];
  write_reg_addr(buf, reg);
  buf[2] = value;
  auto err = dev(p)->write(buf, 3);
  return (err == ErrorCode::NO_ERROR) ? 0 : 1;
}

uint8_t VL53L5CX_RdByte(VL53L5CX_Platform *p, uint16_t reg, uint8_t *value) {
  uint8_t addr[2];
  write_reg_addr(addr, reg);
  auto err = dev(p)->write(addr, 2);
  if (err != ErrorCode::NO_ERROR) return 1;
  err = dev(p)->read(value, 1);
  return (err == ErrorCode::NO_ERROR) ? 0 : 1;
}

uint8_t VL53L5CX_WrMulti(VL53L5CX_Platform *p, uint16_t reg, uint8_t *data, uint32_t size) {
  // Write in 512-byte chunks. A single 84 KB I2C transaction may exceed the
  // ESPHome driver's transfer timeout or internal buffer limits, silently
  // delivering corrupted firmware to the sensor.
  const uint32_t CHUNK = 512;
  uint8_t buf[CHUNK + 2];
  uint32_t offset = 0;

  while (offset < size) {
    uint32_t n = std::min(CHUNK, size - offset);
    uint16_t chunk_reg = static_cast<uint16_t>(reg + offset);
    buf[0] = static_cast<uint8_t>(chunk_reg >> 8);
    buf[1] = static_cast<uint8_t>(chunk_reg & 0xFF);
    memcpy(buf + 2, data + offset, n);
    auto err = dev(p)->write(buf, n + 2);
    if (err != ErrorCode::NO_ERROR) return 1;
    esp_task_wdt_reset();
    offset += n;
  }
  return 0;
}

uint8_t VL53L5CX_RdMulti(VL53L5CX_Platform *p, uint16_t reg, uint8_t *data, uint32_t size) {
  uint8_t addr[2];
  write_reg_addr(addr, reg);
  auto err = dev(p)->write(addr, 2);
  if (err != ErrorCode::NO_ERROR) return 1;
  err = dev(p)->read(data, size);
  return (err == ErrorCode::NO_ERROR) ? 0 : 1;
}

uint8_t VL53L5CX_Reset_Sensor(VL53L5CX_Platform *p) {
  (void)p;
  return 0;
}

void VL53L5CX_SwapBuffer(uint8_t *buffer, uint16_t size) {
  for (uint16_t i = 0; i < size; i += 4) {
    uint8_t tmp;
    tmp = buffer[i];     buffer[i]     = buffer[i + 3]; buffer[i + 3] = tmp;
    tmp = buffer[i + 1]; buffer[i + 1] = buffer[i + 2]; buffer[i + 2] = tmp;
  }
}

uint8_t VL53L5CX_WaitMs(VL53L5CX_Platform *p, uint32_t ms) {
  (void)p;
  esphome::delay(ms);
  return 0;
}
