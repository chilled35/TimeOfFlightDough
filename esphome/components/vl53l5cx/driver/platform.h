#pragma once
/*
 * VL53L5CX Platform Abstraction Layer header.
 *
 * This file satisfies the ST ULD's #include "platform.h" requirement.
 * The actual I2C calls are implemented in vl53l5cx_platform_esp.cpp
 * in the parent directory, using ESPHome's i2c::I2CDevice.
 *
 * Do NOT replace this file with the version from the ST ZIP archive.
 */

#include <stdint.h>
#include <string.h>

/* Forward declaration — the ESP component provides this struct */
struct VL53L5CX_Platform {
  uint16_t address;   /* 8-bit I2C write address (default 0x52) */
  void    *i2c_handle; /* pointer to ESPHome i2c::I2CDevice instance */
};

/* ST ULD calls these — implemented in vl53l5cx_platform_esp.cpp */
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
