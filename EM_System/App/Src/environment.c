#include "environment.h"
#include "control.h"

EnvironmentData g_env_data = {0};
ControlMode g_control_mode = CONTROL_MODE_AUTO;

Threshold g_threshold =
    {
        .fan_temp_on_x10 = DEFAULT_FAN_TEMP_ON_X10,
        .high_temp_alarm_x10 = DEFAULT_HIGH_TEMP_ALARM_X10,
        .high_humidity_alarm_x10 = DEFAULT_HIGH_HUMI_ALARM_X10,
        .light_on_threshold = DEFAULT_LIGHT_ON_THRESHOLD,
};

uint8_t g_manual_fan = 0;
uint8_t g_manual_light = 0;
uint8_t g_manual_pump = 0;
