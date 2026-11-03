#include "stm32f10x.h"                  // Device header
/* Measure.c */
#include "Measure.h"
#include "MAX30102.h"
#include "xinlv.h"
#include "OLED.h"
#include "Delay.h"
#include "MPU6050.h"

void Measure_All(Measure_Result_t *out)
{
    out->spo2    = 0;
    out->hr      = 0;
    out->aborted = 0;

    /* 测血氧 */
    out->spo2 = Measure_SpO2();

    if (out->spo2 == 0 && MPU6050_GetMotionState() == STATE_RUN) {
        out->aborted = 1;
        return;
    }

    /* 测心率 */
    out->hr = Measure_HeartRate();
}