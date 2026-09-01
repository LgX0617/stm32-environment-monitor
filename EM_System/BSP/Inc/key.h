#ifndef __KEY_H
#define __KEY_H

#include "main.h"

#include "environment.h"

extern volatile uint8_t key_up_flag;
extern volatile uint8_t key0_flag;
extern volatile uint8_t key1_flag;

extern uint32_t key_up_tick;
extern uint32_t key0_tick;
extern uint32_t key1_tick;
#endif

