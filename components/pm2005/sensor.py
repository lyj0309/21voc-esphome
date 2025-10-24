import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    CONF_PM_2_5,
    CONF_PM_10_0,
    STATE_CLASS_MEASUREMENT,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
)

CODEOWNERS = ["@lyj0309"]
DEPENDENCIES = ["uart"]

# Define custom constants
CONF_PM_0_5 = "pm_0_5"
CONF_PM_2_5_MASS = "pm_2_5_mass"
CONF_PM_10_0_MASS = "pm_10_0_mass"
ICON_CHEMICAL_WEAPON = "mdi:chemical-weapon"

pm2005_ns = cg.esphome_ns.namespace("pm2005")
PM2005Sensor = pm2005_ns.class_(
    "PM2005Sensor", uart.UARTDevice, cg.Component
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PM2005Sensor),
            cv.Optional(CONF_PM_0_5): sensor.sensor_schema(
                unit_of_measurement="PCS/L",
                icon=ICON_CHEMICAL_WEAPON,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PM_2_5): sensor.sensor_schema(
                unit_of_measurement="PCS/L",
                icon=ICON_CHEMICAL_WEAPON,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PM_10_0): sensor.sensor_schema(
                unit_of_measurement="PCS/L",
                icon=ICON_CHEMICAL_WEAPON,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PM_2_5_MASS): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
                icon=ICON_CHEMICAL_WEAPON,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PM_10_0_MASS): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
                icon=ICON_CHEMICAL_WEAPON,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_PM_0_5 in config:
        sens = await sensor.new_sensor(config[CONF_PM_0_5])
        cg.add(var.set_pm_0_5_sensor(sens))

    if CONF_PM_2_5 in config:
        sens = await sensor.new_sensor(config[CONF_PM_2_5])
        cg.add(var.set_pm_2_5_sensor(sens))

    if CONF_PM_10_0 in config:
        sens = await sensor.new_sensor(config[CONF_PM_10_0])
        cg.add(var.set_pm_10_0_sensor(sens))

    if CONF_PM_2_5_MASS in config:
        sens = await sensor.new_sensor(config[CONF_PM_2_5_MASS])
        cg.add(var.set_pm_2_5_mass_sensor(sens))

    if CONF_PM_10_0_MASS in config:
        sens = await sensor.new_sensor(config[CONF_PM_10_0_MASS])
        cg.add(var.set_pm_10_0_mass_sensor(sens))
