#include "delay.h"
#include "ti_msp_dl_config.h"

static volatile uint32_t g_uwTick = 0;

void SysTick_Handler(void)
{
    g_uwTick++;
}

void Delay_Init(void)
{
    /* 32MHz / 1000 = 32000 ticks = 1ms interrupt */
    DL_SYSTICK_config(CPUCLK_FREQ / 1000);
}

void Delay_us(uint32_t us)
{
    uint32_t start = g_uwTick * 1000;
    while ((g_uwTick * 1000 - start) < us);
}

void Delay_ms(uint32_t ms)
{
    uint32_t start = g_uwTick;
    while ((g_uwTick - start) < ms);
}

uint32_t Delay_GetTick(void)
{
    return g_uwTick;
}
