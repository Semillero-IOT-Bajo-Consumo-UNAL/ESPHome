import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_X,
    CONF_Y,
    UNIT_LUX,
    UNIT_KELVIN,
    DEVICE_CLASS_ILLUMINANCE,
    STATE_CLASS_MEASUREMENT,
)

DEPENDENCIES = ["i2c"]

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
CONF_RA = "ra"
CONF_CLA = "CLa"
CONF_CS = "CS"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS7341Component),
            cv.Optional(CONF_LUX): sensor.sensor_schema(
                unit_of_measurement=UNIT_LUX,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_ILLUMINANCE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MEDI): sensor.sensor_schema(
                unit_of_measurement=UNIT_LUX,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CCT): sensor.sensor_schema(
                unit_of_measurement=UNIT_KELVIN,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DUV): sensor.sensor_schema(
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_X): sensor.sensor_schema(
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_Y): sensor.sensor_schema(
                accuracy_decimals=3,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_RA): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CLA): sensor.sensor_schema(
                            accuracy_decimals=1,
                            state_class=STATE_CLASS_MEASUREMENT,
                        ),
            cv.Optional(CONF_CS): sensor.sensor_schema(
                            accuracy_decimals=3,
                            state_class=STATE_CLASS_MEASUREMENT,
                        ),
        }
    )
    .extend(i2c.i2c_device_schema(0x39))
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if lux_config := config.get(CONF_LUX):
        sens = await sensor.new_sensor(lux_config)
        cg.add(var.set_lux_sensor(sens))

    if medi_config := config.get(CONF_MEDI):
        sens = await sensor.new_sensor(medi_config)
        cg.add(var.set_medi_sensor(sens))

    if cct_config := config.get(CONF_CCT):
        sens = await sensor.new_sensor(cct_config)
        cg.add(var.set_cct_sensor(sens))

    if duv_config := config.get(CONF_DUV):
        sens = await sensor.new_sensor(duv_config)
        cg.add(var.set_duv_sensor(sens))

    if x_config := config.get(CONF_X):
        sens = await sensor.new_sensor(x_config)
        cg.add(var.set_x_sensor(sens))

    if y_config := config.get(CONF_Y):
        sens = await sensor.new_sensor(y_config)
        cg.add(var.set_y_sensor(sens))

    if ra_config := config.get(CONF_RA):
        sens = await sensor.new_sensor(ra_config)
        cg.add(var.set_ra_sensor(sens))
    if cla_config := config.get(CONF_CLA):
            sens = await sensor.new_sensor(cla_config)
            cg.add(var.set_cla_sensor(sens))
    if cs_config := config.get(CONF_CS):
            sens = await sensor.new_sensor(cs_config)
            cg.add(var.set_cs_sensor(sens))