#include "uart.h"

char buffer[100];
#define UART_RX_BUFFER_SIZE 64

static uint8_t rx_byte;                         //当前收到的一个字节
static char rx_buffer[UART_RX_BUFFER_SIZE];    //存储接收的指令
static volatile uint8_t rx_index = 0;         //下一个字符应写入数组的位置
static volatile uint8_t rx_frame_ready = 0;  //是否已经收到\n,即是否有完整命令待处理
static volatile uint8_t rx_discarding = 0;   //缓存区是否溢出

unsigned int new_light_threshold;            //新的光照阈值
char extra_char;
int parse_result;														//解析命令返回值
uint8_t updated = 0;												//是否解析成功


//发送传感器数据
void SensorData_Send(EnvironmentData *data){
	sprintf(buffer,"$DATA,L=%u,A=%u\r\n",data->light,data->alarm);
	HAL_UART_Transmit(&huart1,(uint8_t*)buffer,strlen(buffer),HAL_MAX_DELAY);
}

//启动接收中断
//接收命令
void UART_RxStart(void){
	HAL_UART_Receive_IT(&huart1,&rx_byte,1);
}

//处理命令
uint8_t UART_ProcessCommand(Threshold *threshold){
	updated = 0;
	if(rx_frame_ready == 0)
	{
		return 0;
	}
	
	parse_result = sscanf(rx_buffer,"$SET,L_thr,%u%c",  
	                      &new_light_threshold,
												&extra_char);
	if((parse_result == 1) && new_light_threshold <=4095)
	{
		threshold->light_max = (uint16_t )new_light_threshold;
		sprintf(buffer,"$ACK,L_thr,%u\r\n",new_light_threshold);
		HAL_UART_Transmit(&huart1,(uint8_t*)buffer,strlen(buffer),HAL_MAX_DELAY);
		updated = 1;
	}else if(parse_result != 1){
		sprintf(buffer,"$ERR,FORMAT\r\n");
		HAL_UART_Transmit(&huart1,(uint8_t*)buffer,strlen(buffer),HAL_MAX_DELAY);
	}else if(new_light_threshold >4095){
		sprintf(buffer,"$ERR,RANGE\r\n");
		HAL_UART_Transmit(&huart1,(uint8_t*)buffer,strlen(buffer),HAL_MAX_DELAY);
	}
	
	rx_frame_ready = 0;
  rx_index = 0;
  UART_RxStart();

return updated;
	
	
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart == &huart1)
		{
		if(rx_discarding == 1)
		{
			if(rx_byte == '\n')
			{
				rx_discarding = 0;
				rx_index = 0;
			}
			UART_RxStart();
			return;
		}
		if(rx_byte == '\r')
		{
			UART_RxStart();
			
		}
		else if(rx_byte == '\n'){
			rx_buffer[rx_index] = '\0';
			rx_index = 0;
			rx_frame_ready = 1;
		}
		else{
			if(rx_index < UART_RX_BUFFER_SIZE-1){
				rx_buffer[rx_index] = rx_byte;
				rx_index++;
				UART_RxStart();
			}
			else{
				rx_index = 0;
				rx_discarding = 1;
				rx_buffer[0] = '\0';
				UART_RxStart();
			}
			
			
		}
			
	}
}



