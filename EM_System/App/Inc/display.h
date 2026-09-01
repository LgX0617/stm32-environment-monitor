#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "oled.h"
#include "main.h"
#include "environment.h"
#include "font.h"
#include "stdio.h"
#include "key.h"

typedef enum
{
	PAGE_DATA,
	PAGE_FAN_TEMP_THRESHOLD,
	PAGE_HIGH_TEMP_THRESHOLD,
	PAGE_FAN_HUMI_THRESHOLD,
	PAGE_HIGH_HUMI_THRESHOLD,
	PAGE_LIGHT_THRESHOLD,
	PAGE_COUNT
} DisplayPage_t;

void Display_AdjustThreshold(int8_t direction);

void Display_Update(void);
 
void Display_NextPage(void);
#endif
