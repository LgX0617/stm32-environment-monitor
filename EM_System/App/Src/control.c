#include "control.h"
#include "led.h"
#include "motor.h"
#include "water.h"

// 控制照明灯开关
void Light_Control_Update(const EnvironmentData *data, const Threshold *threshold)
{
	if (data->light >= threshold->light_on_threshold)
	{
		Led_GOpen();
	}
	else if (data->light <= DEFAULT_LIGHT_OFF_THRESHOLD)
	{
		Led_GClose();
	}
}

// //控制风扇开关
// void Fan_Control_Update(const EnvironmentData* data,const Threshold* threshold)
// {
// 	if((data->temperature_x10 >= threshold->fan_temp_on_x10) ||
// 	data->smoke_alarm == SET)
// 	{
// 		Fan_On();
// 	}
// 	else if((data->temperature_x10 <= (threshold->fan_temp_on_x10 - 20)) &&
// 		data->smoke_alarm == RESET)
// 	{
// 		Fan_Off();
// 	}
// }

// //控制水泵开关
// void Water_Control_Update(void)
// {
// 	if(Water_IsAlarm())
// 	{
// 		Pump_On();
// 	}
// 	else
// 	{
// 		Pump_Off();
// 	}
// }

void Device_Control_Update(const EnvironmentData *data, const Threshold *threshold)
{
	static uint8_t fan_auto_state = 0; // 保存自动模式下的回差状态
	uint8_t fan_required = 0;
	uint8_t pump_required = 0;

	// 安全保护
	if (data->smoke_alarm || data->temperature_x10 >= threshold->high_temp_alarm_x10)
	{
		fan_auto_state = 1;
	}
	if (data->water_alarm)
	{
		pump_required = 1;
	}

	// 自动模式
	if (g_control_mode == CONTROL_MODE_AUTO)
	{
		// 温度达到风扇开启阈值
		if (data->temperature_x10 >= threshold->fan_temp_on_x10)
		{
			fan_auto_state = 1;
		}
		// 温度低于风扇关闭阈值且没有烟雾报警
		if (data->temperature_x10 <= (threshold->fan_temp_on_x10 - 20) && data->smoke_alarm == 0U)
		{
			fan_auto_state = 0;
		}
		fan_required = fan_auto_state;

		// 没有积水时关闭水泵
		if (!data->water_alarm)
		{
			pump_required = 0;
		}

		// 判断是否开启灯光
		Light_Control_Update(data, threshold);
	}

	// 手动模式
	else if (g_control_mode == CONTROL_MODE_MANUAL)
	{
		if (g_manual_fan)
		{
			fan_required = 1;
		}
		if (g_manual_pump)
		{
			pump_required = 1;
		}
		if (g_manual_light)
		{
			Led_GOpen();
		}
		else
		{
			Led_GClose();
		}
	}

	if (data->smoke_alarm || data->temperature_x10 >= threshold->high_temp_alarm_x10)
	{
		fan_required = 1;
	}

	// 根据需求控制风扇和水泵
	if (fan_required)
	{
		Fan_On();
	}
	else
	{
		Fan_Off();
	}
	if (pump_required)
	{
		Pump_On();
	}
	else
	{
		Pump_Off();
	}
}
