#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H

#include "stdint.h"

typedef struct
{
    int16_t temperature_x10; // 例如 256 表示 25.6 C
    uint16_t humidity_x10;   // 例如 653 表示 65.3 %
    uint16_t light;          // ADC 原始值

    uint8_t smoke_alarm; // 0: 正常，1: 检测到烟雾
    uint8_t water_alarm; // 0: 正常，1: 检测到积水

    uint8_t alarm; // 最终蜂鸣器报警状态  0: 正常，1: 超阈值
} EnvironmentData;

typedef struct
{
    int16_t fan_temp_on_x10;          // 风扇温度阈值
    int16_t high_temp_alarm_x10;      // 高温报警阈值
    uint16_t high_humidity_alarm_x10; // 高湿报警阈值
    uint16_t light_on_threshold;      // 补光开启阈值
} Threshold;

typedef enum
{
    CONTROL_MODE_AUTO = 0,
    CONTROL_MODE_MANUAL = 1
} ControlMode;

extern ControlMode g_control_mode;
extern uint8_t g_manual_fan;
extern uint8_t g_manual_light;
extern uint8_t g_manual_pump;

extern EnvironmentData g_env_data;
extern Threshold g_threshold;

#endif
