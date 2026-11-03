#ifndef __MPU6050_F_H
#define __MPU6050_F_H

#include "stm32f10x.h"

/* ---- 寄存器地址 ---- */
#define MPU_ACCEL_XOUTH_REG  0x3B
#define MPU_ACCEL_XOUTL_REG  0x3C
#define MPU_ACCEL_YOUTH_REG  0x3D
#define MPU_ACCEL_YOUTL_REG  0x3E
#define MPU_ACCEL_ZOUTH_REG  0x3F
#define MPU_ACCEL_ZOUTL_REG  0x40
#define MPU_GYRO_XOUTH_REG   0x43
#define MPU_GYRO_XOUTL_REG   0x44
#define MPU_GYRO_YOUTH_REG   0x45
#define MPU_GYRO_YOUTL_REG   0x46
#define MPU_GYRO_ZOUTH_REG   0x47
#define MPU_GYRO_ZOUTL_REG   0x48

/* ---- 运动状态枚举 ---- */
typedef enum {
    STATE_SIT = 0,
    STATE_WALK,
    STATE_RUN
} MotionState_t;

void MPU6050_Init(void);
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);

void MPU6050_GETDATA(int16_t *ARRX, int16_t *ARRY, int16_t *ARRZ,
                     int16_t *DRRX, int16_t *DRRY, int16_t *DRRZ);

MotionState_t MPU6050_GetMotionState(void);

/* 调试用（可在 main 里 extern 后显示） */
extern int32_t g_dbg_ppA;
extern int32_t g_dbg_avgG;

#endif
