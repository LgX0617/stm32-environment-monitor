#include "uart.h"

char buffer[100];

void SensorData_Send(EnvironmentData *data){
	sprintf(buffer,"light: %d\r\n",data->light);
	HAL_UART_Transmit(&huart1,(uint8_t*)buffer,strlen(buffer),HAL_MAX_DELAY);
}

