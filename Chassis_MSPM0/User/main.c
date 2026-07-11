#include "main.h"
#include "ti_msp_dl_config.h"
#include "delay.h"
#include "oled.h"
#include "key.h"
#include "track_sensor.h"
#include "motor.h"
#include "tb6612.h"

static void Display_LapSetting(uint8_t laps)
{
    OLED_Clear();
    OLED_ShowString(20, 0, "== Set Laps ==", 8);
    OLED_ShowChar(40, 3, 'N', 16);
    OLED_ShowChar(56, 3, '=', 16);
    OLED_ShowNum(72, 3, laps, 1, 16);
    OLED_ShowString(0,  6, "UP+ DOWN- OK Go", 8);
}
static void Display_Running(uint8_t cur, uint8_t target)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "Tracking...", 8);
    OLED_ShowString(0, 3, "Lap:", 8);
    OLED_ShowNum(40, 3, cur + 1, 1, 8);
    OLED_ShowChar(52, 3, '/', 8);
    OLED_ShowNum(64, 3, target, 1, 8);
}
static void Display_Done(void)
{
    OLED_Clear();
    OLED_ShowString(32, 2, "Done!", 16);
    OLED_ShowString(8, 6, "All laps OK", 8);
}

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
    SYSCFG_DL_init();
    Delay_Init();
    OLED_Init();
    Key_Init();
    TrackSensor_Init();
    Motor_TrackInit();

    OLED_ShowString(24, 1, "System OK", 8);
    Delay_ms(1000);

    uint8_t targetLaps = LapSetting_ByKey();
    Motor_SetLaps(targetLaps);
    Motor_Start();

    while (!Motor_IsFinished()) {
        Motor_TrackRun();
        TB6612_Poll();        /* 软件PWM更新 */
        Display_Running(Motor_GetCurrentLap(), targetLaps);
        Delay_ms(1);          /* ~1ms周期 -> 软件PWM频率正确 */
    }

    Display_Done();
    while (1);
}
