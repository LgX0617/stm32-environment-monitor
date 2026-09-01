#include "motor.h"

void Motor_Init(void)
{
	
	Fan_Off();
	Pump_Off();
	HAL_GPIO_WritePin(TB_STBY_GPIO_Port,TB_STBY_Pin,GPIO_PIN_RESET);
}

void Fan_On(void)
{
	HAL_GPIO_WritePin(TB_STBY_GPIO_Port,TB_STBY_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(TB_PWMA_GPIO_Port, TB_PWMA_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TB_AIN1_GPIO_Port, TB_AIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TB_AIN2_GPIO_Port, TB_AIN2_Pin, GPIO_PIN_RESET);
}

void Fan_Off(void)
{
    HAL_GPIO_WritePin(TB_PWMA_GPIO_Port, TB_PWMA_Pin, GPIO_PIN_RESET);
}

void Pump_On(void)
{
	HAL_GPIO_WritePin(TB_STBY_GPIO_Port,TB_STBY_Pin,GPIO_PIN_SET);
    HAL_GPIO_WritePin(TB_PWMB_GPIO_Port, TB_PWMB_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TB_BIN1_GPIO_Port, TB_BIN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TB_BIN2_GPIO_Port, TB_BIN2_Pin, GPIO_PIN_RESET);
}

void Pump_Off(void)
{
	HAL_GPIO_WritePin(TB_PWMB_GPIO_Port, TB_PWMB_Pin, GPIO_PIN_RESET);
}

void Motor_StopAll(void)
{
    Fan_Off();
    Pump_Off();
}
