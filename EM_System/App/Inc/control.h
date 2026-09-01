#ifndef __CONTROL_H
#define __CONTROL_H

#include "main.h"
#include "environment.h"

#define DEFAULT_FAN_TEMP_ON_X10      260             
#define DEFAULT_HIGH_TEMP_ALARM_X10  300
#define DEFAULT_FAN_HUMI_ON_X10      700
#define DEFAULT_HIGH_HUMI_ALARM_X10  850
#define DEFAULT_LIGHT_ON_THRESHOLD   1000              //打开灯光阈值
#define DEFAULT_LIGHT_OFF_THRESHOLD   800             //关闭灯光阈值

void Light_Control_Update(const EnvironmentData* data,const Threshold* threshold);
void Fan_Control_Update(const EnvironmentData* data,const Threshold* threshold);
void Water_Control_Update(void);

#endif
