#ifndef __MAX30102_xinlv_H
#define __MAX30102_xinlv_H

#include "stm32f10x.h"

void HR_ShowResult(uint8_t result);
uint8_t Measure_HeartRate(void);
void HR_Measure_Reset(void);
void HeartRate_Process(int32_t ac_now);

/* ★ 删除 void Process_Sample(uint32_t ir); 避免与 MAX30102.c 冲突 */

#endif
