import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import CONF_ID, CONF_GAIN

DEPENDENCIES = ["i2c"]

as7341_ns = cg.esphome_ns.namespace("as7341")
AS7341Component = as7341_ns.class_("AS7341Component", cg.PollingComponent, i2c.I2CDevice)

# Claves propias
CONF_INTEGRATION_STEPS = "integration_steps"
CONF_INTEGRATION_TIME  = "integration_time"
CONF_LED_CURRENT       = "led_current"
CONF_LED_ENABLED       = "led_enabled"
CONF_SENSITIVITY_FACTORS = "sensitivity_factors"
DEFAULT_FACTORS = [24.5, 12.3, 6.4, 3.5, 2.3, 1.6, 1.0, 1.3]
CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(): cv.declare_id(AS7341Component),
        cv.Optional(CONF_SENSITIVITY_FACTORS, default=DEFAULT_FACTORS):
            cv.All(
                cv.ensure_list(cv.float_),
                cv.Length(min=8, max=8)
            ),
        cv.Optional(CONF_GAIN, default=6):
            cv.int_range(min=0, max=10),
        cv.Optional(CONF_INTEGRATION_STEPS, default=100):
            cv.int_range(min=1, max=255),
        cv.Optional(CONF_INTEGRATION_TIME, default=999):
            cv.int_range(min=0, max=65534),
        cv.Optional(CONF_LED_ENABLED, default=True):
            cv.boolean,
        cv.Optional(CONF_LED_CURRENT, default=10):
            cv.int_range(min=1, max=19),
    })
    .extend(cv.polling_component_schema("5s"))
    .extend(i2c.i2c_device_schema(0x39))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_gain(config[CONF_GAIN]))
    cg.add(var.set_integration_steps(config[CONF_INTEGRATION_STEPS]))
    cg.add(var.set_integration_time(config[CONF_INTEGRATION_TIME]))
    cg.add(var.set_led_enabled(config[CONF_LED_ENABLED]))
    cg.add(var.set_led_current(config[CONF_LED_CURRENT]))
    factors = config[CONF_SENSITIVITY_FACTORS]
    for i, f in enumerate(factors):
        cg.add(var.set_sensitivity_factor(i, f))
