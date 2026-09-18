# 山洞边缘服务器环境监测与语音控制系统

基于 STM32F103 凤凰开发板的物联网环境监测项目。系统用于监测山洞边缘服务器部署点的环境状态，在温湿度、光照、烟雾或积水异常时执行报警与联动控制，并支持 CI-03T 离线语音控制。

## 系统功能

- AHT20 采集温度、湿度
- 光敏电阻采集环境光照
- MQ-2 烟雾模块检测烟雾
- 水浸模块检测积水
- OLED 显示实时环境数据、阈值与报警状态
- 蜂鸣器提示温度、湿度等超阈值状态
- TB6612FNG 驱动风扇和水泵
- CI-03T 支持“你好小智”唤醒及设备语音控制
- 预留 UART 上位机数据传输接口，可接 USB 串口或 JDY-33 蓝牙模块

## 项目结构

```text
EM_System/                  STM32 固件工程
  Core/                     CubeMX 外设初始化、主循环
  BSP/                      传感器、OLED、LED、电机等硬件驱动
  App/                      环境数据、控制规则、显示、通信协议
  Drivers/                  STM32 HAL 与 CMSIS
  MDK-ARM/                  Keil uVision 工程
  EM_System.ioc             CubeMX 引脚与外设配置

PC_Host/                    Python 上位机原型
```

## 硬件接口

| 模块 | STM32 外设或引脚 | 说明 |
| --- | --- | --- |
| 温湿度 | AHT20，I2C1 | 与 OLED 共用 I2C 总线 |
| 光照 | ADC3_IN6 / PF8 | ADC 值为原始采样值 |
| 烟雾 DO | PB0 | 低电平表示烟雾报警 |
| 水浸 DO | PB1 | 低电平表示检测到积水 |
| OLED | SPI1 + 控制引脚 | 显示环境数据和阈值页面 |
| 蜂鸣器 | PB8 | 高温、高湿等报警提示 |
| TB6612 AIN1/AIN2 | PC2/PC3 | 风扇方向控制 |
| TB6612 PWMA | PC4 | 风扇使能 |
| TB6612 BIN1/BIN2 | PC5/PB9 | 水泵方向控制 |
| TB6612 PWMB | PB10 | 水泵使能 |
| TB6612 STBY | PB11 | 驱动器总使能 |
| 上位机 USART2 | PA2 TX、PA3 RX | 当前使用 USB 转串口或 JDY-33 蓝牙模块，配置为 9600、8N1 |
| 语音模块 USART3 | PD8 TX、PD9 RX | CI-03T 串口通信，配置为 115200、8N1 |

TB6612 的 `VCC` 接 3.3 V，`VM` 接 5 V 电机电源，所有模块必须共地。风扇和水泵必须由 TB6612 驱动，不能直接连接 STM32 GPIO。

## 自动控制逻辑

```text
温度达到风扇阈值 或 检测到烟雾 -> 启动风扇
温度低于风扇阈值回差 且 无烟雾 -> 关闭风扇
检测到积水 -> 启动水泵
积水消失 -> 关闭水泵
光照不足 -> 打开照明 LED
温度或湿度超报警阈值 -> 蜂鸣器报警
```

环境数据和阈值定义在 `EM_System/App/Inc/environment.h`，控制逻辑在 `EM_System/App/Src/control.c`。

## 离线语音控制

CI-03T 固件在智能公元平台配置为单麦离线语音方案，唤醒词为“你好小智”。CI-03T 使用模块的 `A2/A1` 串口复用脚通信；平台中的 `UART1_TX/UART1_RX` 是模块内部名称，不等于 STM32 USART1。

模块与 STM32 串口需要交叉连接，并设置为：`115200`、`8N1`、无校验、无流控。

| 语音命令 | CI-03T 发送帧 |
| --- | --- |
| 打开照明 | `AA 55 01 55 AA` |
| 关闭照明 | `AA 55 02 55 AA` |
| 打开风扇 | `AA 55 03 55 AA` |
| 关闭风扇 | `AA 55 04 55 AA` |
| 打开水泵 | `AA 55 05 55 AA` |
| 关闭水泵 | `AA 55 06 55 AA` |

## 编译

### Keil MDK

打开 `EM_System/MDK-ARM/EM_System.uvprojx`，执行 Rebuild 后下载到 STM32F103。

### GNU Arm

安装 GNU Arm 工具链和 GNU Make 后，在 `EM_System` 目录执行：

```powershell
make
```

输出在 `EM_System/build/`。该目录、Keil 缓存与中间文件均不纳入 Git。

## 上位机说明

STM32 通过 USART2（PA2/PA3）与 USB 转串口或 JDY-33 蓝牙模块通信，串口参数为 `9600`、`8N1`、无校验、无流控。环境数据定时上报，上位机可下发阈值设置帧；协议实现位于 `EM_System/App/Src/protocol.c`，Python 上位机位于 `PC_Host/`。

> 注意：使用 JDY-33 时，模块串口速率必须与 USART2 的 9600 一致。手机蓝牙串口工具可直接使用；电脑端只有在蓝牙驱动创建可用的虚拟 COM 口后，Python 上位机才能通过 PySerial 连接。

## FreeRTOS 运行方式

工程已通过 STM32CubeMX 启用 FreeRTOS，并使用 CMSIS-RTOS v1 接口。CubeMX 配置保存在 `EM_System/EM_System.ioc`，FreeRTOS 内核及 Cortex-M3 端口位于 `EM_System/Middlewares/Third_Party/FreeRTOS/`。

系统启动流程如下：

1. `main()` 完成时钟、GPIO、DMA、ADC、定时器和串口等硬件初始化。
2. `MX_FREERTOS_Init()` 创建 RTOS 任务及内核对象。
3. `osKernelStart()` 启动调度器，此后业务代码由 RTOS 任务执行。
4. 环境采样、按键处理、设备控制、报警、显示、语音和串口协议处理位于 `EM_System/Core/Src/freertos.c` 的 `StartDefaultTask()` 中。

`main()` 中原有的裸机轮询循环已停用，不会与 RTOS 任务重复执行。需要新增周期业务时，应放入任务中，并通过 `osDelay()` 等 RTOS 延时接口让出 CPU；不要在任务中编写不阻塞的永久忙循环。
