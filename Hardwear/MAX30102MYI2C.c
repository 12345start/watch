#include "MAX30102MYI2C.h"
#include "Delay.h"

static void MAX30102_I2C_Delay(void)
{
    uint8_t i = 10;
    while (i--);
}

static void MAX30102_SDA_Mode(uint8_t Out)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = MAX30102_I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    if (Out)
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    else
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(MAX30102_I2C_GPIO_PORT, &GPIO_InitStructure);
}

void MAX30102_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(MAX30102_I2C_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = MAX30102_I2C_SCL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(MAX30102_I2C_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MAX30102_I2C_SDA_PIN;
    GPIO_Init(MAX30102_I2C_GPIO_PORT, &GPIO_InitStructure);

    MAX30102_SCL_WRITE(1);
    MAX30102_SDA_WRITE(1);
}

void MAX30102_I2C_Start(void)
{
    MAX30102_SDA_Mode(1);
    MAX30102_SDA_WRITE(1);
    MAX30102_SCL_WRITE(1);
    MAX30102_I2C_Delay();
    MAX30102_SDA_WRITE(0);
    MAX30102_I2C_Delay();
    MAX30102_SCL_WRITE(0);
    MAX30102_I2C_Delay();
}

void MAX30102_I2C_Stop(void)
{
    MAX30102_SDA_Mode(1);
    MAX30102_SDA_WRITE(0);
    MAX30102_SCL_WRITE(1);
    MAX30102_I2C_Delay();
    MAX30102_SDA_WRITE(1);
    MAX30102_I2C_Delay();
}

void MAX30102_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    MAX30102_SDA_Mode(1);
    for (i = 0; i < 8; i++)
    {
        MAX30102_SDA_WRITE(Byte & (0x80 >> i));
        MAX30102_I2C_Delay();
        MAX30102_SCL_WRITE(1);
        MAX30102_I2C_Delay();
        MAX30102_SCL_WRITE(0);
        MAX30102_I2C_Delay();
    }
}

uint8_t MAX30102_I2C_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;
    MAX30102_SDA_Mode(1);       // 先切到输出模式
    MAX30102_SDA_WRITE(1);      // ★ 释放 SDA（开漏写1）
    MAX30102_SDA_Mode(0);       // ★ 再切到输入模式（IPU）
    for (i = 0; i < 8; i++)
    {
        MAX30102_SCL_WRITE(1);
        MAX30102_I2C_Delay();
        if (MAX30102_SDA_READ() == 1)
            Byte |= (0x80 >> i);
        MAX30102_SCL_WRITE(0);
        MAX30102_I2C_Delay();
    }
    return Byte;
}

void MAX30102_I2C_SendAck(uint8_t AckBit)
{
    MAX30102_SDA_Mode(1);
    MAX30102_SDA_WRITE(AckBit);
    MAX30102_I2C_Delay();
    MAX30102_SCL_WRITE(1);
    MAX30102_I2C_Delay();
    MAX30102_SCL_WRITE(0);
    MAX30102_I2C_Delay();
}

uint8_t MAX30102_I2C_ReceiveAck(void)
{
    uint8_t AckBit;
    MAX30102_SDA_Mode(1);       // 先切输出
    MAX30102_SDA_WRITE(1);      // ★ 释放
    MAX30102_SDA_Mode(0);       // ★ 再切输入
    MAX30102_I2C_Delay();
    MAX30102_SCL_WRITE(1);
    MAX30102_I2C_Delay();
    AckBit = MAX30102_SDA_READ();
    MAX30102_SCL_WRITE(0);
    MAX30102_I2C_Delay();
    return AckBit;
}
