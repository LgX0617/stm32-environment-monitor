#include "water.h"

uint8_t Water_IsAlarm(void)
{
	return HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == RESET;
}
