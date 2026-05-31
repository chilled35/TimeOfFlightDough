#pragma once
/*
 * VL53L5CX Platform Abstraction Layer header — ESPHome / ESP32 version.
 *
 * This file must live in the component root alongside vl53l5cx_api.h and
 * the other ST driver files. Do NOT replace it with the platform.h from
 * the ST ZIP archive.
 */

#include <stdint.h>
#include <string.h>

struct VL53L5CX_Platform {
  uint16_t address;    /* 8-bit I2C write address (default 0x52) */
  void    *i2c_handle; /* pointer to ESPHome i2c::I2CDevice instance */
};

#ifdef __cplusplus
extern "C" {
#endif

uint8_t VL53L5CX_RdByte(VL53L5CX_Platform *p_platform, uint16_t reg, uint8_t *p_value);
uint8_t VL53L5CX_WrByte(VL53L5CX_Platform *p_platform, uint16_t reg, uint8_t value);
uint8_t VL53L5CX_RdMulti(VL53L5CX_Platform *p_platform, uint16_t reg, uint8_t *p_data, uint32_t size);
uint8_t VL53L5CX_WrMulti(VL53L5CX_Platform *p_platform, uint16_t reg, uint8_t *p_data, uint32_t size);
uint8_t VL53L5CX_Reset_Sensor(VL53L5CX_Platform *p_platform);
void    VL53L5CX_SwapBuffer(uint8_t *buffer, uint16_t size);
uint8_t VL53L5CX_WaitMs(VL53L5CX_Platform *p_platform, uint32_t TimeMs);

#ifdef __cplusplus
}
#endif
