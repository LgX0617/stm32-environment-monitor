#include "display.h"

char line[20];

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

void Display_Update(const EnvironmentData *data){

	OLED_NewFrame();  //Çå³þÈí¼þÏÔ´æ
	
	FormatTemperature(line, sizeof(line),data->temperature_x10);
	OLED_PrintASCIIString(0,0,line,&afont16x8,OLED_COLOR_NORMAL);
	
	FormatHumidity(line,sizeof(line), data->humidity_x10);
	OLED_PrintASCIIString(0,16,line,&afont16x8,OLED_COLOR_NORMAL);
	
	FormatLight(line,sizeof(line), data->light);
	OLED_PrintASCIIString(0,32,line,&afont16x8,OLED_COLOR_NORMAL);
	
	FormatAlarm(line, sizeof(line),data->alarm);
	OLED_PrintASCIIString(0,48,line,&afont16x8,OLED_COLOR_NORMAL);
	
	OLED_ShowFrame();
}