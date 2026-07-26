#ifndef __UART_H
#define __UART_H

#include "main.h"

#include "usart.h"

#include "stm32f1xx_hal.h"

#include "environment.h"

void SensorData_Send(EnvironmentData *data);

#endif


