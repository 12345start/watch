#include "ALERT.h"
#include "Delay.h"
#include "OLED.h"
#include "KEY.h"
#include "MPU6050.h"

/* PA1 = LED */
#define ALERT_LED_PORT   GPIOA
#define ALERT_LED_PIN    GPIO_Pin_1
#define ALERT_LED_CLK    RCC_APB2Periph_GPIOA

void ALERT_Init(void)
{
    RCC_APB2PeriphClockCmd(ALERT_LED_CLK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin   = ALERT_LED_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ALERT_LED_PORT, &GPIO_InitStructure);

    ALERT_LED_Off();
}

void ALERT_LED_On(void)
{
    GPIO_ResetBits(ALERT_LED_PORT, ALERT_LED_PIN);
}

void ALERT_LED_Off(void)
{
    GPIO_SetBits(ALERT_LED_PORT, ALERT_LED_PIN);
}

/* ---------- 预警流程 ---------- */
uint8_t ALERT_Run(void)
{
    uint32_t elapsed_ms   = 0;
    uint32_t motion_ms    = 0;
    uint8_t  last_sec     = 0xFF;

    while (elapsed_ms < 15000)
    {
        /* 1) 按键检测 */
        uint8_t key = KEY_Scan();
        if (key == 1) {
            /* 取消求助 */
            OLED_Clear();
            OLED_ShowString(1, 1, "Cancelled");
            Delay_ms(800);
            return 0;
        }
        if (key == 2) {
            /* 确认求助 */
            ALERT_LED_On();
            OLED_Clear();
            OLED_ShowString(1, 1, "Help Sent!");
            OLED_ShowString(2, 1, "LED ON");
            Delay_ms(2000);
            return 1;
        }

        /* 2) 连续 8 秒运动检测 */
        if (MPU6050_GetMotionState() != STATE_SIT) {
            motion_ms += 100;
        } else {
            motion_ms = 0;   // 一旦坐下，清零
        }

        if (motion_ms >= 8000) {
            OLED_Clear();
            OLED_ShowString(1, 1, "Motion Cancel");
            Delay_ms(800);
            return 0;
        }

        /* 3) 显示剩余时间 */
        uint8_t sec_left = (15000 - elapsed_ms) / 1000;
        if (sec_left != last_sec) {
            last_sec = sec_left;
            OLED_ShowString(1, 1, "Need Help?    ");
            OLED_ShowString(2, 1, "Cancel in:");
            OLED_ShowNum(2, 11, sec_left, 2);
            OLED_ShowString(2, 13, "S ");
            OLED_ShowString(3, 1, "K1:Cancel K2:Yes");
            OLED_ShowString(4, 1, "or move 8s...   ");
        }

        Delay_ms(100);
        elapsed_ms += 100;
    }

    /* 4) 超时 = 确认求助 */
    ALERT_LED_On();
    OLED_Clear();
    OLED_ShowString(1, 1, "Timeout!");
    OLED_ShowString(2, 1, "Help Sent!");
    OLED_ShowString(3, 1, "LED ON");
    Delay_ms(2000);
    return 1;
}
