# 山洞边缘服务器环境监测系统

基于 STM32F103 凤凰开发板的物联网环境监测项目。系统面向山洞边缘服务器场景，采集温湿度、光照、烟雾和水浸状态，并根据环境状态控制照明、风扇和排水水泵；同时接入 CI-03T 离线语音模块，实现语音操作。

## 功能

- AHT20 采集温度和湿度
- 光敏电阻采集光照强度
- MQ-2 烟雾模块进行烟雾报警
- 水浸模块检测积水并自动启动水泵
- TB6612FNG 驱动 5 V 风扇和直流水泵
- OLED 显示环境数据、阈值和报警状态
- 蜂鸣器进行高温、高湿等报警
- CI-03T 离线语音控制照明、风扇和水泵
- 预留串口向上位机发送监测数据

## 目录

```text
Core/       CubeMX 生成的启动、外设初始化和主循环
BSP/        板级支持及硬件驱动（传感器、OLED、LED、电机）
App/        环境数据、控制逻辑、显示和通信协议
Drivers/    STM32 HAL 与 CMSIS 库
MDK-ARM/    Keil uVision 工程文件
Makefile    GNU Arm 编译脚本
```

## 主要引脚

| 功能 | STM32 引脚 |
| --- | --- |
| 水浸 DO | PB1（低电平表示有水） |
| 烟雾 DO | PB0（低电平表示报警） |
| TB6612 AIN1/AIN2 | PC2/PC3 |
| TB6612 PWMA（风扇） | PC4 |
| TB6612 BIN1/BIN2 | PC5/PB9 |
| TB6612 PWMB（水泵） | PB10 |
| TB6612 STBY | PB11 |
| 上位机串口 USART1 | PA9 TX、PA10 RX |
| I2C1（AHT20/OLED） | 以 `EM_System.ioc` 为准 |

TB6612 的 `VCC` 接 3.3 V，`VM` 接电机电源（本项目为 5 V），所有设备必须共地；风扇和水泵不能直接接 STM32 GPIO。

## 语音串口

平台中 CI-03T 的 `UART1_TX/UART1_RX` 是模块内部编号，不等于 STM32 的 USART1。模块收发线需要交叉连接到 STM32 的一组空闲 UART，并设置为 9600、8N1、无校验、无流控。语音命令帧为：

```text
打开照明  AA 55 01 55 AA
关闭照明  AA 55 02 55 AA
打开风扇  AA 55 03 55 AA
关闭风扇  AA 55 04 55 AA
打开水泵  AA 55 05 55 AA
关闭水泵  AA 55 06 55 AA
```

## 自动控制规则

- 温度达到风扇开启阈值或检测到烟雾时启动风扇；温度回差恢复且无烟雾时关闭风扇。
- 检测到水浸时启动水泵，积水消失后关闭水泵。
- 光照不足时打开照明，光照恢复后关闭照明。
- 温度或湿度超过报警阈值时启动蜂鸣器。

阈值默认值和数据结构位于 `App/Inc/environment.h`，自动控制逻辑位于 `App/Src/control.c`。

## 编译

打开 `MDK-ARM/EM_System.uvprojx` 后在 Keil 中执行 Rebuild。使用 GNU Arm 时，在 `EM_System` 目录执行：

```powershell
make
```

生成文件位于 `build/`，该目录为本地构建产物，不纳入版本控制。

## 添加模块

1. 驱动头文件和源文件分别放入 `BSP/Inc`、`BSP/Src`。
2. 在 `MDK-ARM/EM_System.uvprojx` 中加入源文件。
3. 在 `EM_System.ioc` 配置外设和 GPIO。
4. 阈值判断和自动控制放在 `App`，硬件驱动只负责设备读写。
