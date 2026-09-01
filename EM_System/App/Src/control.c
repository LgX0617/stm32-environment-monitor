#include "control.h"
#include "led.h"
#include "motor.h"
#include "water.h"

//控制照明灯开关
void Light_Control_Update(const EnvironmentData* data,const Threshold* threshold)
{
	if(data->light >= threshold->light_on_threshold)
	{
		Led_GOpen();
	}
	else if(data->light <= DEFAULT_LIGHT_OFF_THRESHOLD)
	{
		Led_GClose();
	}
}

//控制风扇开关
void Fan_Control_Update(const EnvironmentData* data,const Threshold* threshold)
{
	if((data->temperature_x10 >= threshold->fan_temp_on_x10) || 
	data->smoke_alarm == SET)
	{
		Fan_On();
	}
	else if((data->temperature_x10 <= (threshold->fan_temp_on_x10 - 20)) && 
		data->smoke_alarm == RESET)
	{
		Fan_Off();
	}
}

//控制水泵开关
void Water_Control_Update(void)
{
	if(Water_IsAlarm())
	{
		Pump_On();
	}
	else 
	{
		Pump_Off();
	}
}

