import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, time
from esphome.const import CONF_ID, CONF_TIME_ID

DEPENDENCIES = ["i2c", "json"]
AUTO_LOAD = ["sensor", "text_sensor", "binary_sensor"]
MULTI_CONF = False

CONF_VL53L5CX_ID  = "vl53l5cx_id"
CONF_RESOLUTION   = "resolution"
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
    "closest":   cg.uint8(1),
    "strongest": cg.uint8(2),
}

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(VL53L5CXComponent),
            cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
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

    if CONF_TIME_ID in config:
        time_var = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time(time_var))
