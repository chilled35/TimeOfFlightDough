"""
VL53L5CX ESPHome custom component.

Wraps the STMicroelectronics Ultra Lite Driver (ULD) for the VL53L5CX
8x8 Time-of-Flight sensor. Publishes zone distance data as JSON via a
text sensor, exposes per-zone numeric sensors, and manages baseline
calibration stored in ESP32 NVS (ESPHome Preferences).

ST ULD download: https://www.st.com/en/embedded-software/stsw-img023.html
Place the extracted driver C files under components/vl53l5cx/driver/
(see driver/DRIVER_README.md for the exact file list required).
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import (
    CONF_ID,
    CONF_UPDATE_INTERVAL,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor", "text_sensor", "binary_sensor"]
MULTI_CONF = False

# Shared constant used by sub-platforms (sensor.py) to reference the parent
CONF_VL53L5CX_ID = "vl53l5cx_id"
CONF_RESOLUTION = "resolution"
CONF_RANGING_MODE = "ranging_mode"
CONF_TARGET_ORDER = "target_order"

vl53l5cx_ns = cg.esphome_ns.namespace("vl53l5cx")
VL53L5CXComponent = vl53l5cx_ns.class_(
    "VL53L5CXComponent", cg.PollingComponent, i2c.I2CDevice
)

RANGING_MODES = {
    "continuous": cg.uint8(1),
    "autonomous": cg.uint8(3),
}

TARGET_ORDERS = {
    "closest":  cg.uint8(1),
    "strongest": cg.uint8(2),
}

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(VL53L5CXComponent),
            cv.Optional(CONF_RESOLUTION, default=8): cv.one_of(4, 8, int=True),
            cv.Optional(CONF_RANGING_MODE, default="continuous"): cv.enum(
                RANGING_MODES, lower=True
            ),
            cv.Optional(CONF_TARGET_ORDER, default="closest"): cv.enum(
                TARGET_ORDERS, lower=True
            ),
        }
    )
    .extend(cv.polling_component_schema("500ms"))
    .extend(i2c.i2c_device_schema(0x29))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_resolution(config[CONF_RESOLUTION]))
    cg.add(var.set_ranging_mode(config[CONF_RANGING_MODE]))
    cg.add(var.set_target_order(config[CONF_TARGET_ORDER]))
