#ifndef __UART_H
#define __UART_H

#include "main.h"

#include "usart.h"

#include "stm32f1xx_hal.h"

#include "environment.h"

void SensorData_Send(EnvironmentData *data,Threshold *thr);

void UART_RxStart(void);

uint8_t UART_ProcessCommand(Threshold *threshold);


//帧头
#define FRAME_HEAD  0xAA

#define UART_TX_FRAME_MAX  20

//数据指令接收状态
typedef enum
{
    WAIT_HEAD,
    GET_CMD,
    GET_DATA,
    GET_CHECK
} RxState_t;

//命令类型
typedef enum
{
	CMD_DATA = 0x01,
	CMD_SET_THRESHOLD = 0x02,
	CMD_ACK = 0x03,
  CMD_ERROR = 0x04
}Command_t;

//参数编号
typedef enum
{
    PARAM_TEMP_MAX  = 0x01,
    PARAM_HUMI_MAX  = 0x02,
    PARAM_LIGHT_DARK = 0x03
} ParameterId_t;

//错误码
typedef enum
{
    ERROR_PARAM = 0x01,
    ERROR_RANGE = 0x02
} ErrorCode_t;

#endif


