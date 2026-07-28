# STM32 环境监测终端

基于 STM32F103 的环境监测终端开发项目。当前版本已完成光照采样、定时任务调度、阈值报警、UART 二进制双向协议和 Python 控制台上位机；AHT20 温湿度、OLED 显示和按键页面切换正在开发。

## 功能进度

- [x] STM32F103 外设初始化与 CubeMX 工程配置
- [x] 光敏电阻 ADC 采样与 ADC 校准
- [x] TIM2 周期采样调度
- [x] 光照过暗阈值判断
- [x] 蜂鸣器与 LED 报警联动
- [x] USART1 周期数据上报
- [x] UART 中断接收与二进制状态机组帧、校验
- [x] Python 控制台上位机：数据显示与阈值设置
- [ ] AHT20 温湿度采集
- [ ] OLED 实时显示
- [ ] 按键页面切换
- [ ] 阈值掉电保存

## 系统结构

```text
光敏电阻
    |
    v
ADC3 -> 环境数据结构体 -> 阈值判断 -> 蜂鸣器 / LED
    |                         |
    v                         v
TIM2 周期调度             报警状态
    |
    v
USART1 <------------------------> Python 上位机
```

TIM2 中断只设置采样标志，ADC 读取、报警判断、串口发送和命令解析均在主循环执行，避免在中断中进行耗时操作。

## 硬件接口

| 模块 | STM32 外设 / 引脚 | 说明 |
|---|---|---|
| 光敏电阻 | ADC3_IN6 / PF8 | 读取光照分压的 ADC 原始值 |
| 串口 | USART1: PA9(TX), PA10(RX) | 115200, 8N1 |
| 定时器 | TIM2 | 当前配置为 1 s 周期 |
| LED | PB5 | 报警指示 |
| 蜂鸣器 | PB8 | 超阈值报警 |
| 温湿度 | 待接入 AHT20 | 计划使用 I2C |
| 显示 | 待接入 OLED | 计划使用 I2C |

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
| STM32 -> PC | `AA 01 TT TT HH HH LL LL A CS` | `AA 01 00 00 00 00 09 C4 00 CE` | 上传温度、湿度、光照和报警状态 |
| PC -> STM32 | `AA 02 P VH VL CS` | `AA 02 03 09 C4 D2` | 设置一个阈值参数 |
| STM32 -> PC | `AA 03 P VH VL CS` | `AA 03 03 09 C4 D3` | 阈值设置成功确认 |
| STM32 -> PC | `AA 04 E CS` | `AA 04 02 06` | 阈值设置失败 |

`DATA` 帧的 7 字节数据区定义如下：

| 字节 | 含义 | 类型 / 范围 |
|---|---|---|
| `TT TT` | 温度 x10 | `int16_t`，例如 `256` 表示 25.6 C |
| `HH HH` | 湿度 x10 | `uint16_t`，例如 `653` 表示 65.3 % |
| `LL LL` | 光照 ADC 原始值 | `uint16_t`，当前范围 0 到 4095 |
| `A` | 报警状态 | `0` 正常，`1` 报警 |

`SET_THRESHOLD` 与 `ACK` 的参数编号：

| 参数编号 | 阈值字段 | 有效范围 |
|---|---|---|
| `0x01` | 温度最大值 x10 | -400 到 850 |
| `0x02` | 湿度最大值 x10 | 0 到 1000 |
| `0x03` | 光照过暗 ADC 阈值 | 0 到 4095 |

`ERROR` 的错误码为 `0x01`（参数编号错误）和 `0x02`（数值超出范围）。STM32 使用逐字节 UART 接收中断完成状态机组帧；中断只接收和校验，主循环负责修改阈值、更新报警和发送 ACK/ERROR。

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
  serial_receive.py   控制台上位机
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

程序启动后输入 `0` 到 `4095` 的整数，即可下发光照过暗阈值；输入 `q` 退出。上位机会发送二进制 `SET_THRESHOLD` 帧，并显示 STM32 返回的 ACK 或 ERROR。

## 后续计划

1. 接入 AHT20，完成温度和湿度采集、换算与阈值报警。
2. 接入 OLED，显示环境参数、报警状态和阈值页面。
3. 使用按键切换 OLED 页面，并增加本地阈值配置入口。
4. 将阈值写入内部 Flash，实现掉电保存。
5. 将 Python 控制台升级为图形化上位机。
