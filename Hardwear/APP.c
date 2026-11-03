#include "stm32f10x.h"                  // Device header
/* APP.c */
#include "APP.h"
#include "Config.h"
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "MAX30102.h"
#include "xinlv.h"
#include "BTSend.h"
#include "KEY.h"
#include "ALERT.h"
#include "LED.h"
#include "Result.h"
#include "Measure.h"
#include "UI.h"

typedef enum {
    SYS_IDLE = 0,
    SYS_MEASURE,
    SYS_SHOW_RESULT,
    SYS_ALERT,
    SYS_WAIT_STAND
} SysState_t;

typedef enum {
    MODE_AUTO = 0,
    MODE_MANUAL,
    MODE_WAIT
} MeasureMode_t;

static SysState_t    sys_state    = SYS_IDLE;
static MeasureMode_t measure_mode = MODE_AUTO;
static uint8_t       sit_cnt      = 0;
static Result_t      result;

void APP_Init(void)
{
    OLED_Init();
    MPU6050_Init();
    MAX30102_Init();
    BT_Init();
    KEY_Init();
    LED_Init();
    ALERT_Init();

    UI_Boot();
    Delay_ms(1000);
    OLED_Clear();
    BT_SendReady();
}

void APP_Run(void)
{
    MotionState_t motion = MPU6050_GetMotionState();
    uint8_t key = KEY_Scan();

    /* -------- 按键 -------- */
    if (key == 1) {
        measure_mode = MODE_MANUAL;
        sys_state = SYS_MEASURE;
        sit_cnt = 0;
        BT_SendMeasuring();
        return;
    }
    if (key == 2) {
        measure_mode = MODE_WAIT;
        sys_state = SYS_IDLE;
        sit_cnt = 0;
        LED_Off();
        return;
    }

    /* -------- 状态机 -------- */
    switch (sys_state)
    {
    case SYS_IDLE:
        UI_ShowIdle(measure_mode);
        if (measure_mode == MODE_AUTO && motion == STATE_SIT) {
            sit_cnt++;
            if (sit_cnt >= CFG_SIT_CONFIRM) {
                sit_cnt = 0;
                sys_state = SYS_MEASURE;
                BT_SendMeasuring();
            }
        } else {
            sit_cnt = 0;
        }
        Delay_ms(100);
        break;

    case SYS_MEASURE:
    {
        Measure_Result_t m;
        Measure_All(&m);

        if (m.aborted) {
            BT_SendAbort();
            result.valid = 0;
            result.abnormal = 0;
            sys_state = SYS_SHOW_RESULT;
        } else {
            Result_Process(&result, m.spo2, m.hr);
            sys_state = result.abnormal ? SYS_ALERT : SYS_WAIT_STAND;
        }
        break;
    }

    case SYS_ALERT:
        BT_SendString("ALERT:ABNORMAL\r\n");
        if (ALERT_Run()) {
            BT_SendString("HELP:REQUESTED\r\n");
        } else {
            LED_Off();
            BT_SendString("HELP:CANCELLED\r\n");
        }
        sys_state = SYS_WAIT_STAND;
        break;

    case SYS_WAIT_STAND:
        if (measure_mode == MODE_MANUAL || measure_mode == MODE_WAIT) {
            sys_state = SYS_IDLE;
            OLED_Clear();
        } else {
            if (motion != STATE_SIT) {
                sit_cnt++;
                if (sit_cnt >= CFG_SIT_CONFIRM) {
                    sit_cnt = 0;
                    LED_Off();
                    sys_state = SYS_IDLE;
                    OLED_Clear();
                }
            } else {
                sit_cnt = 0;
            }
            Delay_ms(200);
        }
        break;
    }
}
