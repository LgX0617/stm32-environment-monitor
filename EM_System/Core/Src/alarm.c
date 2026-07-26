/**
蜂鸣器报警
*/

#include "alarm.h"

void Alarm_Update(EnvironmentData *data,const Threshold *threshold){
	if((data->temperature_x10 > threshold->temperature_max_x10) || 
			(data->humidity_x10 > threshold->humidity_max_x10) ||
	    (data->light > threshold->light_max))
			{
				data->alarm = 1;
				HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
			}
			else {
				data->alarm = 0;
				HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
			}
		
}

