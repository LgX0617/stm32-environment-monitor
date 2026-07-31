# STM32 环境监测终端

基于 STM32F103 的环境监测终端。系统采集温湿度与光照数据，在 OLED 上实时显示；超过阈值时驱动蜂鸣器和 LED 报警，并通过 UART 二进制协议与 Tkinter 上位机双向通信。三个物理按键可切换 OLED 页面并本地调整阈值。

## 功能进度

- [x] STM32F103 外设初始化与 CubeMX 工程配置
- [x] 光敏电阻 ADC 采样与 ADC 校准
- [x] TIM2 周期采样调度
- [x] 光照过暗阈值判断
- [x] 蜂鸣器与 LED 报警联动
- [x] USART1 周期数据上报
- [x] UART 中断接收与二进制状态机组帧、校验
- [x] Python 控制台上位机：实时数据显示与三个阈值设置
- [x] OLED SPI 驱动与基础字符显示
- [x] AHT20 I2C 驱动接入
- [x] AHT20 温湿度采集与 x10 定点数换算
- [x] OLED 环境数据实时显示
- [x] USART1 DMA 接收空闲事件与 DMA 发送
- [x] 按键页面切换与本地阈值调整

## 系统结构

```text
光敏电阻 -> ADC3 ┐
AHT20 -> I2C1    ├-> 环境数据结构体 -> 阈值判断 -> 蜂鸣器 / LED
                 │                         |
TIM2 周期调度 --┘                         v
                                      OLED 显示
                                          |
                         USART1 <------> Python 上位机
```

TIM2 中断只设置采样标志，ADC 读取、报警判断、串口发送和命令解析均在主循环执行，避免在中断中进行耗时操作。

按键使用 EXTI 中断记录事件，主循环根据时间间隔和当前电平确认按键，再执行页面切换和阈值修改；温度阈值步进为 1.0 C，湿度阈值步进为 1.0%，光照 ADC 阈值步进为 100。

OLED 共有四页：环境数据页、温度阈值页、湿度阈值页和光照阈值页。阈值仅保存在 RAM 中，复位或断电后恢复为 `environment.c` 中定义的默认值。

## 硬件接口

| 模块 | STM32 外设 / 引脚 | 说明 |
|---|---|---|
| 光敏电阻 | ADC3_IN6 / PF8 | 读取光照分压的 ADC 原始值 |
| 串口 | USART1: PA9(TX), PA10(RX) | 115200, 8N1 |
| 定时器 | TIM2 | 当前配置为 1 s 周期 |
| LED | PB5 | 报警指示 |
| 蜂鸣器 | PB8 | 超阈值报警 |
| 温湿度 | AHT20：I2C1 PB6(SCL)、PB7(SDA) | 7 位地址 `0x38`，HAL 地址 `0x70` |
| 显示 | OLED：SPI1 PA5(SCK)、PA7(MOSI)，PA4(CS)、PC0(RES)、PC1(DC) | 已完成基础字符显示 |
| 按键 | KEY_UP: PA0，KEY0: PE4，KEY1: PE3 | 切页、阈值减、阈值加 |

当前光敏电阻连接方式下，ADC 值越大表示环境越暗。因此程序将 `light > light_max` 判定为过暗报警。`light_max` 当前是 ADC 原始阈值，不是 lux 值。

## UART 协议

协议使用固定长度二进制帧：

```text
0xAA | CMD | PAYLOAD | CHECKSUM
```

- 帧头固定为 `0xAA`。
- `CHECKSUM` 是 `CMD` 与全部 `PAYLOAD` 字节的和，仅保留低 8 位，即模 256。
- 多字节数值使用大端序：高字节在前、低字节在后。
- 接收端对校验失败帧和未知命令帧直接丢弃，不执行阈值修改。

| 方向 | 帧格式 | 示例 | 说明 |
|---|---|---|---|
| STM32 -> PC | `AA 01` + 13 字节数据 + `CS` | 固定 16 字节 | 上传实时数据和当前三个阈值 |
| PC -> STM32 | `AA 02 P VH VL CS` | `AA 02 03 09 C4 D2` | 设置一个阈值参数 |
| STM32 -> PC | `AA 03 P VH VL CS` | `AA 03 03 09 C4 D3` | 阈值设置成功确认 |
| STM32 -> PC | `AA 04 E CS` | `AA 04 02 06` | 阈值设置失败 |

`DATA` 帧的数据区为 13 字节：

| 字节 | 含义 | 类型 / 范围 |
|---|---|---|
| `0..1` | 温度 x10 | `int16_t`，例如 `256` 表示 25.6 C |
| `2..3` | 湿度 x10 | `uint16_t`，例如 `653` 表示 65.3 % |
| `4..5` | 光照 ADC 原始值 | `uint16_t`，当前范围 0 到 4095 |
| `6` | 报警状态 | `0` 正常，`1` 报警 |
| `7..8` | 温度最大阈值 x10 | `int16_t` |
| `9..10` | 湿度最大阈值 x10 | `uint16_t` |
| `11..12` | 光照过暗 ADC 阈值 | `uint16_t` |

`SET_THRESHOLD` 与 `ACK` 的参数编号：

| 参数编号 | 阈值字段 | 有效范围 |
|---|---|---|
| `0x01` | 温度最大值 x10 | -400 到 850 |
| `0x02` | 湿度最大值 x10 | 0 到 1000 |
| `0x03` | 光照过暗 ADC 阈值 | 0 到 4095 |

`ERROR` 的错误码为 `0x01`（参数编号错误）和 `0x02`（数值超出范围）。STM32 使用 USART1 DMA 接收配合 IDLE 事件，将一批字节逐个交给协议状态机；DMA 发送完成后通过发送完成回调释放发送缓冲区。主循环负责修改阈值、更新报警和刷新 OLED。

## 目录说明

```text
EM_System/
  Core/Inc/       应用模块头文件
  Core/Src/       ADC、报警、LED、UART 和主循环实现
  Drivers/        STM32 HAL 与 CMSIS 依赖
  EM_System.ioc   CubeMX 配置文件
  MDK-ARM/        Keil MDK 工程文件

PC_Host/
  list_ports.py       查看可用串口
  serial_receive.py   旧版命令行上位机
  protocol.py         二进制帧组包与解析
  serial_worker.py    串口接收线程
  app.py              Tkinter 图形化上位机
```

## 编译与运行

### STM32 固件

1. 使用 Keil MDK 打开 `EM_System/MDK-ARM/EM_System.uvprojx`。
2. 编译并下载固件到 STM32F103 开发板。
3. 使用串口模块连接 USART1，串口参数设置为 `115200-8-N-1`。

### Python 上位机

```powershell
cd PC_Host
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install pyserial
.\.venv\Scripts\python.exe serial_receive.py
```

启动 GUI：

```powershell
.\.venv\Scripts\python.exe app.py
```

界面支持选择串口、显示温度/湿度/光照/报警状态，以及分别设置温度、湿度和光照三个阈值。温度和湿度输入使用实际单位，例如 `30.0` C、`70.0` %；协议内部仍使用 x10 定点整数。
