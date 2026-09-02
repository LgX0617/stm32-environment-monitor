#ifndef __VOICE_H
#define __VOICE_H

#include "main.h"

typedef enum
{
    VOICE_CMD_LIGHT_ON = 0x01,
    VOICE_CMD_LIGHT_OFF = 0x02,
    VOICE_CMD_FAN_ON = 0x03,
    VOICE_CMD_FAN_OFF = 0x04,
    VOICE_CMD_PUMP_ON = 0x05,
    VOICE_CMD_PUMP_OFF = 0x06,
    VOICE_CMD_MANUAL_MODE = 0x07,
    VOICE_CMD_AUTO_MODE = 0x08
} Voice_Cmd_t;

void Voice_Init(void);
uint8_t Voice_Process(void);

#endif
