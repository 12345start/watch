/* LED.h */
#ifndef __LED_H
#define __LED_H

void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

#endif
/* LED.c */
#include "stm32f10x.h"
#include "LED.h"


#define LED_PORT   GPIOA
#define LED_PIN    GPIO_Pin_1
#define LED_CLK    RCC_APB2Periph_GPIOA

void LED_Init(void)
{
    RCC_APB2PeriphClockCmd(LED_CLK, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = LED_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    LED_Off();
}

void LED_On(void)     { GPIO_ResetBits(LED_PORT, LED_PIN); }
void LED_Off(void)    { GPIO_SetBits(LED_PORT, LED_PIN); }
void LED_Toggle(void)
{
    if (GPIO_ReadOutputDataBit(LED_PORT, LED_PIN))
        GPIO_ResetBits(LED_PORT, LED_PIN);
    else
        GPIO_SetBits(LED_PORT, LED_PIN);
}
