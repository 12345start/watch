#ifndef __TTLCHUAN_H
#define __TTLCHUAN_H

#include "stm32f10x.h"

/* USART1: PA9=TX, PA10=RX  —— 接蓝牙模块 */
#define TTL_USART           USART1
#define TTL_USART_IRQn      USART1_IRQn
#define TTL_GPIO_PORT       GPIOA
#define TTL_TX_PIN          GPIO_Pin_9
#define TTL_RX_PIN          GPIO_Pin_10

#define TTL_RX_BUF_SIZE     128

void     TTL_Init(void);
void     TTL_SendByte(uint8_t data);
void     TTL_SendString(char *str);
void     TTL_SendArray(uint8_t *buf, uint16_t len);
uint8_t  TTL_GetRxFlag(void);
void     TTL_ClearRxFlag(void);
uint8_t* TTL_GetRxBuffer(void);
uint16_t TTL_GetRxCount(void);
void     TTL_ResetRx(void);

#endif
