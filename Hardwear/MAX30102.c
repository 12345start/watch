#include "stm32f10x.h"
#include "Delay.h"
#include "MAX30102MYI2C.h"
#include "MAX30102.h"
#include "OLED.h"
#include "MPU6050.h"
#include "KEY.h" 

#define MAX30102_ADDR_W   0xAE
#define MAX30102_ADDR_R   0xAF

uint32_t ir_buf[BUF_SIZE];
uint32_t red_buf[BUF_SIZE];
uint16_t sample_idx = 0;

/* 调试计数 */
uint32_t g_poll_cnt = 0;
uint32_t g_fifo_cnt = 0;

/* ---------- 基础读写 ---------- */
void MAX30102_WriteReg(uint8_t RegAddr, uint8_t Data)
{
    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte(MAX30102_ADDR_W);
    MAX30102_I2C_ReceiveAck();
    MAX30102_I2C_SendByte(RegAddr);
    MAX30102_I2C_ReceiveAck();
    MAX30102_I2C_SendByte(Data);
    MAX30102_I2C_ReceiveAck();
    MAX30102_I2C_Stop();
}

uint8_t MAX30102_ReadReg(uint8_t RegAddr)
{
    uint8_t Data;
    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte(MAX30102_ADDR_W);
    MAX30102_I2C_ReceiveAck();
    MAX30102_I2C_SendByte(RegAddr);
    MAX30102_I2C_ReceiveAck();

    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte(MAX30102_ADDR_R);
    MAX30102_I2C_ReceiveAck();
    Data = MAX30102_I2C_ReceiveByte();
    MAX30102_I2C_SendAck(1);
    MAX30102_I2C_Stop();
    return Data;
}

/* ---------- 初始化 ---------- */
void MAX30102_Init(void)
{
    uint32_t timeout;

    MAX30102_I2C_Init();
    Delay_ms(100);

    /* 软复位 */
    MAX30102_WriteReg(0x09, 0x40);
    Delay_ms(50);
    timeout = 0;
    while ((MAX30102_ReadReg(0x09) & 0x40) && (++timeout < 1000))
        Delay_ms(1);

    /* ★★★ FIFO：不平均 + 滚动 + 满 4 触发（0x0F→0x1F 打开滚动） ★★★ */
    MAX30102_WriteReg(0x08, 0x1F);

    /* SpO2：16384nA / 50Hz / 411us */
    MAX30102_WriteReg(0x0A, 0x63);

    /* LED 电流 50mA */
    MAX30102_WriteReg(0x0C, 0x3F);
    MAX30102_WriteReg(0x0D, 0x3F);

    /* 清中断状态 */
    MAX30102_ReadReg(0x00);
    MAX30102_ReadReg(0x01);

    /* 关中断，靠 FIFO 指针判断 */
    MAX30102_WriteReg(0x02, 0x00);
    MAX30102_WriteReg(0x03, 0x00);

    /* 清 FIFO 指针 */
    MAX30102_WriteReg(0x04, 0x00);
    MAX30102_WriteReg(0x05, 0x00);
    MAX30102_WriteReg(0x06, 0x00);

    /* 启动 SpO2 模式 */
    MAX30102_WriteReg(0x09, 0x03);
    Delay_ms(50);
}

uint8_t MAX30102_DataReady(void)
{
    uint8_t wr = MAX30102_ReadReg(0x04);
    uint8_t rd = MAX30102_ReadReg(0x06);
    return (wr != rd) ? 1 : 0;
}

/* ---------- 算法变量 ---------- */
#define FS       50
#define WARMUP   250
#define MEASURE_SAMPLES   (FS * 60)

uint32_t ir_last = 0;
uint32_t rd_last = 0;
int32_t  ir_ac = 0;
int32_t  rd_ac = 0;
uint32_t ir_dc = 0;
uint32_t rd_dc = 0;
uint8_t  spo2 = 0;
uint8_t  spo2_valid = 0;

static uint32_t measure_cnt  = 0;
static uint8_t  measure_done = 0;
static uint32_t spo2_sum     = 0;
static uint32_t spo2_num     = 0;
static uint8_t  final_spo2   = 0;

static int64_t  ir_dc_acc = 0;
static int64_t  rd_dc_acc = 0;
static uint8_t  first_flag = 1;
static uint16_t warm = 0;
static int32_t  ir_ac_max = 0, ir_ac_min = 0;
static int32_t  rd_ac_max = 0, rd_ac_min = 0;
static uint32_t sample_cnt = 0;

static void Process_Sample(uint32_t ir, uint32_t rd);

/* ---------- 复位测量状态 ---------- */
static void Measure_Reset(void)
{
    measure_cnt = 0; measure_done = 0;
    spo2_sum = 0; spo2_num = 0; final_spo2 = 0;
    spo2 = 0; spo2_valid = 0;
    first_flag = 1; warm = 0; sample_cnt = 0;
    ir_ac_max = ir_ac_min = 0;
    rd_ac_max = rd_ac_min = 0;
    g_poll_cnt = 0; g_fifo_cnt = 0;
}

/* ---------- 单样本算法 ---------- */
static void Process_Sample(uint32_t ir, uint32_t rd)
{
    if (first_flag) { ir_dc_acc = (int64_t)ir; rd_dc_acc = (int64_t)rd; first_flag = 0; }

    ir_dc_acc = ir_dc_acc - (ir_dc_acc >> 6) + ((int64_t)ir >> 6);
    rd_dc_acc = rd_dc_acc - (rd_dc_acc >> 6) + ((int64_t)rd >> 6);

    ir_ac = (int32_t)((int64_t)ir - ir_dc_acc);
    rd_ac = (int32_t)((int64_t)rd - rd_dc_acc);
    ir_dc = (uint32_t)ir_dc_acc;
    rd_dc = (uint32_t)rd_dc_acc;
    ir_last = ir; rd_last = rd;

    if (warm < WARMUP) { warm++; return; }

    sample_cnt++;
    if (ir_ac > ir_ac_max) ir_ac_max = ir_ac;
    if (ir_ac < ir_ac_min) ir_ac_min = ir_ac;
    if (rd_ac > rd_ac_max) rd_ac_max = rd_ac;
    if (rd_ac < rd_ac_min) rd_ac_min = rd_ac;

    if (sample_cnt % 50 == 0)
    {
        int32_t ir_pp = ir_ac_max - ir_ac_min;
        int32_t rd_pp = rd_ac_max - rd_ac_min;

        if (ir_pp > 0 && rd_pp > 0 && ir_dc > 0 && rd_dc > 0)
        {
            float ratio_ir = (float)ir_pp / (float)ir_dc;
            float ratio_rd = (float)rd_pp / (float)rd_dc;
            float R = ratio_rd / ratio_ir;
            float spo2_f = 110.0f - 25.0f * R;
            if (spo2_f > 100) spo2_f = 100;
            if (spo2_f < 70)  spo2_f = 70;
            spo2 = (uint8_t)spo2_f;
            spo2_valid = 1;
            if (!measure_done) { spo2_sum += spo2; spo2_num++; }
        }

        ir_ac_max = ir_ac; ir_ac_min = ir_ac;
        rd_ac_max = rd_ac; rd_ac_min = rd_ac;

        if (!measure_done)
        {
            measure_cnt += 50;
            if (measure_cnt >= MEASURE_SAMPLES)
            {
                measure_done = 1;
                final_spo2 = (spo2_num > 0) ? (uint8_t)(spo2_sum / spo2_num) : 0;
            }
        }
    }
}

/* ---------- 读 FIFO 并直接处理 ---------- */
uint16_t MAX30102_ReadFIFO_All(void)
{
    uint8_t wr = MAX30102_ReadReg(0x04);
    uint8_t rd = MAX30102_ReadReg(0x06);

    uint8_t n;
    if (wr >= rd) n = wr - rd;
    else          n = 32 - rd + wr;

    if (n == 0) return 0;
    if (n > 32) n = 32;

    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte(MAX30102_ADDR_W);
    MAX30102_I2C_ReceiveAck();
    MAX30102_I2C_SendByte(0x07);
    MAX30102_I2C_ReceiveAck();

    MAX30102_I2C_Start();
    MAX30102_I2C_SendByte(MAX30102_ADDR_R);
    MAX30102_I2C_ReceiveAck();

    for (uint8_t s = 0; s < n; s++)
    {
        uint8_t buf[6];
        for (uint8_t i = 0; i < 6; i++)
        {
            buf[i] = MAX30102_I2C_ReceiveByte();
            MAX30102_I2C_SendAck((s == n - 1 && i == 5) ? 1 : 0);
        }

        uint32_t red = (((uint32_t)buf[0] << 16) |
                        ((uint32_t)buf[1] << 8)  |
                         (uint32_t)buf[2]) & 0x3FFFF;
        uint32_t ir  = (((uint32_t)buf[3] << 16) |
                        ((uint32_t)buf[4] << 8)  |
                         (uint32_t)buf[5]) & 0x3FFFF;

        ir_buf[sample_idx]  = ir;
        red_buf[sample_idx] = red;
        sample_idx++;
        if (sample_idx >= BUF_SIZE) sample_idx = 0;

        Process_Sample(ir, red);    // ★ 直接处理
    }

    MAX30102_I2C_Stop();
    return n;
}

/* ---------- 主测量函数 ---------- */
uint8_t Measure_SpO2(void)
{
    Measure_Reset();
    uint32_t timeout = 0;

    while (!measure_done)
    {
        /* ★ 运动中止 */
        if (MPU6050_GetMotionState() == STATE_RUN) {
            OLED_Clear();
            OLED_ShowString(1, 1, "Motion Detected");
            OLED_ShowString(2, 1, "SpO2 Abort");
            Delay_ms(1500);
            return 0;
        }

        /* ★ KEY2 中止 */
        if (KEY_Scan() == 2) {
            OLED_Clear();
            OLED_ShowString(1, 1, "Key Stop");
            Delay_ms(800);
            return 0;
        }

        g_poll_cnt++;
        if (MAX30102_DataReady())
        {
            uint16_t n = MAX30102_ReadFIFO_All();
            g_fifo_cnt += n;
        }

        /* 调试显示 */
        uint8_t id = MAX30102_ReadReg(0xFF);
        uint8_t wr = MAX30102_ReadReg(0x04);
        uint8_t rd = MAX30102_ReadReg(0x06);

        OLED_ShowString(1, 1, "ID:");
        OLED_ShowHexNum(1, 4, id, 2);
        OLED_ShowString(1, 8, "W:");
        OLED_ShowHexNum(1, 10, wr, 2);
        OLED_ShowString(1, 13, "R:");
        OLED_ShowHexNum(1, 15, rd, 2);

        OLED_ShowString(2, 1, "Prog:");
        OLED_ShowNum(2, 6, (measure_cnt * 100) / MEASURE_SAMPLES, 3);
        OLED_ShowString(2, 9, "%");

        OLED_ShowString(3, 1, "P:");
        OLED_ShowNum(3, 3, g_poll_cnt, 4);
        OLED_ShowString(3, 8, "F:");
        OLED_ShowNum(3, 10, g_fifo_cnt, 5);

        OLED_ShowString(4, 1, "MC:");
        OLED_ShowNum(4, 4, measure_cnt, 5);

        if (++timeout > 1500) return 0;
        Delay_ms(100);
    }

    return final_spo2;
}


void Measure_ShowResult(uint8_t result)
{
    OLED_Clear();
    if (result > 0)
    {
        OLED_ShowString(1, 1, "Your SpO2 is:");
        OLED_ShowNum(2, 1, result, 3);
        OLED_ShowString(2, 4, "%");
    }
    else
    {
        OLED_ShowString(1, 1, "Measure Failed");
        OLED_ShowString(2, 1, "Please retry");
    }
}
