#ifndef __ALARM_H
#define __ALARM_H

#include "gpio.h"

#include "main.h"

#include "stm32f1xx_hal.h"

#include "environment.h"

void Alarm_Update(EnvironmentData *date,const Threshold *threshold);

#endif
