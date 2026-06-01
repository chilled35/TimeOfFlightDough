import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from . import VL53L5CXComponent, CONF_VL53L5CX_ID, vl53l5cx_ns

DEPENDENCIES = ["vl53l5cx"]

VL53L5CXGridTextSensor = vl53l5cx_ns.class_(
    "VL53L5CXGridTextSensor", text_sensor.TextSensor, cg.Component
)

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(VL53L5CXGridTextSensor)
    .extend({
        cv.GenerateID(CONF_VL53L5CX_ID): cv.use_id(VL53L5CXComponent),
    })
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)

    parent = await cg.get_variable(config[CONF_VL53L5CX_ID])
    cg.add(parent.set_grid_text_sensor(var))
