#pragma once
/*
 * VL53L8CX Platform Abstraction Layer header — ESPHome / ESP32 version.
 *
 * This file must live in the component root alongside vl53l8cx_api.h.
 * Do NOT replace it with the platform.h from the ST ZIP archive.
 *
 * Include order in the ST driver:
 *   vl53l8cx_api.h
 *     -> #include "platform.h"          (this file — defines NB_TARGET_PER_ZONE)
 *     -> #include "vl53l8cx_buffers.h"  (uses NB_TARGET_PER_ZONE for buffer sizes)
 *
 * VL53L8CX_NB_TARGET_PER_ZONE must therefore be defined HERE, before
 * vl53l8cx_buffers.h is included by vl53l8cx_api.h.
 */

#include <stdint.h>
#include <string.h>

/*
 * Number of targets reported per zone.
 * 1  = closest/strongest only (default, lower RAM, faster)
 * 4  = up to 4 targets per zone (higher RAM, richer data)
 * Must be 1 or 4 per ST ULD documentation.
 */
#ifndef VL53L8CX_NB_TARGET_PER_ZONE
#define VL53L8CX_NB_TARGET_PER_ZONE    1U
#endif

/* Typedef required — ST driver C files use 'VL53L8CX_Platform' without 'struct' prefix */
typedef struct {
  uint16_t address;    /* 8-bit I2C write address (default 0x52) */
  void    *i2c_handle; /* pointer to ESPHome i2c::I2CDevice instance */
} VL53L8CX_Platform;

#ifdef __cplusplus
extern "C" {
#endif

uint8_t VL53L8CX_RdByte(VL53L8CX_Platform *p_platform, uint16_t reg, uint8_t *p_value);
uint8_t VL53L8CX_WrByte(VL53L8CX_Platform *p_platform, uint16_t reg, uint8_t value);
uint8_t VL53L8CX_RdMulti(VL53L8CX_Platform *p_platform, uint16_t reg, uint8_t *p_data, uint32_t size);
uint8_t VL53L8CX_WrMulti(VL53L8CX_Platform *p_platform, uint16_t reg, uint8_t *p_data, uint32_t size);
uint8_t VL53L8CX_Reset_Sensor(VL53L8CX_Platform *p_platform);
void    VL53L8CX_SwapBuffer(uint8_t *buffer, uint16_t size);
uint8_t VL53L8CX_WaitMs(VL53L8CX_Platform *p_platform, uint32_t TimeMs);

#ifdef __cplusplus
}
#endif
