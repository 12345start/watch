# watch
STM32F103C8T6 ,MAX30102,MPU6050,OLED,KT66368A,IIC,UART,GPIO,SWD

硬件清单
MCU：STM32F103C8T6（Cortex-M3）
心率血氧：MAX30102（I2C）
姿态：MPU6050（I2C）
显示：OLED（I2C）
蓝牙：KT6368A（UART AT 指令，BLE）
调试：SWD（ST-Link）

功能
OLED 显示心率、血氧与告警状态
MPU6050 计步与姿态判断，用于“主动抬手确认”检测
MAX30102 心率/血氧采集，支撑 RR 间期与 SDNN 计算
五层心梗预警：心率绝对值超限、HRV（SDNN）短时程下降、SpO₂ 骤降、用户主动按键求助、30s 无响应自动预警
KT6368A 蓝牙推送体征数据与紧急求助消息
固件分层：硬件抽象层 → 传感器驱动层 → 算法与主控层

工程结构
项目采用模块化、分层架构设计，基于 Keil MDK 构建，具体目录结构如下：
Start/：启动文件与内核相关代码。

User/：应用层主控逻辑。
main.c：程序入口，系统初始化与主循环调度。
stm32f10x_it.c：中断服务函数。
stm32f10x_conf.h：标准库配置头文件。

Library/：STM32F10x 标准外设库（固件库源码）。

System/：系统级支持。
Delay.c / Delay.h：精准延时函数封装。
Config.h：全局引脚与系统时钟配置宏定义。

Hardware/：硬件驱动层（各外设与传感器驱动）。
显示模块：OLED.c / .h、OLED_Font.h（OLED 驱动与字库）。
姿态模块：MPU6050.c / .h、MPU6050_f.h、MPU6050MYI2C.c / .h（MPU6050 驱动与底层软件 I2C 实现）。
心率血氧模块：MAX30102.c / .h、MAX30102MYI2C.c / .h（MAX30102 驱动与软件 I2C）、xinlv.c / .h（心率数据滤波与计算算法）。
通信模块：TTLchuan.c / .h（串口底层收发）、BTSend.c / .h（KT6368A 蓝牙 AT 指令封装与发送逻辑）。
交互模块：KEY.c / .h（按键扫描）、LED.c / .h（状态指示灯）。
业务与预警模块：Measuer.c / .h（测量状态机）、Result.c / .h（数据综合判定）、ALERT.c / .h（五层心梗预警逻辑与动作触发）。
