import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID
from . import VL53L5CXComponent, CONF_VL53L5CX_ID, vl53l5cx_ns

DEPENDENCIES = ["vl53l5cx"]

VL53L5CXCalBinarySensor = vl53l5cx_ns.class_(
    "VL53L5CXCalBinarySensor", binary_sensor.BinarySensor, cg.Component
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(VL53L5CXCalBinarySensor)
    .extend({
        cv.GenerateID(CONF_VL53L5CX_ID): cv.use_id(VL53L5CXComponent),
    })
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await binary_sensor.register_binary_sensor(var, config)

    parent = await cg.get_variable(config[CONF_VL53L5CX_ID])
    cg.add(parent.set_cal_binary_sensor(var))
