#ifndef __BTSEND_H
#define __BTSEND_H

#include "stm32f10x.h"

/* 测量结果结构体 */
typedef struct {
    uint8_t  spo2;      // 血氧，0 表示无效
    uint8_t  hr;        // 心率，0 表示无效
    uint8_t  valid;     // 1=有效，0=失败/中止
} BT_Result_t;

/* 初始化（内部调用 TTL_Init） */
void BT_Init(void);

/* 发送：系统就绪 */
void BT_SendReady(void);

/* 发送：开始测量 */
void BT_SendMeasuring(void);

/* 发送：测量结果 */
void BT_SendResult(BT_Result_t *r);

/* 发送：运动中止 */
void BT_SendAbort(void);

/* 发送：自定义字符串（可选） */
void BT_SendString(const char *str);

#endif
