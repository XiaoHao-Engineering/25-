#include "main.h"
#include "ti_msp_dl_config.h"
#include "delay.h"
#include "oled.h"
#include "key.h"
#include "track_sensor.h"
#include "motor.h"
#include "tb6612.h"

/* ---------- 显示: 圈数设置界面 ---------- */
static void Display_LapSetting(uint8_t laps)
{
    OLED_Clear();
    OLED_ShowString(20, 0, "== Set Laps ==", 8);
    OLED_ShowChar(40, 3, 'N', 16);
    OLED_ShowChar(56, 3, '=', 16);
    OLED_ShowNum(72, 3, laps, 1, 16);
    OLED_ShowString(0,  6, "UP+ DOWN- OK Go", 8);
}

/* ---------- 显示: 运行中界面 ---------- */
static void Display_Running(uint8_t currentLap, uint8_t targetLaps)
{
    OLED_Clear();
    OLED_ShowString(20, 0, "== Running ==", 8);
    OLED_ShowString(30, 3, "Lap:", 16);
    OLED_ShowNum(74, 3, currentLap, 1, 16);
    OLED_ShowChar(90, 3, '/', 16);
    OLED_ShowNum(104, 3, targetLaps, 1, 16);
}

/* ---------- 显示: 完成界面 ---------- */
static void Display_Done(void)
{
    OLED_Clear();
    OLED_ShowString(32, 2, "Done!", 16);
    OLED_ShowString(8, 6, "All laps OK", 8);
}

/* ---------- 按键设置圈数 (1~5) ---------- */
static uint8_t LapSetting_ByKey(void)
{
    uint8_t laps = 3;
    Display_LapSetting(laps);
    while (1) {
        uint8_t key = Key_Scan();
        if (key == KEY_UP)       { if (laps < 5) laps++; Display_LapSetting(laps); }
        else if (key == KEY_DOWN){ if (laps > 1) laps--; Display_LapSetting(laps); }
        else if (key == KEY_CONFIRM) break;
        Delay_ms(15);
    }
    return laps;
}

int main(void)
{
    SYSCFG_DL_init();              /* 系统时钟 + GPIO 电源 */
    Delay_Init();                  /* SysTick 1ms 定时器 */

    /*
     * 使能全局中断 —— Cortex-M0+ 复位后 PRIMASK=0 (中断已使能),
     * 此处显式调用确保 SysTick_Handler 正常工作, 消除调试器
     * 或启动代码可能屏蔽中断的隐患.
     */
    __enable_irq();

    OLED_Init();                   /* SSD1306 初始化 + 清屏 */
    Key_Init();
    TrackSensor_Init();
    Motor_TrackInit();

    OLED_ShowString(24, 1, "System OK", 8);
    Delay_ms(1000);

    /* 圈数设置 -> 启动 */
    uint8_t targetLaps = LapSetting_ByKey();
    Motor_SetLaps(targetLaps);
    Motor_Start();

    /* 主运行循环: 巡线 + 软件 PWM + OLED 更新 */
    while (!Motor_IsFinished()) {
        Motor_TrackRun();
        TB6612_Poll();        /* 软件 PWM 更新 */
        Display_Running(Motor_GetCurrentLap(), targetLaps);
        Delay_ms(1);          /* ~1ms 周期 -> 软件 PWM 频率正确 */
    }

    Display_Done();
    while (1);
}
