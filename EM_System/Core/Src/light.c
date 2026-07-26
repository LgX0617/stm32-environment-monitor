#include "light.h"

uint16_t adc_value;

uint16_t Light_Read(){
	HAL_ADC_Start(&hadc3);
	HAL_ADC_PollForConversion(&hadc3,HAL_MAX_DELAY);
		adc_value = HAL_ADC_GetValue(&hadc3);
	HAL_ADC_Stop(&hadc3);
		return adc_value;
}

