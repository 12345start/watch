/* Result.c */
#include "Result.h"
#include "OLED.h"
#include "BTSend.h"
#include "Config.h"
#include "Delay.h"      // ★ 新增

void Result_Fill(Result_t *r, uint8_t spo2, uint8_t hr)
{
    r->spo2     = spo2;
    r->hr       = hr;
    r->valid    = (spo2 > 0 || hr > 0) ? 1 : 0;
    r->abnormal = Result_IsAbnormal(spo2, hr);
}

uint8_t Result_IsAbnormal(uint8_t spo2, uint8_t hr)
{
    if (spo2 > 0 && spo2 < CFG_SPO2_LOW) return 1;
    if (hr > 0 && (hr < CFG_HR_LOW || hr > CFG_HR_HIGH)) return 1;
    return 0;
}

void Result_Show(Result_t *r)
{
    OLED_Clear();

    if (!r->valid) {
        OLED_ShowString(1, 1, "Measure Aborted");
        OLED_ShowString(2, 1, "or Failed");
        OLED_ShowString(3, 1, "Please stay still");
        OLED_ShowString(4, 1, "and retry");
        Delay_ms(10000);          // ★ 无效结果也停 10 秒
        return;
    }

    OLED_ShowString(1, 1, "=== Result ===");

    OLED_ShowString(2, 1, "SpO2:");
    if (r->spo2 > 0) {
        OLED_ShowNum(2, 6, r->spo2, 3);
        OLED_ShowString(2, 9, "%");
    } else {
        OLED_ShowString(2, 6, "---");
    }

    OLED_ShowString(3, 1, "HR:");
    if (r->hr > 0) {
        OLED_ShowNum(3, 4, r->hr, 3);
        OLED_ShowString(3, 8, "bpm");
    } else {
        OLED_ShowString(3, 4, "---");
    }

    if (r->abnormal) {
        OLED_ShowString(4, 1, "! ABNORMAL !");
        /* 异常：不停留，立即进入预警 */
    }
    else {
        OLED_ShowString(4, 1, "Sent to Phone");
        Delay_ms(10000);          // ★ 正常：停留 10 秒
    }
}

void Result_SendBT(Result_t *r)
{
    BT_Result_t bt;
    bt.spo2  = r->spo2;
    bt.hr    = r->hr;
    bt.valid = r->valid;
    BT_SendResult(&bt);
}

/* ★ 先发蓝牙，再显示 + 停留 */
void Result_Process(Result_t *r, uint8_t spo2, uint8_t hr)
{
    Result_Fill(r, spo2, hr);     // 1. 填充结构体
    Result_SendBT(r);             // 2. 先发蓝牙（手机立即收到）
    Result_Show(r);               // 3. 再显示 + 停留 10 秒
}

