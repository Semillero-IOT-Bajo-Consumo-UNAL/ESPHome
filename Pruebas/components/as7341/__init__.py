import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.components import i2c, sensor
from esphome.const import CONF_ID, CONF_ADDRESS

DEPENDENCIES = ["i2c"]

as7341_ns = cg.esphome_ns.namespace("as7341")

AS7341Component = as7341_ns.class_(
    "AS7341Component",
    cg.PollingComponent,
    i2c.I2CDevice,
    sensor.Sensor,
)

CONFIG_SCHEMA = (
    sensor.sensor_schema()
    .extend({
        cv.GenerateID(): cv.declare_id(AS7341Component),
    })
    .extend(cv.polling_component_schema("5s"))
    .extend(i2c.i2c_device_schema(0x39))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    await sensor.register_sensor(var, config)