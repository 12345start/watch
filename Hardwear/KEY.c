#include "stm32f10x.h"
#include "Delay.h"

void KEY_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/* ---------- 阻塞式（保留，不推荐在测量中用） ---------- */
uint8_t KEY_GetNum(void)
{
    uint8_t keynum = 0;

    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0) {
        Delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0);
        Delay_ms(20);
        keynum = 1;
    }
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0) {
        Delay_ms(20);
        while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) == 0);
        Delay_ms(20);
        keynum = 2;
    }

    return keynum;
}

/* ---------- ★ 非阻塞扫描（测量中可随时调用） ----------
 * 检测下降沿：上次读到 1（松开），这次读到 0（按下）→ 触发一次
 * 返回值：1=KEY1，2=KEY2，0=无
 * 注意：没有消抖，但主循环调用间隔 >10ms 时，抖动影响可忽略。
 *      若抖动严重，可把检测间隔拉长（如在测量循环里每 50ms 调一次）。
 */
uint8_t KEY_Scan(void)
{
    static uint8_t last1 = 1;
    static uint8_t last2 = 1;

    uint8_t cur1 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12);
    uint8_t cur2 = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);

    uint8_t ret = 0;

    if (last1 == 1 && cur1 == 0) ret = 1;
    if (last2 == 1 && cur2 == 0) ret = 2;

    last1 = cur1;
    last2 = cur2;

    return ret;
}
