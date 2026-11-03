#include "BTSend.h"
#include "TTLchuan.h"

/* ---------- 内部工具：把 0~999 的数字转成 3 位定长字符串 ---------- */
static void u8_to_3digit(uint8_t v, char *out)
{
    if (v == 0) {
        out[0] = '-'; out[1] = '-'; out[2] = '-';
    } else {
        out[0] = '0' + (v / 100) % 10;
        out[1] = '0' + (v / 10)  % 10;
        out[2] = '0' +  v        % 10;
    }
    out[3] = '\0';
}

/* ---------- 内部工具：拼接并发送一行 ---------- */
static void send_line(const char *s)
{
    TTL_SendString((char *)s);
}

/* ---------- 初始化 ---------- */
void BT_Init(void)
{
    TTL_Init();
}

/* ---------- 系统就绪 ---------- */
void BT_SendReady(void)
{
    send_line("SYSTEM:READY\r\n");
}

/* ---------- 开始测量 ---------- */
void BT_SendMeasuring(void)
{
    send_line("STATUS:MEASURING\r\n");
}

/* ---------- 测量结果 ---------- */
void BT_SendResult(BT_Result_t *r)
{
    if (r == 0) return;

    if (r->valid && (r->spo2 > 0 || r->hr > 0))
    {
        char spo2_str[4];
        char hr_str[4];
        char buf[32];
        uint8_t i = 0;

        u8_to_3digit(r->spo2, spo2_str);
        u8_to_3digit(r->hr,   hr_str);

        /* "SPO2:xxx,HR:xxx,OK\r\n" */
        const char *p;

        p = "SPO2:"; while (*p) buf[i++] = *p++;
        buf[i++] = spo2_str[0];
        buf[i++] = spo2_str[1];
        buf[i++] = spo2_str[2];
        buf[i++] = ',';

        p = "HR:";   while (*p) buf[i++] = *p++;
        buf[i++] = hr_str[0];
        buf[i++] = hr_str[1];
        buf[i++] = hr_str[2];
        buf[i++] = ',';

        p = "OK";    while (*p) buf[i++] = *p++;
        buf[i++] = '\r';
        buf[i++] = '\n';
        buf[i]   = '\0';

        TTL_SendString(buf);
    }
    else
    {
        send_line("SPO2:---,HR:---,FAIL\r\n");
    }
}

/* ---------- 运动中止 ---------- */
void BT_SendAbort(void)
{
    send_line("STATUS:ABORT\r\n");
}

/* ---------- 自定义字符串 ---------- */
void BT_SendString(const char *str)
{
    TTL_SendString((char *)str);
}
