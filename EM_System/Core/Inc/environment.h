#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H

#include "stdint.h"

typedef struct{
	  int16_t temperature_x10;  // 例如 256 表示 25.6 C
    uint16_t humidity_x10;     // 例如 653 表示 65.3 %
    uint16_t light;            // ADC 原始值
    uint8_t alarm;              // 0: 正常，1: 超阈值
}EnvironmentData;

typedef struct{
		int16_t temperature_max_x10;
    uint16_t humidity_max_x10;
    uint16_t light_max;
}Threshold;

extern EnvironmentData g_env_data;
extern Threshold g_threshold;

#endif

