import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.automation import maybe_simple_id
from esphome.components import sensor, uart
from esphome.const import (
    CONF_CO2,
    CONF_ID,
    DEVICE_CLASS_CARBON_DIOXIDE,
    STATE_CLASS_MEASUREMENT,
    UNIT_PARTS_PER_MILLION,
)

CODEOWNERS = ["@lyj0309"]
DEPENDENCIES = ["uart"]

ICON_MOLECULE_CO2 = "mdi:molecule-co2"

jx_co2_102_ns = cg.esphome_ns.namespace("jx_co2_102")
JXCO2102Sensor = jx_co2_102_ns.class_(
    "JXCO2102Sensor", cg.PollingComponent, uart.UARTDevice
)
JXCO2102CalibrateZeroAction = jx_co2_102_ns.class_(
    "JXCO2102CalibrateZeroAction",
    automation.Action,
)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(JXCO2102Sensor),
            cv.Optional(CONF_CO2): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_MOLECULE_CO2,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_CARBON_DIOXIDE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_CO2 in config:
        sens = await sensor.new_sensor(config[CONF_CO2])
        cg.add(var.set_co2_sensor(sens))


CALIBRATION_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.GenerateID(): cv.use_id(JXCO2102Sensor),
    }
)


@automation.register_action(
    "jx_co2_102.calibrate_zero",
    JXCO2102CalibrateZeroAction,
    CALIBRATION_ACTION_SCHEMA,
)
async def jx_co2_102_calibration_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)
