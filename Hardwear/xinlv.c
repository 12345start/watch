#include "stm32f10x.h"
#include "Delay.h"
#include "MAX30102MyI2C.h"
#include "MAX30102.h"
#include "OLED.h"
#include "MPU6050.h"      // ★ 新增：运动检测
#include "KEY.h"

#define HR_WIN   7
#define FS       50

#define MEASURE_SAMPLES   (FS * 60)

int32_t  ac = 0;
extern uint32_t ir_last;
uint8_t  heart_rate = 0;
uint8_t  hr_valid = 0;

static uint32_t measure_cnt  = 0;
static uint8_t  measure_done = 0;
static uint32_t hr_sum       = 0;
static uint32_t hr_num       = 0;
static uint8_t  final_hr     = 0;

/* ---------------- 找峰算心率 ---------------- */
void HeartRate_Process(int32_t ac_now)
{
    static int32_t  prev_ac = 0;
    static uint8_t  rising = 0;
    static uint32_t sample_cnt = 0;
    static uint32_t last_peak = 0;

    static uint8_t  hr_buf[HR_WIN] = {0};
    static uint8_t  hr_idx = 0;
    static uint8_t  hr_cnt = 0;

    sample_cnt++;

    if (prev_ac < 0 && ac_now >= 0)
        rising = 1;

    if (rising && ac_now > prev_ac && ac_now > 15)
    {
        if (sample_cnt - last_peak > 15)
        {
            if (last_peak != 0)
            {
                uint32_t pp = sample_cnt - last_peak;
                uint32_t hr = 60 * FS / pp;

                if (hr >= 40 && hr <= 180)
                {
                    hr_buf[hr_idx] = (uint8_t)hr;
                    hr_idx = (hr_idx + 1) % HR_WIN;
                    if (hr_cnt < HR_WIN) hr_cnt++;

                    uint8_t tmp[HR_WIN];
                    uint8_t m = hr_cnt;
                    for (uint8_t i = 0; i < m; i++) tmp[i] = hr_buf[i];
                    for (uint8_t a = 0; a < m - 1; a++)
                        for (uint8_t b = 0; b < m - 1 - a; b++)
                            if (tmp[b] > tmp[b + 1])
                            {
                                uint8_t t = tmp[b];
                                tmp[b] = tmp[b + 1];
                                tmp[b + 1] = t;
                            }

                    heart_rate = tmp[m / 2];
                    hr_valid = 1;

                    if (!measure_done)
                    {
                        hr_sum += heart_rate;
                        hr_num++;
                    }
                }
            }
            last_peak = sample_cnt;
        }
        rising = 0;
    }

    prev_ac = ac_now;

    if (!measure_done)
    {
        measure_cnt++;
        if (measure_cnt >= MEASURE_SAMPLES)
        {
            measure_done = 1;
            final_hr = (hr_num > 0) ? (uint8_t)(hr_sum / hr_num) : 0;
        }
    }
}

/* ---------------- 处理单个样本（改名，避免与 MAX30102.c 冲突） ---------------- */
static void HR_Process_Sample(uint32_t ir)
{
    static uint32_t dc = 0;
    static uint8_t  first = 1;

    if (first)
    {
        dc = ir;
        first = 0;
    }

    dc = (dc * 3 + ir) / 4;
    ac = (int32_t)ir - (int32_t)dc;
    ir_last = ir;

    HeartRate_Process(ac);
}

/* ---------------- 复位测量状态 ---------------- */
void HR_Measure_Reset(void)
{
    measure_cnt  = 0;
    measure_done = 0;
    hr_sum       = 0;
    hr_num       = 0;
    final_hr     = 0;
    heart_rate   = 0;
    hr_valid     = 0;
}

/* ========== 主测量函数：先等10秒，再阻塞约1分钟测量 ========== */
uint8_t Measure_HeartRate(void)
{
    HR_Measure_Reset();

    /* ---- 阶段1：等待 10 秒 ---- */
    for (uint8_t sec = 10; sec > 0; sec--)
    {
        if (MPU6050_GetMotionState() == STATE_RUN) {
            OLED_Clear();
            OLED_ShowString(1, 1, "Motion Detected");
            OLED_ShowString(2, 1, "HR Abort");
            Delay_ms(1500);
            return 0;
        }

        if (KEY_Scan() == 2) {
            OLED_Clear();
            OLED_ShowString(1, 1, "Key Stop");
            Delay_ms(800);
            return 0;
        }

        OLED_ShowString(1, 1, "Please wait:");
        OLED_ShowNum(1, 13, sec, 2);
        OLED_ShowString(1, 15, "S");
        OLED_ShowString(2, 1, "Measuring...");

        for (uint8_t i = 0; i < 10; i++) {
            Delay_ms(100);
        }
    }

    OLED_Clear();
    OLED_ShowString(1, 1, "Start Measure...");
    Delay_ms(500);

    /* ---- 阶段2：正式测量 1 分钟 ---- */
    while (!measure_done)
    {
        if (MPU6050_GetMotionState() == STATE_RUN) {
            OLED_Clear();
            OLED_ShowString(1, 1, "Motion Detected");
            OLED_ShowString(2, 1, "HR Abort");
            Delay_ms(1500);
            return 0;
        }

        if (KEY_Scan() == 2) {
            OLED_Clear();
            OLED_ShowString(1, 1, "Key Stop");
            Delay_ms(800);
            return 0;
        }

        if (MAX30102_DataReady())
        {
            uint16_t n = MAX30102_ReadFIFO_All();
            for (uint16_t k = 0; k < n; k++)
            {
                uint16_t idx = (sample_idx + BUF_SIZE - n + k) % BUF_SIZE;
                HR_Process_Sample(ir_buf[idx]);
            }
        }

        OLED_ShowString(1, 1, "HR:");
        if (hr_valid) {
            OLED_ShowNum(1, 4, heart_rate, 3);
            OLED_ShowString(1, 8, "bpm");
        } else {
            OLED_ShowString(1, 4, "---");
        }

        OLED_ShowString(2, 1, "Prog:");
        OLED_ShowNum(2, 6, (measure_cnt * 100) / MEASURE_SAMPLES, 3);
        OLED_ShowString(2, 9, "%");

        OLED_ShowString(3, 1, "AC:");
        OLED_ShowSignedNum(3, 4, ac * 10, 6);

        OLED_ShowString(4, 1, "IR:");
        OLED_ShowNum(4, 4, ir_last, 6);

        Delay_ms(10);
    }

    return final_hr;
}
/* ========== 显示结果 ========== */
void HR_ShowResult(uint8_t result)
{
    OLED_Clear();
    if (result > 0)
    {
        OLED_ShowString(1, 1, "Your HR is:");
        OLED_ShowNum(2, 1, result, 3);
        OLED_ShowString(2, 4, "bpm");
    }
    else
    {
        OLED_ShowString(1, 1, "Measure Failed");
        OLED_ShowString(2, 1, "Please retry");
    }
}
