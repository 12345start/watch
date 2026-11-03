#include "TTLchuan.h"

static uint8_t  TTL_RxBuffer[TTL_RX_BUF_SIZE];
static uint16_t TTL_RxCount = 0;
static uint8_t  TTL_RxFlag  = 0;

void TTL_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = TTL_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TTL_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin  = TTL_RX_PIN;
    GPIO_Init(TTL_GPIO_PORT, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_Init(TTL_USART, &USART_InitStructure);

    /* RXNE 逐字节 + IDLE 一帧结束 */
    USART_ITConfig(TTL_USART, USART_IT_RXNE, ENABLE);
    USART_ITConfig(TTL_USART, USART_IT_IDLE, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel                   = TTL_USART_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(TTL_USART, ENABLE);
}

void TTL_SendByte(uint8_t data)
{
    USART_SendData(TTL_USART, data);
    while (USART_GetFlagStatus(TTL_USART, USART_FLAG_TXE) == RESET);
}

void TTL_SendString(char *str)
{
    while (*str) TTL_SendByte(*str++);
}

void TTL_SendArray(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++) TTL_SendByte(buf[i]);
}

uint8_t TTL_GetRxFlag(void)
{
    if (TTL_RxFlag) { TTL_RxFlag = 0; return 1; }
    return 0;
}

void TTL_ClearRxFlag(void)     { TTL_RxFlag = 0; }
uint8_t* TTL_GetRxBuffer(void) { return TTL_RxBuffer; }
uint16_t TTL_GetRxCount(void)  { return TTL_RxCount; }

void TTL_ResetRx(void)
{
    TTL_RxCount = 0;
    TTL_RxFlag  = 0;
}

void USART1_IRQHandler(void)
{
    /* ① 每收到一个字节存进缓冲区 */
    if (USART_GetITStatus(TTL_USART, USART_IT_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(TTL_USART);
        if (TTL_RxCount < TTL_RX_BUF_SIZE)
        {
            TTL_RxBuffer[TTL_RxCount++] = data;
        }
        USART_ClearITPendingBit(TTL_USART, USART_IT_RXNE);
    }

    /* ② 一帧结束，通知主循环 */
    if (USART_GetITStatus(TTL_USART, USART_IT_IDLE) != RESET)
    {
        (void)TTL_USART->SR;
        (void)TTL_USART->DR;
        TTL_RxFlag = 1;
    }
}
