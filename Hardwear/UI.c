/* UI.c */
#include "UI.h"
#include "OLED.h"

void UI_Boot(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "System Ready");
    OLED_ShowString(2, 1, "Please Sit Down");
}

void UI_ShowIdle(uint8_t mode)
{
    OLED_ShowString(1, 1, "Please Sit Down");
    OLED_ShowString(2, 1, "Waiting...     ");
    OLED_ShowString(3, 1, "K1:Start K2:Stop");
    OLED_ShowString(4, 1, "Mode:");
    if (mode == 0)      OLED_ShowString(4, 7, "AUTO   ");
    else if (mode == 1) OLED_ShowString(4, 7, "MANUAL ");
    else                OLED_ShowString(4, 7, "WAIT   ");
}

void UI_ShowAlert(uint8_t sec_left)
{
    OLED_ShowString(1, 1, "Need Help?    ");
    OLED_ShowString(2, 1, "Cancel in:");
    OLED_ShowNum(2, 11, sec_left, 2);
    OLED_ShowString(2, 13, "S ");
    OLED_ShowString(3, 1, "K1:Cancel K2:Yes");
    OLED_ShowString(4, 1, "or move 8s...   ");
}

void UI_ShowHelpSent(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "Help Sent!");
    OLED_ShowString(2, 1, "LED ON");
}

void UI_ShowCancelled(void)
{
    OLED_Clear();
    OLED_ShowString(1, 1, "Cancelled");
}
