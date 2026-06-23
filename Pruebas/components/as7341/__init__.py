import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.components import i2c, sensor
from esphome.const import CONF_ID, CONF_ADDRESS

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor"]

as7341_ns = cg.esphome_ns.namespace("as7341")

AS7341Component = as7341_ns.class_(
    "AS7341Component",
    cg.PollingComponent,
    i2c.I2CDevice,
)

CONF_LUX = "lux"
CONF_MEDI = "medi"
CONF_CCT = "cct"
CONF_DUV = "duv"
CONF_CIE_X = "cie_x"
CONF_CIE_Y = "cie_y"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(AS7341Component),
    cv.Optional(CONF_LUX): sensor.sensor_schema(unit_of_measurement="lx", icon="mdi:brightness-5", accuracy_decimals=2),
    cv.Optional(CONF_MEDI): sensor.sensor_schema(unit_of_measurement="lx", icon="mdi:eye", accuracy_decimals=2),
    cv.Optional(CONF_CCT): sensor.sensor_schema(unit_of_measurement="K", icon="mdi:temperature-kelvin", accuracy_decimals=0),
    cv.Optional(CONF_DUV): sensor.sensor_schema(unit_of_measurement="", icon="mdi:palette", accuracy_decimals=3),
    cv.Optional(CONF_CIE_X): sensor.sensor_schema(unit_of_measurement="", icon="mdi:coordinate", accuracy_decimals=3),
    cv.Optional(CONF_CIE_Y): sensor.sensor_schema(unit_of_measurement="", icon="mdi:coordinate", accuracy_decimals=3),
}).extend(cv.polling_component_schema("5s")).extend(i2c.i2c_device_schema(0x39))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_LUX in config:
        sens = await sensor.new_sensor(config[CONF_LUX])
        cg.add(var.set_lux_sensor(sens))
    if CONF_MEDI in config:
        sens = await sensor.new_sensor(config[CONF_MEDI])
        cg.add(var.set_medi_sensor(sens))
    if CONF_CCT in config:
        sens = await sensor.new_sensor(config[CONF_CCT])
        cg.add(var.set_cct_sensor(sens))
    if CONF_DUV in config:
        sens = await sensor.new_sensor(config[CONF_DUV])
        cg.add(var.set_duv_sensor(sens))
    if CONF_CIE_X in config:
        sens = await sensor.new_sensor(config[CONF_CIE_X])
        cg.add(var.set_x_sensor(sens))
    if CONF_CIE_Y in config:
        sens = await sensor.new_sensor(config[CONF_CIE_Y])
        cg.add(var.set_y_sensor(sens))