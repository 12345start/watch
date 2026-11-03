#include "stm32f10x.h"
#include "MPU6050MYI2C.h"          // 原 MPU6050MYI2C.h
#include "Delay.h"
#include "MPU6050_f.h"
#include "MPU6050.h"
#include <math.h>

#define MPU6050_ADDRESS 0xD0

/* ---- 陀螺仪零偏 ---- */
static int16_t g_off_x = 0, g_off_y = 0, g_off_z = 0;

/* ---- 调试变量 ---- */
int32_t g_dbg_ppA  = 0;
int32_t g_dbg_avgG = 0;

/* ================= 基础读写 ================= */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data){
    MPU6050_I2C_start();
    MPU6050_I2C_SendBIT(MPU6050_ADDRESS);
    MPU6050_I2C_ReceiveACK();
    MPU6050_I2C_SendBIT(RegAddress);
    MPU6050_I2C_ReceiveACK();
    MPU6050_I2C_SendBIT(Data);
    MPU6050_I2C_ReceiveACK();
    MPU6050_I2C_stop();
}

uint8_t MPU6050_ReadReg(uint8_t RegAddress){
    uint8_t Data;
    MPU6050_I2C_start();
    MPU6050_I2C_SendBIT(MPU6050_ADDRESS);
    MPU6050_I2C_ReceiveACK();
    MPU6050_I2C_SendBIT(RegAddress);
    MPU6050_I2C_ReceiveACK();

    MPU6050_I2C_start();
    MPU6050_I2C_SendBIT(MPU6050_ADDRESS | 0x01);
    MPU6050_I2C_ReceiveACK();
    Data = MPU6050_I2C_Receive();
    MPU6050_I2C_SendBITACK(1);
    MPU6050_I2C_stop();
    return Data;
}

/* ================= 原始数据 ================= */
void MPU6050_GETDATA(int16_t *ARRX, int16_t *ARRY, int16_t *ARRZ,
                     int16_t *DRRX, int16_t *DRRY, int16_t *DRRZ){
    uint8_t DataH, DataL;

    DataH = MPU6050_ReadReg(MPU_ACCEL_XOUTH_REG);
    DataL = MPU6050_ReadReg(MPU_ACCEL_XOUTL_REG);
    *ARRX = (DataH << 8) | DataL;

    DataH = MPU6050_ReadReg(MPU_ACCEL_YOUTH_REG);
    DataL = MPU6050_ReadReg(MPU_ACCEL_YOUTL_REG);
    *ARRY = (DataH << 8) | DataL;

    DataH = MPU6050_ReadReg(MPU_ACCEL_ZOUTH_REG);
    DataL = MPU6050_ReadReg(MPU_ACCEL_ZOUTL_REG);
    *ARRZ = (DataH << 8) | DataL;

    DataH = MPU6050_ReadReg(MPU_GYRO_XOUTH_REG);
    DataL = MPU6050_ReadReg(MPU_GYRO_XOUTL_REG);
    *DRRX = ((DataH << 8) | DataL) - g_off_x;

    DataH = MPU6050_ReadReg(MPU_GYRO_YOUTH_REG);
    DataL = MPU6050_ReadReg(MPU_GYRO_YOUTL_REG);
    *DRRY = ((DataH << 8) | DataL) - g_off_y;

    DataH = MPU6050_ReadReg(MPU_GYRO_ZOUTH_REG);
    DataL = MPU6050_ReadReg(MPU_GYRO_ZOUTL_REG);
    *DRRZ = ((DataH << 8) | DataL) - g_off_z;
}

/* ================= 陀螺仪零偏校准 ================= */
static void MPU6050_CalibrateGyro(void){
    int32_t sx = 0, sy = 0, sz = 0;
    int16_t x, y, z, ax, ay, az;
    uint8_t i;
    for (i = 0; i < 100; i++){
        MPU6050_GETDATA(&ax, &ay, &az, &x, &y, &z);
        sx += x; sy += y; sz += z;
        Delay_ms(5);
    }
    g_off_x = sx / 100;
    g_off_y = sy / 100;
    g_off_z = sz / 100;
}

/* ================= 初始化 ================= */
void MPU6050_Init(void){
    MPU6050_I2C_Init();              // 原 MYI2C_Init()

    MPU6050_WriteReg(0x6B, 0x01);   // 解除睡眠
    Delay_ms(100);
    MPU6050_WriteReg(0x6C, 0x00);
    Delay_ms(100);
    MPU6050_WriteReg(0x19, 0x09);   // 采样率 100Hz
    MPU6050_WriteReg(0x1A, 0x03);   // DLPF 44Hz（灵敏度关键）
    MPU6050_WriteReg(0x1B, 0x18);   // 陀螺仪 ±2000°/s
    MPU6050_WriteReg(0x1C, 0x08);   // 加速度 ±4g（灵敏度 8192 LSB/g）

    Delay_ms(100);
    MPU6050_CalibrateGyro();        // 上电时做一次零偏校准，务必保持静止！
}

/* ================= 运动状态判断 ================= */
#define WIN_SIZE   7            // 窗口 = 7 个采样点

#define TH_PP_SIT   2000
#define TH_PP_WALK  10000
#define TH_G_SIT    2000
#define TH_G_WALK   10000

MotionState_t MPU6050_GetMotionState(void){
    static int32_t bufA[WIN_SIZE] = {0};
    static int32_t bufG[WIN_SIZE] = {0};
    static uint8_t idx  = 0;
    static uint8_t fill = 0;

    static MotionState_t last_state = STATE_SIT;
    static uint8_t       stable_cnt = 0;

    int16_t ax, ay, az, gx, gy, gz;
    MPU6050_GETDATA(&ax, &ay, &az, &gx, &gy, &gz);

    int32_t a_total = (int32_t)sqrtf((float)ax*ax + (float)ay*ay + (float)az*az);
    int32_t g_total = (int32_t)sqrtf((float)gx*gx + (float)gy*gy + (float)gz*gz);

    bufA[idx] = a_total;
    bufG[idx] = g_total;
    idx = (idx + 1) % WIN_SIZE;
    if (fill < WIN_SIZE) fill++;

    int32_t sumG = 0;
    int32_t maxA = 0, minA = 0x7FFFFFFF;
    uint8_t i;
    for (i = 0; i < fill; i++){
        sumG += bufG[i];
        if (bufA[i] > maxA) maxA = bufA[i];
        if (bufA[i] < minA) minA = bufA[i];
    }
    int32_t avgG = sumG / fill;
    int32_t ppA  = maxA - minA;

    g_dbg_ppA  = ppA;
    g_dbg_avgG = avgG;

    MotionState_t new_state;
    if (ppA < TH_PP_SIT && avgG < TH_G_SIT){
        new_state = STATE_SIT;
    }
    else if (ppA > TH_PP_WALK || avgG > TH_G_WALK){
        new_state = STATE_RUN;
    }
    else{
        new_state = STATE_WALK;
    }

    if (new_state == last_state){
        stable_cnt = 0;
    } else {
        stable_cnt++;
        if (stable_cnt >= 2){
            last_state = new_state;
            stable_cnt = 0;
        }
    }
    return last_state;
}
