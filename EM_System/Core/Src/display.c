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
	snprintf(line,line_size,"L: %u",light);
}

static void FormatAlarm(char *line,size_t line_size, uint8_t alarm)
{
	snprintf(line,line_size,"A: %u",alarm);
}

//温度阈值
static void FormatTemp_max(char *line,size_t line_size, int16_t temperature_max_x10)
{
		int16_t temp = temperature_max_x10;
		uint16_t temp_abs;
	if(temp < 0)
	{
		temp_abs = (uint16_t)(-temp);
		snprintf(line,line_size,"T_Max: -%u.%uC",temp_abs/10,temp_abs%10);
	}else
	{
		snprintf(line,line_size,"T_Max: %u.%uC",temp/10,temp%10);
	}
}

//湿度阈值
static void FormatHum_max(char *line,size_t line_size, uint16_t humidity_max_x10)
{
	snprintf(line,line_size,"H_Max: %u.%u%%",humidity_max_x10/10,humidity_max_x10%10);
}

//光照阈值
static void FormatLight_max(char *line,size_t line_size, uint16_t light_max)
{
	snprintf(line,line_size,"L_max: %u",light_max);
}

void Display_Update(){

	OLED_NewFrame();  //清楚软件显存
	switch(display_page){
		case PAGE_DATA:
			FormatTemperature(line, sizeof(line),g_env_data.temperature_x10);
			OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
	
			FormatHumidity(line,sizeof(line), g_env_data.humidity_x10);
			OLED_PrintASCIIString(0,16,line,&afont16x8,OLED_COLOR_NORMAL);
	
			FormatLight(line,sizeof(line), g_env_data.light);
			OLED_PrintASCIIString(0,32,line,&afont16x8,OLED_COLOR_NORMAL);
	
			FormatAlarm(line, sizeof(line),g_env_data.alarm);
			OLED_PrintASCIIString(0,48,line,&afont16x8,OLED_COLOR_NORMAL);
		break;
			
		case PAGE_TEMP_THRESHOLD:
			FormatTemp_max(line,sizeof(line),g_threshold.temperature_max_x10);
		  OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
		break;
		
		case PAGE_HUMI_THRESHOLD:
			FormatHum_max(line,sizeof(line),g_threshold.humidity_max_x10);
			OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
		break;
		
		case PAGE_LIGHT_THRESHOLD:
			FormatLight_max(line,sizeof(line),g_threshold.light_max);
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
        case PAGE_TEMP_THRESHOLD:
            if (direction > 0)
            {
                if (g_threshold.temperature_max_x10 < 990)
                {
                    g_threshold.temperature_max_x10 += 10;
                }
            }
            else
            {
                if (g_threshold.temperature_max_x10 > -500)
                {
                    g_threshold.temperature_max_x10 -= 10;
                }
            }
            break;

        case PAGE_HUMI_THRESHOLD:
            if (direction > 0)
            {
                if (g_threshold.humidity_max_x10 < 1000)
                {
                    g_threshold.humidity_max_x10 += 10;
                }
            }
            else
            {
                if (g_threshold.humidity_max_x10 >= 10)
                {
                    g_threshold.humidity_max_x10 -= 10;
                }
            }
            break;

        case PAGE_LIGHT_THRESHOLD:
            if (direction > 0)
            {
                if (g_threshold.light_max < 4095)
                {
                    g_threshold.light_max += 100;
                }
            }
            else
            {
                if (g_threshold.light_max >= 100)
                {
                    g_threshold.light_max -= 100;
                }
            }
            break;

        default:
            break;
    }

    Display_Update();
}
