#include "key.h"
#include "delay.h"
#include "ti_msp_dl_config.h"

static struct {
    GPIO_Regs *port;
    uint32_t   pin;
    uint8_t    lastState;
    uint32_t   pincmIndex;
} g_Keys[3] = {
    {GPIOA, DL_GPIO_PIN_18, 1, KEY_UP_PINCM},
    {GPIOB, DL_GPIO_PIN_14, 1, KEY_DOWN_PINCM},
    {GPIOA, DL_GPIO_PIN_2,  1, KEY_CONFIRM_PINCM},
};

#define KEY_READ(idx) \
    ((DL_GPIO_readPins(g_Keys[idx].port, g_Keys[idx].pin) == 0) ? 0 : 1)

void Key_Init(void)
{
    for (uint8_t i = 0; i < 3; i++) {
        DL_GPIO_initDigitalInput(g_Keys[i].pincmIndex);
    }
}

uint8_t Key_Scan(void)
{
    static uint32_t lastTick = 0;
    uint32_t now = Delay_GetTick();
    if (now - lastTick < 15) return KEY_NONE;
    lastTick = now;

    for (uint8_t i = 0; i < 3; i++) {
        uint8_t cur = KEY_READ(i);
        if (cur == 0 && g_Keys[i].lastState == 1) {
            g_Keys[i].lastState = 0;
            return i;
        }
        g_Keys[i].lastState = cur;
    }
    return KEY_NONE;
}

uint8_t Key_WaitPress(void)
{
    uint8_t key;
    do { key = Key_Scan(); } while (key == KEY_NONE);
    return key;
}
