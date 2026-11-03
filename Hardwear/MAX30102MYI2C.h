#ifndef __MAX30102_MYI2C_H
#define __MAX30102_MYI2C_H

#include "stm32f10x.h"

/* GPIO 端口和时钟配置 */
#define MAX30102_I2C_GPIO_PORT      GPIOB
#define MAX30102_I2C_GPIO_CLK       RCC_APB2Periph_GPIOB
#define MAX30102_I2C_SCL_PIN        GPIO_Pin_6
#define MAX30102_I2C_SDA_PIN        GPIO_Pin_7

/* 引脚操作宏 */
#define MAX30102_SCL_WRITE(x)       GPIO_WriteBit(MAX30102_I2C_GPIO_PORT, MAX30102_I2C_SCL_PIN, (BitAction)(x))
#define MAX30102_SDA_WRITE(x)       GPIO_WriteBit(MAX30102_I2C_GPIO_PORT, MAX30102_I2C_SDA_PIN, (BitAction)(x))
#define MAX30102_SDA_READ()         GPIO_ReadInputDataBit(MAX30102_I2C_GPIO_PORT, MAX30102_I2C_SDA_PIN)

/* 函数声明 */
void MAX30102_I2C_Init(void);
void MAX30102_I2C_Start(void);
void MAX30102_I2C_Stop(void);
void MAX30102_I2C_SendByte(uint8_t Byte);
uint8_t MAX30102_I2C_ReceiveByte(void);
void MAX30102_I2C_SendAck(uint8_t AckBit);
uint8_t MAX30102_I2C_ReceiveAck(void);

#endif
