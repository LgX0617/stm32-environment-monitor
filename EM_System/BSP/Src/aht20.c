#include "aht20.h"

#define AHT20_ADDR 0x70

void AHT20_Init(){
	uint8_t ReadBuffer;
	HAL_Delay(40);
	HAL_I2C_Master_Receive(&hi2c1,AHT20_ADDR,&ReadBuffer,1,HAL_MAX_DELAY);
	
	if((ReadBuffer & 0x08) == 0x00)
	{
		uint8_t sendBuffer[3] = {0xBE,0x08,0x00};
		HAL_I2C_Master_Transmit(&hi2c1,AHT20_ADDR,sendBuffer,3,HAL_MAX_DELAY);
	}
}

void AHT20_read(EnvironmentData *value){
	uint8_t sendBuffer[3] = {0xAC,0x33,0x00};
	uint8_t readBuffer[6];
	HAL_I2C_Master_Transmit(&hi2c1,AHT20_ADDR,sendBuffer,3,HAL_MAX_DELAY);
	HAL_Delay(80);
	HAL_I2C_Master_Receive(&hi2c1,AHT20_ADDR,readBuffer,6,HAL_MAX_DELAY);
	if((readBuffer[0] & 0x80) == 0)
	{
		uint32_t data;
		data = ((uint32_t)readBuffer[3]>>4)| ((uint32_t)readBuffer[2]<<4)
		| ((uint32_t)readBuffer[1]<<12);
		
		 value->humidity_x10 = data*1000/(1<<20);
		
		data = (((uint32_t)readBuffer[3]&0x0F)<<16) | ((uint32_t)readBuffer[4]<<8)
		| ((uint32_t)readBuffer[5]);
		value->temperature_x10 = ((data * 200.0f/(1<<20))-50)*10;
	}
}
