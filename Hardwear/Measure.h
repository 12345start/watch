/* Measure.h */
#ifndef __MEASURE_H
#define __MEASURE_H

#include "stm32f10x.h"

typedef struct {
    uint8_t spo2;
    uint8_t hr;
    uint8_t aborted;    // 1 = 测量被中止（运动/按键）
} Measure_Result_t;

/* 完整测量：血氧 + 心率，阻塞约 2 分钟 */
void Measure_All(Measure_Result_t *out);

#endif
