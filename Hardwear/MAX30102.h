#ifndef __MAX30102_H
#define __MAX30102_H

#include "stm32f10x.h"

#define BUF_SIZE   100

extern uint32_t ir_buf[BUF_SIZE];
extern uint32_t red_buf[BUF_SIZE];
extern uint16_t sample_idx;

extern uint32_t g_poll_cnt;
extern uint32_t g_fifo_cnt;

void     MAX30102_Init(void);
uint8_t  MAX30102_DataReady(void);
uint16_t MAX30102_ReadFIFO_All(void);
void     MAX30102_WriteReg(uint8_t RegAddr, uint8_t Data);
uint8_t  MAX30102_ReadReg(uint8_t RegAddr);

void Measure_ShowResult(uint8_t result);
uint8_t Measure_SpO2(void);

#endif
