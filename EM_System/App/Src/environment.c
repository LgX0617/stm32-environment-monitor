#include "environment.h"
#include "control.h"

EnvironmentData g_env_data = {0};

Threshold g_threshold = 
{
	.fan_temp_on_x10 = DEFAULT_FAN_TEMP_ON_X10,
    .high_temp_alarm_x10 = DEFAULT_HIGH_TEMP_ALARM_X10,
    .fan_humidity_on_x10 = DEFAULT_FAN_HUMI_ON_X10,
    .high_humidity_alarm_x10 = DEFAULT_HIGH_HUMI_ALARM_X10,
    .light_on_threshold = DEFAULT_LIGHT_ON_THRESHOLD,
};
