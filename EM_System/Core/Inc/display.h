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
    PAGE_DATA,             // 0
    PAGE_TEMP_THRESHOLD,   // 1
    PAGE_HUMI_THRESHOLD,   // 2
    PAGE_LIGHT_THRESHOLD,  // 3
    PAGE_COUNT             // 4
} DisplayPage_t;

void Display_AdjustThreshold(int8_t direction);

void Display_Update(void);
 
void Display_NextPage(void);
#endif
