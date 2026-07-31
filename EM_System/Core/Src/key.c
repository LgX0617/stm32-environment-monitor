#include "key.h"

#define KEY_DEBOUNCE_MS 20
volatile uint8_t key_up_flag = 0;
volatile uint8_t key0_flag = 0;
volatile uint8_t key1_flag = 0;

uint32_t key_up_tick = 0;
uint32_t key0_tick = 0;
uint32_t key1_tick = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	uint32_t now = HAL_GetTick();

	if(GPIO_Pin == KEY_UP_Pin)
	{
	key_up_tick = HAL_GetTick();
		key_up_flag = 1;
	}
	else if(GPIO_Pin == KEY0_Pin)
	{
		key0_tick = HAL_GetTick();
		key0_flag = 1;
	}
	else if(GPIO_Pin == KEY1_Pin)
	{
		key1_tick = HAL_GetTick();
		key1_flag = 1;
	}
}
