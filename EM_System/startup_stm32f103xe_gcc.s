.syntax unified
.cpu cortex-m3
.thumb

.global g_pfnVectors
.global Default_Handler
.global Reset_Handler
.extern SystemInit
.extern __libc_init_array
.extern main

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
  .word WWDG_IRQHandler
  .word PVD_IRQHandler
  .word TAMPER_IRQHandler
  .word RTC_IRQHandler
  .word FLASH_IRQHandler
  .word RCC_IRQHandler
  .word EXTI0_IRQHandler
  .word EXTI1_IRQHandler
  .word EXTI2_IRQHandler
  .word EXTI3_IRQHandler
  .word EXTI4_IRQHandler
  .word DMA1_Channel1_IRQHandler
  .word DMA1_Channel2_IRQHandler
  .word DMA1_Channel3_IRQHandler
  .word DMA1_Channel4_IRQHandler
  .word DMA1_Channel5_IRQHandler
  .word DMA1_Channel6_IRQHandler
  .word DMA1_Channel7_IRQHandler
  .word ADC1_2_IRQHandler
  .word USB_HP_CAN1_TX_IRQHandler
  .word USB_LP_CAN1_RX0_IRQHandler
  .word CAN1_RX1_IRQHandler
  .word CAN1_SCE_IRQHandler
  .word EXTI9_5_IRQHandler
  .word TIM1_BRK_IRQHandler
  .word TIM1_UP_IRQHandler
  .word TIM1_TRG_COM_IRQHandler
  .word TIM1_CC_IRQHandler
  .word TIM2_IRQHandler
  .word TIM3_IRQHandler
  .word TIM4_IRQHandler
  .word I2C1_EV_IRQHandler
  .word I2C1_ER_IRQHandler
  .word I2C2_EV_IRQHandler
  .word I2C2_ER_IRQHandler
  .word SPI1_IRQHandler
  .word SPI2_IRQHandler
  .word USART1_IRQHandler
  .word USART2_IRQHandler
  .word USART3_IRQHandler
  .word EXTI15_10_IRQHandler
  .word RTC_Alarm_IRQHandler
  .word USBWakeUp_IRQHandler
  .word TIM8_BRK_IRQHandler
  .word TIM8_UP_IRQHandler
  .word TIM8_TRG_COM_IRQHandler
  .word TIM8_CC_IRQHandler
  .word ADC3_IRQHandler
  .word FSMC_IRQHandler
  .word SDIO_IRQHandler
  .word TIM5_IRQHandler
  .word SPI3_IRQHandler
  .word UART4_IRQHandler
  .word UART5_IRQHandler
  .word TIM6_IRQHandler
  .word TIM7_IRQHandler
  .word DMA2_Channel1_IRQHandler
  .word DMA2_Channel2_IRQHandler
  .word DMA2_Channel3_IRQHandler
  .word DMA2_Channel4_5_IRQHandler

.section .text.Reset_Handler,"ax",%progbits
.type Reset_Handler, %function
Reset_Handler:
  ldr r0, =_estack
  mov sp, r0
  ldr r1, =_sidata
  ldr r2, =_sdata
  ldr r3, =_edata
1:
  cmp r2, r3
  bcs 2f
  ldr r0, [r1], #4
  str r0, [r2], #4
  b 1b
2:
  ldr r2, =_sbss
  ldr r3, =_ebss
  movs r0, #0
3:
  cmp r2, r3
  bcs 4f
  str r0, [r2], #4
  b 3b
4:
  bl SystemInit
  bl __libc_init_array
  bl main
  b .

.section .text.Default_Handler,"ax",%progbits
.type Default_Handler, %function
Default_Handler:
  b .

.macro WEAK_HANDLER name
  .weak \name
  .set \name, Default_Handler
.endm
WEAK_HANDLER NMI_Handler
WEAK_HANDLER HardFault_Handler
WEAK_HANDLER MemManage_Handler
WEAK_HANDLER BusFault_Handler
WEAK_HANDLER UsageFault_Handler
WEAK_HANDLER SVC_Handler
WEAK_HANDLER DebugMon_Handler
WEAK_HANDLER PendSV_Handler
WEAK_HANDLER SysTick_Handler
.irp handler, WWDG_IRQHandler,PVD_IRQHandler,TAMPER_IRQHandler,RTC_IRQHandler,FLASH_IRQHandler,RCC_IRQHandler,EXTI0_IRQHandler,EXTI1_IRQHandler,EXTI2_IRQHandler,EXTI3_IRQHandler,EXTI4_IRQHandler,DMA1_Channel1_IRQHandler,DMA1_Channel2_IRQHandler,DMA1_Channel3_IRQHandler,DMA1_Channel4_IRQHandler,DMA1_Channel5_IRQHandler,DMA1_Channel6_IRQHandler,DMA1_Channel7_IRQHandler,ADC1_2_IRQHandler,USB_HP_CAN1_TX_IRQHandler,USB_LP_CAN1_RX0_IRQHandler,CAN1_RX1_IRQHandler,CAN1_SCE_IRQHandler,EXTI9_5_IRQHandler,TIM1_BRK_IRQHandler,TIM1_UP_IRQHandler,TIM1_TRG_COM_IRQHandler,TIM1_CC_IRQHandler,TIM2_IRQHandler,TIM3_IRQHandler,TIM4_IRQHandler,I2C1_EV_IRQHandler,I2C1_ER_IRQHandler,I2C2_EV_IRQHandler,I2C2_ER_IRQHandler,SPI1_IRQHandler,SPI2_IRQHandler,USART1_IRQHandler,USART2_IRQHandler,USART3_IRQHandler,EXTI15_10_IRQHandler,RTC_Alarm_IRQHandler,USBWakeUp_IRQHandler,TIM8_BRK_IRQHandler,TIM8_UP_IRQHandler,TIM8_TRG_COM_IRQHandler,TIM8_CC_IRQHandler,ADC3_IRQHandler,FSMC_IRQHandler,SDIO_IRQHandler,TIM5_IRQHandler,SPI3_IRQHandler,UART4_IRQHandler,UART5_IRQHandler,TIM6_IRQHandler,TIM7_IRQHandler,DMA2_Channel1_IRQHandler,DMA2_Channel2_IRQHandler,DMA2_Channel3_IRQHandler,DMA2_Channel4_5_IRQHandler
  WEAK_HANDLER \handler
.endr
