#include "voice.h"
#include "environment.h"
#include "control.h"
#include "usart.h"
#include "stm32f1xx_hal.h"

static uint8_t rx_byte;
static volatile Voice_Cmd_t pending_command;
static volatile uint8_t command_ready;
static uint8_t frame[5];
static uint8_t frame_index;

static void Voice_ParseByte(uint8_t byte)
{
    if (frame_index == 0U && byte != 0xAAU) return;
    frame[frame_index++] = byte;
    if (frame_index == 5U)
    {
        if (frame[1] == 0x55U && frame[3] == 0x55U && frame[4] == 0xAAU &&
            frame[2] >= VOICE_CMD_LIGHT_ON && frame[2] <= VOICE_CMD_AUTO_MODE)
        {
            pending_command = (Voice_Cmd_t)frame[2];
            command_ready = 1U;
        }
        frame_index = 0U;
    }
}

void Voice_Init(void)
{
    frame_index = 0U;
    command_ready = 0U;
    (void)HAL_UART_Receive_IT(&huart3, &rx_byte, 1U);
}

uint8_t Voice_Process(void)
{
    Voice_Cmd_t command;
    if (command_ready == 0U) return 0U;
    __disable_irq();
    command = pending_command;
    command_ready = 0U;
    __enable_irq();
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
    Device_Control_Update(&g_env_data, &g_threshold);
    return 1U;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        Voice_ParseByte(rx_byte);
        (void)HAL_UART_Receive_IT(&huart3, &rx_byte, 1U);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        (void)HAL_UART_Receive_IT(&huart3, &rx_byte, 1U);
    }
}
