#ifndef __LED_H
#define __LED_H

#include "gpio.h"

#include "stm32f1xx_hal.h"

#include "main.h"

void Led_ROpen(void);

void Led_RClose(void);

void Led_GOpen(void);

void Led_GClose(void);

#endif
