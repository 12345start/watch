/* Result.h */
#ifndef __RESULT_H
#define __RESULT_H

#include "stm32f10x.h"

typedef struct {
    uint8_t spo2;       // 0 = 无效
    uint8_t hr;         // 0 = 无效
    uint8_t valid;      // 1 = 有结果
    uint8_t abnormal;   // 1 = 异常
} Result_t;

/* 采集结果 → 填充结构体 → 返回是否有效 */
void Result_Fill(Result_t *r, uint8_t spo2, uint8_t hr);

/* 判断是否异常（读 Config.h 阈值） */
uint8_t Result_IsAbnormal(uint8_t spo2, uint8_t hr);

/* 显示到 OLED */
void Result_Show(Result_t *r);

/* 通过蓝牙发送 */
void Result_SendBT(Result_t *r);

/* 一步到位：填充 + 显示 + 发送 */
void Result_Process(Result_t *r, uint8_t spo2, uint8_t hr);

#endif
