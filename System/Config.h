#ifndef __CONFIG_H
#define __CONFIG_H

/* ========== 测量参数 ========== */
#define CFG_FS              50      // 采样率 Hz
#define CFG_WARMUP          250     // 血氧预热样本数
#define CFG_MEASURE_SEC     60      // 测量时长（秒）

/* ========== 预警阈值 ========== */
#define CFG_SPO2_LOW        90      // 血氧低于此值报警
#define CFG_HR_LOW          50      // 心率低于此值报警
#define CFG_HR_HIGH         120     // 心率高于此值报警

/* ========== 预警时间 ========== */
#define CFG_ALERT_TIMEOUT   15000   // 15 秒超时
#define CFG_ALERT_MOTION    8000    // 8 秒运动取消

/* ========== 坐下检测 ========== */
#define CFG_SIT_CONFIRM     3       // 连续 N 次确认坐下
#define CFG_SIT_PP_TH       2000    // 坐下加速度峰峰值阈值
#define CFG_SIT_G_TH        2000    // 坐下陀螺仪均值阈值

/* ========== 蓝牙 ========== */
#define CFG_BT_BAUD         115200

#endif
