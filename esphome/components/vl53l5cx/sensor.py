"""
VL53L5CX sub-platform sensors.

Provides three sub-platform types under the `sensor:` and `text_sensor:`
and `binary_sensor:` keys in the YAML:

  sensor:
    - platform: vl53l5cx      # per-zone numeric (distance_mm or delta_mm)
      zone_row: 0-7
      zone_col: 0-7
      data_type: distance_mm | delta_mm | signal_kcps

  text_sensor:
    - platform: vl53l5cx      # full 8x8 grid as JSON string

  binary_sensor:
    - platform: vl53l5cx      # calibration_valid flag
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, text_sensor, binary_sensor
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_ICON,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_ACCURACY_DECIMALS,
    STATE_CLASS_MEASUREMENT,
    DEVICE_CLASS_DISTANCE,
)
from . import (
    VL53L5CXComponent,
    CONF_VL53L5CX_ID,
    vl53l5cx_ns,
)

CONF_ZONE_ROW  = "zone_row"
CONF_ZONE_COL  = "zone_col"
CONF_DATA_TYPE = "data_type"

DATA_TYPES = {
    "distance_mm": 0,
    "delta_mm":    1,
    "signal_kcps": 2,
}

VL53L5CXZoneSensor = vl53l5cx_ns.class_(
    "VL53L5CXZoneSensor", sensor.Sensor, cg.Component
)
VL53L5CXGridTextSensor = vl53l5cx_ns.class_(
    "VL53L5CXGridTextSensor", text_sensor.TextSensor, cg.Component
)
VL53L5CXCalBinarySensor = vl53l5cx_ns.class_(
    "VL53L5CXCalBinarySensor", binary_sensor.BinarySensor, cg.Component
)

# ---- numeric zone sensor -----------------------------------------------
SENSOR_SCHEMA = (
    sensor.sensor_schema(
        VL53L5CXZoneSensor,
        unit_of_measurement="mm",
        accuracy_decimals=0,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend({
        cv.GenerateID(CONF_VL53L5CX_ID): cv.use_id(VL53L5CXComponent),
        cv.Required(CONF_ZONE_ROW): cv.int_range(min=0, max=7),
        cv.Required(CONF_ZONE_COL): cv.int_range(min=0, max=7),
        cv.Optional(CONF_DATA_TYPE, default="distance_mm"): cv.enum(
            DATA_TYPES, lower=True
        ),
    })
    .extend(cv.COMPONENT_SCHEMA)
)


async def sensor_to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

    parent = await cg.get_variable(config[CONF_VL53L5CX_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_zone(config[CONF_ZONE_ROW], config[CONF_ZONE_COL]))
    cg.add(var.set_data_type(config[CONF_DATA_TYPE]))
    cg.add(parent.register_zone_sensor(var))


# ---- grid text sensor --------------------------------------------------
TEXT_SENSOR_SCHEMA = (
    text_sensor.text_sensor_schema(VL53L5CXGridTextSensor)
    .extend({
        cv.GenerateID(CONF_VL53L5CX_ID): cv.use_id(VL53L5CXComponent),
    })
    .extend(cv.COMPONENT_SCHEMA)
)


async def text_sensor_to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)

    parent = await cg.get_variable(config[CONF_VL53L5CX_ID])
    cg.add(parent.set_grid_text_sensor(var))


# ---- calibration binary sensor -----------------------------------------
BINARY_SENSOR_SCHEMA = (
    binary_sensor.binary_sensor_schema(VL53L5CXCalBinarySensor)
    .extend({
        cv.GenerateID(CONF_VL53L5CX_ID): cv.use_id(VL53L5CXComponent),
    })
    .extend(cv.COMPONENT_SCHEMA)
)


async def binary_sensor_to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await binary_sensor.register_binary_sensor(var, config)

    parent = await cg.get_variable(config[CONF_VL53L5CX_ID])
    cg.add(parent.set_cal_binary_sensor(var))
