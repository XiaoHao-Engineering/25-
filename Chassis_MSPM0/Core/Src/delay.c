#include "delay.h"
#include "ti_msp_dl_config.h"

/* SysTick: 1ms 中断递增计数器, volatile 防止编译器优化中断上下文 */
static volatile uint32_t g_uwTick = 0;

void SysTick_Handler(void)
{
    g_uwTick++;
}

void Delay_Init(void)
{
    /*
     * SysTick: 配置为 1ms 周期中断, 用于 Delay_ms 和 Delay_GetTick.
     * DWT 无需额外初始化, delay_cycles() 为 SDK 内置函数.
     */
    DL_SYSTICK_config(CPUCLK_FREQ / 1000);
}

/*
 * Delay_us: 微秒级忙等待延时
 *
 * 使用 MSPM0 SDK 的 delay_cycles() 实现, 其内部为 2-cycle SUBS/BNE 循环:
 *   delay_cycles(n) 消耗大约 2n 个 CPU 周期
 *
 * 以 CPUCLK_FREQ = 32MHz 为例:
 *   1 us = 32 CPU 周期 = delay_cycles(16) 次迭代
 *   Delay_us(2):   delay_cycles(32)   -> ~64 周期 -> ~2 us
 *   Delay_us(5):   delay_cycles(80)   -> ~160 周期 -> ~5 us
 *   Delay_us(100): delay_cycles(1600) -> ~3200 周期 -> ~100 us
 *
 * 注意: 函数调用和运算有 ~10 周期固定开销, 极短延时 (< 5us) 实际值略大,
 *       但对 I2C bit-bang (SSD1306 无最低速率限制) 而言完全可接受.
 */
void Delay_us(uint32_t us)
{
    delay_cycles(us * (CPUCLK_FREQ / 2000000));
}

/* Delay_ms: 毫秒级延时, 依赖 SysTick 1ms 中断 */
void Delay_ms(uint32_t ms)
{
    uint32_t start = g_uwTick;
    while ((g_uwTick - start) < ms);
}

/* 返回系统自启动以来的毫秒数 (约 49.7 天回绕) */
uint32_t Delay_GetTick(void)
{
    return g_uwTick;
}
