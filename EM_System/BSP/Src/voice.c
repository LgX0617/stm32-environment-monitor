#include "voice.h"
#include "environment.h"
#include "control.h"
#include "usart.h"
#include "stm32f1xx_hal.h"

/*
 * 语音串口使用 USART3，采用“每次接收 1 字节”的普通中断方式。
 * 控制帧格式：AA 55 CMD 55 AA
 */

static uint8_t rx_byte;                         /* USART3 当前接收到的字节 */
static volatile Voice_Cmd_t pending_command;   /* 已解析、等待主循环处理的命令 */
static volatile uint8_t command_ready;         /* 1 表示有新命令 */
static uint8_t frame[5];                        /* 正在接收的 5 字节控制帧 */
static uint8_t frame_index;                     /* 当前帧已接收的字节数 */

/* 将接收到的单字节拼成完整控制帧并校验 */
static void Voice_ParseByte(uint8_t byte)
{
    /* 帧必须以 0xAA 开始，丢弃帧头之前的无效数据 */
    if (frame_index == 0U && byte != 0xAAU) return;
    frame[frame_index++] = byte;

    /* 收满 5 字节后检查帧格式和命令范围 */
    if (frame_index == 5U)
    {
        if (frame[1] == 0x55U && frame[3] == 0x55U && frame[4] == 0xAAU &&
            frame[2] >= VOICE_CMD_LIGHT_ON && frame[2] <= VOICE_CMD_AUTO_MODE)
        {
            pending_command = (Voice_Cmd_t)frame[2];
            command_ready = 1U;
        }
        frame_index = 0U; /* 无论校验是否成功，都准备接收下一帧 */
    }
}

/* 初始化语音命令状态，并启动 USART3 单字节中断接收 */
void Voice_Init(void)
{
    frame_index = 0U;
    command_ready = 0U;
    (void)HAL_UART_Receive_IT(&huart3, &rx_byte, 1U);
}

uint8_t Voice_Process(void)
{
    Voice_Cmd_t command;

    /* 没有完整命令时立即返回，避免阻塞主循环 */
    if (command_ready == 0U) return 0U;

    /* 临界区：防止读取命令过程中被串口中断修改状态 */
    __disable_irq();
    command = pending_command;
    command_ready = 0U;
    __enable_irq();

    /* 语音控制外设时切换到手动模式；自动模式命令例外 */
    switch (command)
    {
    case VOICE_CMD_LIGHT_ON:    g_control_mode = CONTROL_MODE_MANUAL; g_manual_light = 1U; break;
    case VOICE_CMD_LIGHT_OFF:   g_control_mode = CONTROL_MODE_MANUAL; g_manual_light = 0U; break;
    case VOICE_CMD_FAN_ON:      g_control_mode = CONTROL_MODE_MANUAL; g_manual_fan = 1U; break;
    case VOICE_CMD_FAN_OFF:     g_control_mode = CONTROL_MODE_MANUAL; g_manual_fan = 0U; break;
    case VOICE_CMD_PUMP_ON:     g_control_mode = CONTROL_MODE_MANUAL; g_manual_pump = 1U; break;
    case VOICE_CMD_PUMP_OFF:    g_control_mode = CONTROL_MODE_MANUAL; g_manual_pump = 0U; break;
    case VOICE_CMD_MANUAL_MODE: g_control_mode = CONTROL_MODE_MANUAL; break;
    case VOICE_CMD_AUTO_MODE:   g_control_mode = CONTROL_MODE_AUTO; break;
    default: return 0U;
    }
    /* 立即应用新状态，不必等下一次传感器采样 */
    Device_Control_Update(&g_env_data, &g_threshold);
    return 1U;
}

/* HAL 串口接收完成回调：处理当前字节后继续接收下一个字节 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        Voice_ParseByte(rx_byte);
        (void)HAL_UART_Receive_IT(&huart3, &rx_byte, 1U);
    }
}

/* USART3 发生错误后重新启动接收，避免通信中断 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        (void)HAL_UART_Receive_IT(&huart3, &rx_byte, 1U);
    }
}
