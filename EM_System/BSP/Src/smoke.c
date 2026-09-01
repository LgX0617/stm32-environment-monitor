#include "smoke.h"
#include "environment.h"

uint8_t Smoke_IsAlarm(void)
{
	 return HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET;
}
