#ifndef __MPU6050_I2C_H
#define __MPU6050_I2C_H

void MPU6050_I2C_Init(void);
void MPU6050_I2C_start(void);
void MPU6050_I2C_stop(void);
void MPU6050_I2C_SendBIT(uint8_t bit);
uint8_t MPU6050_I2C_Receive(void);
void MPU6050_I2C_SendBITACK(uint8_t bit);
uint8_t MPU6050_I2C_ReceiveACK(void);

#endif
