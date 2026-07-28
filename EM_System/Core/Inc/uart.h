#ifndef __UART_H
#define __UART_H

#include "main.h"

#include "usart.h"

#include "stm32f1xx_hal.h"

#include "environment.h"

void SensorData_Send(EnvironmentData *data);

void UART_RxStart(void);

uint8_t UART_ProcessCommand(Threshold *threshold);

#endif


