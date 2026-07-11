# 2025 National Undergraduate Electronics Design Competition - Problem E

## 项目概述

本装置用于完成 2025 电赛本科 E 题《简易自行瞄准装置》的基础部分。小车沿 100cm 正方形黑色闭环赛道逆时针自动巡线，圈数 N 可设(1-5圈)，每跑完一圈自动停顿 2s，跑完设定圈数后自动停车。

**当前状态:** 底盘代码已编译通过

---

## 系统架构

```
                      2025 电赛 E 题 简易自行瞄准装置
 子系统A: 底盘 (MSPM0G3507)     |  子系统B: 云台 (STM32F103C8T6)
  Keil 5 + DAP-Link             |  CubeMX + HAL + Keil
  独立电源                       |  独立电源
 两系统间无直接通信, 独立运行
```

## 硬件清单

| 器件 | 型号/规格 | 数量 |
|------|----------|------|
| 主控板 | MSPM0G3507 | 1 |
| 电机驱动 | TB6612FNG | 1 |
| 减速电机 | N20 直流减速 | 2 |
| 巡线传感器 | 8路灰度循迹模块 | 1 |
| IMU(预留) | MPU6050 | 1 |
| OLED | 0.96寸 SSD1306 128x64 | 1 |
| 按键 | 轻触按键 | 3 |
| 调试器 | DAP-Link (SWD) | 1 |

## 引脚定义

| 功能 | 引脚 | 说明 |
|------|------|------|
| 8路灰度CH0-CH7 | PB18 PB21 PB22 PA30 PB0 PB1 PB10 PB11 | GPIO输入, 0=白/1=黑 |
| 左电机PWM | PA12 | 软件PWM |
| 左电机IN1/IN2 | PB17/PB19 | 方向控制 |
| 右电机PWM | PA13 | 软件PWM |
| 右电机IN1/IN2 | PA16/PB24 | 方向控制 |
| MPU6050 SDA/SCL | PA0/PA1 | 软件I2C |
| OLED SDA/SCL | PA28/PA31 | 软件I2C |
| KEY_UP (+1圈) | PA18 | 上拉输入 |
| KEY_DOWN (-1圈) | PB14 | 上拉输入 |
| KEY_CONFIRM (启动) | PA2 | 上拉输入 |

## 代码结构

```
Chassis_MSPM0/
  ti_msp_dl_config.c/h       系统初始化 (GPIO电源+时钟)
  System/
    delay.c/h                SysTick延时
  Hardware/
    OLED/                    SSD1306驱动 (软件I2C)
    MPU6050/                 MPU6050驱动 (软件I2C)
    TB6612/                  TB6612电机驱动 (软件PWM)
    TrackSensor/             8路灰度循迹 (加权平均)
    Motor/                   PID巡线 + 圈数管理
    Key/                     3按键 (消抖+扫描)
  User/
    main.c                   主程序
  Keil/
    README.md                Keil工程配置
```

## 巡线算法

**偏差计算:** 加权平均法, 传感器索引0-7, 权重0-7
```
中心位置 = sum(i x detected[i]) / sum(detected[i]) - 3.5
```

**PID控制:**
```
output = Kp x error + Ki x integral + Kd x derivative
左轮 = BASE_SPEED + output
右轮 = BASE_SPEED - output
```

默认PID: Kp=30, Ki=0, Kd=50, BASE_SPEED=500

## 编译与烧录

### SDK环境

路径: `C:\ti\mspm0_sdk_2_10_00_04`

### Keil工程设置

1. 安装DFP: 打开 tools/keil/ 双击.pack文件
2. 新建工程 -> 选 MSPM0G3507
3. RTE -> 勾选 CMSIS::CORE + Device::Startup
4. 添加源文件 (共9个.c, 不包含sysinit.c和empty.c)

**Include Paths:**
```
C:\ti\mspm0_sdk_2_10_00_04\source\
C:\ti\mspm0_sdk_2_10_00_04\source\ti\driverlib
.\System
.\Hardware\OLED
.\Hardware\MPU6050
.\Hardware\TB6612
.\Hardware\TrackSensor
.\Hardware\Motor
.\Hardware\Key
.\User
```

5. 编译 -> DAP-Link烧录

## 操作说明

1. 上电 -> OLED显示 "N = ?" (默认3圈)
2. KEY_UP(+)加圈数, KEY_DOWN(-)减圈数 (范围1-5)
3. KEY_CONFIRM 确认启动
4. 小车逆时针巡线, 每圈结束停2秒
5. 跑完停车, OLED显示 "Done!"

## 常见问题

| 问题 | 解决 |
|------|------|
| 编译找不到头文件 | 检查 Include Paths 是否包含 SDK source/ 目录 |
| 小车抖动 | 增大 motor.c 中 g_PID.Kd |
| 过弯甩出 | 减小 motor.c 中 BASE_SPEED |
| key_confirm | 检查 Include Paths 是否包含 SDK source/ 目录 |
| 圈数不计数 | 检查CH6传感器是否对准起点线 |
| OLED不显示 | 检查PA28/PA31接线, 确认OLED地址0x3C |

## 引脚复用与硬件PWM

当前使用软件PWM (GPIO翻转)。如需硬件PWM:
- PA12可配置为 TIMA1_CCP0 或 TIMA0_CCP2
- PA13可配置为 TIMA1_CCP1
- 使用 `DL_Timer_initFourCCPWMMode(TIMA1, DL_TIMER_COUNT_MODE_UP)`
- 对应管脚功能码: IOMUX_PINCM12_PF_TIMA1_CCP0 (0x04), IOMUX_PINCM13_PF_TIMA1_CCP1 (0x04)

## 版本

| 日期 | 版本 | 说明 |
|------|------|------|
| 2026-07-10 | v0.1 | 初始代码框架 |
| 2026-07-11 | v0.2 | 底盘编译通过, 软件PWM+软件I2C |

2025 National Undergraduate Electronics Design Competition - Problem E
