#ifndef __ALERT_H
#define __ALERT_H

#include "stm32f10x.h"

void    ALERT_Init(void);        // 初始化 PA1 LED
void    ALERT_LED_On(void);
void    ALERT_LED_Off(void);

/* 运行预警流程（阻塞），返回：
 *   1 = 用户确认求助（按 KEY2 或超时）
 *   0 = 用户取消（按 KEY1 或连续 8 秒运动）
 */
uint8_t ALERT_Run(void);

#endif
