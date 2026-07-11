#include "tb6612.h"
#include "delay.h"
#include "ti_msp_dl_config.h"

/* 方向引脚 */
#define L_IN1_PORT GPIOB
#define L_IN1_PIN  DL_GPIO_PIN_17
#define L_IN2_PORT GPIOB
#define L_IN2_PIN  DL_GPIO_PIN_19
#define R_IN1_PORT GPIOA
#define R_IN1_PIN  DL_GPIO_PIN_16
#define R_IN2_PORT GPIOB
#define R_IN2_PIN  DL_GPIO_PIN_24

#define L1_H() DL_GPIO_setPins(L_IN1_PORT, L_IN1_PIN)
#define L1_L() DL_GPIO_clearPins(L_IN1_PORT, L_IN1_PIN)
#define L2_H() DL_GPIO_setPins(L_IN2_PORT, L_IN2_PIN)
#define L2_L() DL_GPIO_clearPins(L_IN2_PORT, L_IN2_PIN)
#define R1_H() DL_GPIO_setPins(R_IN1_PORT, R_IN1_PIN)
#define R1_L() DL_GPIO_clearPins(R_IN1_PORT, R_IN1_PIN)
#define R2_H() DL_GPIO_setPins(R_IN2_PORT, R_IN2_PIN)
#define R2_L() DL_GPIO_clearPins(R_IN2_PORT, R_IN2_PIN)

/* PWM 引脚: PA12, PA13 */
#define PWM_PORT  GPIOA
#define PWM_L_PIN DL_GPIO_PIN_12
#define PWM_R_PIN DL_GPIO_PIN_13
#define PWM_L_H() DL_GPIO_setPins(PWM_PORT, PWM_L_PIN)
#define PWM_L_L() DL_GPIO_clearPins(PWM_PORT, PWM_L_PIN)
#define PWM_R_H() DL_GPIO_setPins(PWM_PORT, PWM_R_PIN)
#define PWM_R_L() DL_GPIO_clearPins(PWM_PORT, PWM_R_PIN)

#define PWM_PERIOD 100
static uint16_t g_SpeedL = 0, g_SpeedR = 0;
static uint16_t g_PwmCnt = 0;

/*
 * TB6612 引脚初始化
 *
 * 注意: initDigitalOutput 后必须调用 enableOutput 打开 DOER,
 *       否则 GPIO 输出驱动不会启用, 参见 MSPM0 SDK gpio_toggle_output 例程.
 */
void TB6612_Init(void)
{
    DL_GPIO_initDigitalOutput(LEFT_IN1_PINCM);
    DL_GPIO_initDigitalOutput(LEFT_IN2_PINCM);
    DL_GPIO_initDigitalOutput(RIGHT_IN1_PINCM);
    DL_GPIO_initDigitalOutput(RIGHT_IN2_PINCM);
    DL_GPIO_initDigitalOutput(12); /* PA12 */
    DL_GPIO_initDigitalOutput(13); /* PA13 */
    L1_L(); L2_L(); R1_L(); R2_L();
    PWM_L_L(); PWM_R_L();
    /* 使能输出驱动 */
    DL_GPIO_enableOutput(L_IN1_PORT, L_IN1_PIN);
    DL_GPIO_enableOutput(L_IN2_PORT, L_IN2_PIN);
    DL_GPIO_enableOutput(R_IN1_PORT, R_IN1_PIN);
    DL_GPIO_enableOutput(R_IN2_PORT, R_IN2_PIN);
    DL_GPIO_enableOutput(PWM_PORT, PWM_L_PIN);
    DL_GPIO_enableOutput(PWM_PORT, PWM_R_PIN);
}

void TB6612_SetSpeed(uint8_t motor, int16_t speed)
{
    if (speed > 1000) speed = 1000;
    if (speed < -1000) speed = -1000;
    uint16_t absSpeed = (speed >= 0) ? speed : (-speed);
    if (motor == MOTOR_LEFT) {
        g_SpeedL = absSpeed / 10; /* 1000->100 */
        if (speed > 0)      { L1_H(); L2_L(); }
        else if (speed < 0) { L1_L(); L2_H(); }
        else                { L1_L(); L2_L(); }
    } else {
        g_SpeedR = absSpeed / 10;
        if (speed > 0)      { R1_H(); R2_L(); }
        else if (speed < 0) { R1_L(); R2_H(); }
        else                { R1_L(); R2_L(); }
    }
}

/* 在主循环中调用 (20ms周期 -> 100步 -> 500Hz PWM) */
void TB6612_Poll(void)
{
    g_PwmCnt = (g_PwmCnt + 1) % PWM_PERIOD;
    (g_PwmCnt < g_SpeedL) ? PWM_L_H() : PWM_L_L();
    (g_PwmCnt < g_SpeedR) ? PWM_R_H() : PWM_R_L();
}

void TB6612_Brake(void) { g_SpeedL = 0; g_SpeedR = 0; }
