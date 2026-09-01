/**
蜂鸣器报警
*/

#include "alarm.h"

void Alarm_Update(EnvironmentData *data,const Threshold *threshold){
	if((data->temperature_x10 > threshold->high_temp_alarm_x10) || 
			(data->humidity_x10 > threshold->high_humidity_alarm_x10) ||
			(data->smoke_alarm) || (data->water_alarm)
	    )
			{
				data->alarm = 1;
				HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_SET);
				Led_ROpen();
			}
			else {
				data->alarm = 0;
				HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,GPIO_PIN_RESET);
				Led_RClose();
			}
		
}

