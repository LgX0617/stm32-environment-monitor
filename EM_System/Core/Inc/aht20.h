#ifndef __AHT20_H
#define __AHT20_H

#include "i2c.h"

#include "main.h"

#include "environment.h"

void AHT20_Init(void);

void AHT20_read(EnvironmentData * vlaue);

//uint16_t Hum_Read(void);

//int16_t Tem_Read(void);

#endif