# Leg-Track Hybrid Robot（四足 + 履带混合机器人）

Leg–Track Hybrid Robot: Innovative Integration to Conquer Challenging Terrain

一台可在“履带模式”和“四足模式”之间切换的机器人，通过手机/电脑网页遥控，并带有摄像头视频画面。

## 系统组成

| 部分 | 目录 | 芯片 / 开发环境 | 作用 |
|---|---|---|---|
| 主控 | `stm32/` | STM32F103C8，Keil MDK 工程（`stm32/USER/TIMER.uvprojx`），ST 标准库 | 驱动 2 个履带电机（TIM3 PWM）、通过 PCA9685 驱动舵机（四足步态）、OLED 显示、串口接收指令 |
| 遥控与图传 | `esp32cam/` | ESP32-CAM（AI-Thinker），PlatformIO + Arduino 框架 | 开 Wi-Fi 热点，提供网页遥控界面和 MJPEG 视频流，把指令通过串口发给 STM32 |

```
浏览器 ──Wi-Fi──> ESP32-CAM ──UART 115200──> STM32F103C8 ──> 履带电机 / 舵机 / OLED
```

## 目录结构

```
.
├── stm32/              # STM32 Keil 工程
│   ├── USER/           # main.c、工程文件 TIMER.uvprojx
│   ├── HARDWARE/       # 电机、PCA9685、OLED、IIC、24CXX、定时器、步态（DOG）等驱动
│   ├── SYSTEM/         # delay、sys、usart
│   ├── CORE/           # CMSIS 内核文件与启动文件
│   └── STM32F10x_FWLib # ST 标准外设库
└── esp32cam/           # ESP32-CAM PlatformIO 工程
    ├── src/main.cpp    # 热点、视频流、遥控接口、串口打包
    └── data/index.html # 遥控网页（上传到 SPIFFS）
```

## 通信协议（ESP32-CAM → STM32）

串口 115200。每帧：`0xA5 0x11` + 8 字节数据 + `0x5A`。以下含义来自 ESP32 端代码（`esp32cam/src/main.cpp`）：

| 字节 | 含义 |
|---|---|
| `data[0]` | 模式：0 履带 / 1 四足 |
| `data[1]` | 运动：0 停止，1 前进，2 后退，3 左转，4 右转，5 执行动作 |
| `data[2]` | 动作编号（1–6） |
| `data[3]`、`data[4]` | 网页滑块 1、2 的数值 |

## 履带电机引脚（来自 `stm32/HARDWARE/Motor.c`）

| 功能 | 引脚 |
|---|---|
| 左电机方向 | PA6、PA7 |
| 右电机方向 | PB10、PB11 |
| 左电机 PWM | PB0（TIM3 CH3） |
| 右电机 PWM | PB1（TIM3 CH4） |

PCA9685 和 OLED 的 I2C 引脚见 `stm32/HARDWARE/IIC/myiic.h`。

## 编译与烧录

**STM32**：用 Keil MDK 打开 `stm32/USER/TIMER.uvprojx`，编译后用 J-Link / ST-Link 下载。

**ESP32-CAM**：用 VS Code + PlatformIO 打开 `esp32cam/`，执行：

1. `Upload Filesystem Image`（上传 `data/` 里的网页到 SPIFFS）
2. `Upload`（烧录固件）

## 使用方法

1. 给机器人上电，用手机或电脑连接 ESP32-CAM 的 Wi-Fi 热点（名称和密码见 `esp32cam/src/main.cpp`，建议自行修改）。
2. 浏览器打开 `http://192.168.1.1`。
3. 在网页上切换履带/四足模式，使用方向键和动作按钮遥控。

## 硬件清单

| 项目 | 型号 / 说明 |
|---|---|
| 主控 | STM32F103C8 |
| 图传与遥控 | ESP32-CAM（AI-Thinker） |
| 舵机驱动 | PCA9685 |
| 显示 | OLED（型号待补充） |
| 履带电机 / 电机驱动模块 | 待补充 |
| 舵机 | 待补充 |
| 电源 | 待补充 |

## 许可证与第三方代码

本仓库暂未指定许可证。`stm32/STM32F10x_FWLib`、`stm32/CORE`、`stm32/USER/stm32f10x.h` 等为意法半导体 / ARM 提供的库文件；`stm32/SYSTEM/delay`、`stm32/HARDWARE/IIC`、`24CXX`、`STMFLASH` 等文件带有正点原子（ALIENTEK）的标识。使用时请遵守各自的许可条款。
