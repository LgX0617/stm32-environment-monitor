#include "uart.h"

static uint8_t rx_dma_byte[64];                         //当前收到的一个字节

static RxState_t state;                             //当前状态
static Command_t rx_cmd;                            //当前命令
static uint8_t rx_payload[3];                                //数据区
static uint8_t payload_index;                              //已经接收到的数据字节数
static uint8_t expected_length;                             //本应该接收的数据字节数
static uint8_t checksum;                                   //校验和
static volatile uint8_t frame_ready = 0;                  //主循环是否可以处理完整帧

static void UART_SendFrame(uint8_t cmd,
                           const uint8_t *payload,
                           uint8_t length);
													 
static void UART_ParseByte(uint8_t byte);
													 
static uint8_t tx_dma_buffer[UART_TX_FRAME_MAX];            //发送数据存储
static volatile uint8_t tx_busy = 0;                         //发送完成标志位
													 
//发送传感器数据
void SensorData_Send(EnvironmentData *data,Threshold *thr){
	uint8_t payload[13];
   uint16_t temperature_raw;
	uint16_t temperature_max_x10_raw;
	
	temperature_raw = (uint16_t)data->temperature_x10;
	
	payload[0] = (uint8_t)(temperature_raw>>8);
	payload[1] = (uint8_t)temperature_raw;
	
	payload[2] = (uint8_t)(data->humidity_x10>>8);
	payload[3] = (uint8_t)data->humidity_x10;
	
	payload[4] = (uint8_t)(data->light>>8);
	payload[5] = (uint8_t)data->light;
	
	payload[6] = data->alarm;
	
	temperature_max_x10_raw = (uint16_t)thr->temperature_max_x10;
	
	payload[7] = (uint8_t)(temperature_max_x10_raw>>8);
	payload[8] = (uint8_t)temperature_max_x10_raw;
	
	payload[9] = (uint8_t)(thr->humidity_max_x10>>8);
	payload[10] = (uint8_t)thr->humidity_max_x10;
	
	payload[11] = (uint8_t)(thr->light_max>>8);
	payload[12] = (uint8_t)thr->light_max;
	
	
	UART_SendFrame(CMD_DATA,payload,13);
}

//启动接收中断
//接收命令
void UART_RxStart(void){
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,rx_dma_byte,sizeof(rx_dma_byte));
	__HAL_DMA_DISABLE_IT(huart1.hdmarx,DMA_IT_HT);
}

//处理命令
uint8_t UART_ProcessCommand(Threshold *threshold){
	uint8_t parameter_id ;
	uint16_t value;
	uint8_t updated = 0;												//是否解析成功
	int16_t temperature_value;
	uint8_t error_payload[1];
	
	if(frame_ready == 0)
	{
		return 0;
	}
	if(rx_cmd == CMD_SET_THRESHOLD)
	{
		parameter_id = rx_payload[0];
		
		value = ((uint16_t)rx_payload[1] << 8) | rx_payload[2];
		temperature_value = (int16_t)value;
		switch(parameter_id){
			case PARAM_TEMP_MAX:
				if((temperature_value >= -400) && (temperature_value <= 850))
				{
					threshold->temperature_max_x10 = temperature_value;
					updated = 1;
				}
				else
				{
					error_payload[0] = ERROR_RANGE;
				}
			break;
			
			case PARAM_HUMI_MAX:
				if(value <= 1000)
				{
					threshold->humidity_max_x10 = value;
					updated = 1;
				}
				else
				{
					error_payload[0] = ERROR_RANGE;
				}
			break;
			
			case PARAM_LIGHT_DARK:
				if(value <= 4095)
				{
					threshold->light_max = value;
					updated = 1;
				}
				else
				{
					error_payload[0] = ERROR_RANGE;
				}
			break;
				
			default:
				error_payload[0] = ERROR_PARAM;
			break;
		}
	}
	if(updated == 1)
	{
		UART_SendFrame(CMD_ACK,rx_payload,3);
	}
	else
	{
		UART_SendFrame(CMD_ERROR,error_payload,1);
	}
	
	frame_ready = 0;
	
	return updated;
}


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	uint16_t i;
	if(huart == &huart1)
		{
		for(i = 0;i<Size;i++)
			{
				 UART_ParseByte(rx_dma_byte[i]);
			}
			UART_RxStart();
	}
}

static void UART_ParseByte(uint8_t rx_byte)
{
    switch(state){
			case WAIT_HEAD:
				if((frame_ready == 0) && (rx_byte == FRAME_HEAD))
				{
					state = GET_CMD;
					checksum = 0;
					payload_index = 0;
				}
				break;
				
			case GET_CMD:
				if(rx_byte == CMD_SET_THRESHOLD)
				{
					rx_cmd = (Command_t)rx_byte;
					expected_length = 3;
					payload_index = 0;
					checksum = rx_byte;
					state = GET_DATA;
				}
				else
				{
					state =WAIT_HEAD;
				}
				break;
				
			case GET_DATA:
				rx_payload[payload_index] = rx_byte;
			  checksum += rx_byte;
			  payload_index++;
			
			  if(payload_index >= expected_length)
			  {
					state = GET_CHECK;
			  }
				break;
			
			case GET_CHECK:
				if(rx_byte == checksum)
				{
					frame_ready = 1;
				}
				state = WAIT_HEAD;
				break;
		}
}

static void UART_SendFrame(uint8_t cmd,
                           const uint8_t *payload,
                           uint8_t length)
{
	if(tx_busy == 0)
	{
    uint8_t index = 0;
    uint8_t tx_checksum = cmd;
    uint8_t i;

    tx_dma_buffer[index++] = FRAME_HEAD;
    tx_dma_buffer[index++] = cmd;

    for (i = 0; i < length; i++)
    {
        tx_dma_buffer[index++] = payload[i];
        tx_checksum += payload[i];
    }

    tx_dma_buffer[index++] = tx_checksum;
		tx_busy = 1;

    HAL_UART_Transmit_DMA(&huart1, tx_dma_buffer, index);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if(huart == &huart1)
	{
		tx_busy= 0;
	}
}



