import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, STATE_CLASS_MEASUREMENT
from . import VL53L5CXComponent, CONF_VL53L5CX_ID, vl53l5cx_ns

DEPENDENCIES = ["vl53l5cx"]

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

CONFIG_SCHEMA = (
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


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

    parent = await cg.get_variable(config[CONF_VL53L5CX_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_zone(config[CONF_ZONE_ROW], config[CONF_ZONE_COL]))
    cg.add(var.set_data_type(config[CONF_DATA_TYPE]))
    cg.add(parent.register_zone_sensor(var))
