#ifndef _KEY_H
#define _KEY_H

#include "stm32f10x.h"

void    KEY_Init(void);
uint8_t KEY_GetNum(void);    // 阻塞式（保留，不推荐在测量中用）
uint8_t KEY_Scan(void);      // ★ 非阻塞式：返回 1=KEY1按下，2=KEY2按下，0=无

#endif
