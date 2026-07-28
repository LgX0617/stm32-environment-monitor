#include "uart.h"

static uint8_t rx_byte;                         //当前收到的一个字节

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
													 
//发送传感器数据
void SensorData_Send(EnvironmentData *data){
	uint8_t payload[7];
   uint16_t temperature_raw;
	
	temperature_raw = (uint16_t)data->temperature_x10;
	
	payload[0] = (uint8_t)(temperature_raw>>8);
	payload[1] = (uint8_t)temperature_raw;
	
	payload[2] = (uint8_t)(data->humidity_x10>>8);
	payload[3] = (uint8_t)data->humidity_x10;
	
	payload[4] = (uint8_t)(data->light>>8);
	payload[5] = (uint8_t)data->light;
	
	payload[6] = data->alarm;
	
	UART_SendFrame(CMD_DATA,payload,7);
}

//启动接收中断
//接收命令
void UART_RxStart(void){
	HAL_UART_Receive_IT(&huart1,&rx_byte,1);
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


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart == &huart1)
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
		UART_RxStart();
	}
}

static void UART_SendFrame(uint8_t cmd,
                           const uint8_t *payload,
                           uint8_t length)
{
    uint8_t tx_frame[UART_TX_FRAME_MAX];
    uint8_t index = 0;
    uint8_t tx_checksum = cmd;
    uint8_t i;

    tx_frame[index++] = FRAME_HEAD;
    tx_frame[index++] = cmd;

    for (i = 0; i < length; i++)
    {
        tx_frame[index++] = payload[i];
        tx_checksum += payload[i];
    }

    tx_frame[index++] = tx_checksum;

    HAL_UART_Transmit(&huart1, tx_frame, index, HAL_MAX_DELAY);
}



