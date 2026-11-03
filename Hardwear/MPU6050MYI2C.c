#include "stm32f10x.h"
#include "Delay.h"

void MPU6050_I2C_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_MPU6050_I2C_Init;
    GPIO_MPU6050_I2C_Init.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_MPU6050_I2C_Init.GPIO_Pin   = GPIO_Pin_11 | GPIO_Pin_10;
    GPIO_MPU6050_I2C_Init.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_MPU6050_I2C_Init);
    GPIO_SetBits(GPIOB, GPIO_Pin_11 | GPIO_Pin_10);
}

void MPU6050_I2C_W_SCL(uint16_t bit){
    GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)bit);
    Delay_us(10);
}

void MPU6050_I2C_W_SDA(uint16_t bit){
    GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)bit);
    Delay_us(10);
}

uint16_t MPU6050_I2C_R_SDA(void){
    uint16_t BITVALE;
    BITVALE = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
    Delay_us(10);
    return BITVALE;
}

void MPU6050_I2C_start(void){
    MPU6050_I2C_W_SDA(1);
    MPU6050_I2C_W_SCL(1);
    MPU6050_I2C_W_SDA(0);
    MPU6050_I2C_W_SCL(0);
}

void MPU6050_I2C_stop(void){
    MPU6050_I2C_W_SDA(0);
    MPU6050_I2C_W_SCL(1);
    MPU6050_I2C_W_SDA(1);
}

void MPU6050_I2C_SendBIT(uint8_t bit){
    uint8_t i = 0;
    for(; i < 8; i++){
        MPU6050_I2C_W_SDA(bit & (0x80 >> i));
        MPU6050_I2C_W_SCL(1);
        MPU6050_I2C_W_SCL(0);
    }
}

uint8_t MPU6050_I2C_Receive(void){
    uint8_t bit = 0;
    MPU6050_I2C_W_SDA(1);
    for(int i = 0; i < 8; i++){
        MPU6050_I2C_W_SCL(1);
        if(MPU6050_I2C_R_SDA() == 1){
            bit |= 0x80 >> i;
        }
        MPU6050_I2C_W_SCL(0);
    }
    return bit;
}

void MPU6050_I2C_SendBITACK(uint8_t bit){
    // bit: 0=ACK, 1=NACK
    if(bit)
        MPU6050_I2C_W_SDA(1);
    else
        MPU6050_I2C_W_SDA(0);
    MPU6050_I2C_W_SCL(1);
    MPU6050_I2C_W_SCL(0);
}

uint8_t MPU6050_I2C_ReceiveACK(void){
    uint8_t bit;
    MPU6050_I2C_W_SDA(1);
    MPU6050_I2C_W_SCL(1);
    bit = MPU6050_I2C_R_SDA();
    MPU6050_I2C_W_SCL(0);
    return bit;
}
