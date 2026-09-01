#include "display.h"

static char line[20];
DisplayPage_t display_page;

static void FormatTemperature(char *line,size_t line_size ,int16_t temperature_x10)
{
		int16_t temp = temperature_x10;
		uint16_t temp_abs;
	if(temp < 0)
	{
		temp_abs = (uint16_t)(-temp);
		snprintf(line,line_size,"T: -%u.%uC",temp_abs/10,temp_abs%10);
	}else
	{
		snprintf(line,line_size,"T: %u.%uC",temp/10,temp%10);
	}
}

static void FormatHumidity(char *line,size_t line_size, uint16_t humidity_x10)
{
	snprintf(line,line_size,"H: %u.%u%%",humidity_x10/10,humidity_x10%10);
}

static void FormatLight(char *line,size_t line_size, uint16_t light)
{
	snprintf(line,line_size,"L: %u ",light);
}

static void FormatAlarm(char *line,size_t line_size, uint8_t alarm)
{
	snprintf(line,line_size,"A: %u",alarm);
}

static void FormatWater_Alarm(char *line,size_t line_size, uint8_t water_alarm)
{
	snprintf(line,line_size,"W: %u",water_alarm);
}

static void FormatSmoke_Alarm(char *line,size_t line_size, uint8_t smoke_alarm)
{
	snprintf(line,line_size,"S: %u",smoke_alarm);
}

//温度阈值
static void FormatTemperatureThreshold(char *line,size_t line_size,const char * label, int16_t value_x10)
{
		int16_t temp = value_x10;
		uint16_t temp_abs;
	if(temp < 0)
	{
		temp_abs = (uint16_t)(-temp);
		snprintf(line,line_size,"%s: -%u.%uC",label,temp_abs/10,temp_abs%10);
	}else
	{
		snprintf(line,line_size,"%s: %u.%uC",label,temp/10,temp%10);
	}
}

//湿度阈值
static void FormatHumidityThreshold(char *line,size_t line_size, const char *label, uint16_t value_x10)
{
	snprintf(line,line_size,"%s: %u.%u%%",label,value_x10/10,value_x10%10);
}

//光照阈值
static void FormatLightThreshold(char *line,size_t line_size, uint16_t value_x10)
{
	snprintf(line,line_size,"L: %u",value_x10);
}

void Display_Update(){

	OLED_NewFrame();  //清除显存
	switch(display_page){
		case PAGE_DATA:
			FormatTemperature(line, sizeof(line),g_env_data.temperature_x10);
			OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
	
			FormatHumidity(line,sizeof(line), g_env_data.humidity_x10);
			OLED_PrintASCIIString(0,16,line,&afont16x8,OLED_COLOR_NORMAL);
	
			FormatLight(line,sizeof(line), g_env_data.light);
			OLED_PrintASCIIString(0,32,line,&afont16x8,OLED_COLOR_NORMAL);
	
			FormatAlarm(line, sizeof(line),g_env_data.alarm);
			OLED_PrintASCIIString(80,48,line,&afont16x8,OLED_COLOR_NORMAL);
		
			FormatWater_Alarm(line,sizeof(line),g_env_data.water_alarm);
			OLED_PrintASCIIString(40,48,line,&afont16x8,OLED_COLOR_NORMAL);
		
			FormatSmoke_Alarm(line,sizeof(line),g_env_data.smoke_alarm);
			OLED_PrintASCIIString(0,48,line,&afont16x8,OLED_COLOR_NORMAL);
		
		break;
			
		case PAGE_FAN_TEMP_THRESHOLD:
			FormatTemperatureThreshold(line,sizeof(line),"Fan_T:",g_threshold.fan_temp_on_x10);
		  OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
		break;
		
		case PAGE_HIGH_TEMP_THRESHOLD:
			FormatTemperatureThreshold(line,sizeof(line),"Alarm_T:",g_threshold.high_temp_alarm_x10);
			OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
		break;
		
		case PAGE_FAN_HUMI_THRESHOLD:
			FormatHumidityThreshold(line,sizeof(line),"Fan_H:",g_threshold.fan_humidity_on_x10);
			OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
		break;
		
		case PAGE_HIGH_HUMI_THRESHOLD:
			FormatHumidityThreshold(line,sizeof(line),"Alarm_H:",g_threshold.high_humidity_alarm_x10);
			OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
		break;
		
		case PAGE_LIGHT_THRESHOLD:
			FormatLightThreshold(line,sizeof(line),g_threshold.light_on_threshold);
			OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
		break;
		
		case PAGE_COUNT:
			break;
	}
	OLED_ShowFrame();
}

void Display_NextPage(void){
		display_page = (DisplayPage_t)((display_page+1)%PAGE_COUNT);
		Display_Update();
}

void Display_AdjustThreshold(int8_t direction)
{
    switch (display_page)
    {
        case PAGE_FAN_TEMP_THRESHOLD:
            if (direction > 0)
            {
                if (g_threshold.fan_temp_on_x10 < g_threshold.high_temp_alarm_x10 - 30)
                {
                    g_threshold.fan_temp_on_x10 += 10;
                }
            }
            else
            {
                if (g_threshold.fan_temp_on_x10 > -400)
                {
                    g_threshold.fan_temp_on_x10 -= 10;
                }
            }
            break;

        case PAGE_HIGH_TEMP_THRESHOLD:
            if (direction > 0)
            {
                if (g_threshold.high_temp_alarm_x10 < 850)
                {
                    g_threshold.high_temp_alarm_x10 += 10;
                }
            }
            else
            {
                if (g_threshold.high_temp_alarm_x10 > g_threshold.fan_temp_on_x10 + 30)
                {
                    g_threshold.high_temp_alarm_x10 -= 10;
                }
            }
            break;

        case PAGE_FAN_HUMI_THRESHOLD:
            if (direction > 0)
            {
                if (g_threshold.fan_humidity_on_x10 < g_threshold.high_humidity_alarm_x10 - 150)
                {
                    g_threshold.fan_humidity_on_x10 += 10;
                }
            }
            else
            {
                if (g_threshold.fan_humidity_on_x10 >= 10)
                {
                    g_threshold.fan_humidity_on_x10 -= 10;
                }
            }
            break;
			
        case PAGE_HIGH_HUMI_THRESHOLD:
            if (direction > 0)
            {
                if (g_threshold.high_humidity_alarm_x10 < 1000)
                {
                    g_threshold.high_humidity_alarm_x10 += 10;
                }
            }
            else
            {
                if (g_threshold.high_humidity_alarm_x10 > g_threshold.fan_humidity_on_x10 + 150)
                {
                    g_threshold.high_humidity_alarm_x10 -= 10;
                }
            }
            break;

        case PAGE_LIGHT_THRESHOLD:
            if (direction > 0)
            {
                if (g_threshold.light_on_threshold < 4095)
                {
                    g_threshold.light_on_threshold += 100;
                }
            }
            else
            {
                if (g_threshold.light_on_threshold >= 100)
                {
                    g_threshold.light_on_threshold -= 100;
                }
            }
            break;
			
        default:
            break;
    }

    Display_Update();
}
