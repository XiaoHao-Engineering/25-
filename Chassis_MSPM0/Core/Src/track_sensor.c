#include "track_sensor.h"
#include "delay.h"
#include "ti_msp_dl_config.h"

static const struct {
    GPIO_Regs *port;
    uint32_t   pin;
} g_TrackPins[TRACK_NUM_SENSORS] = {
    {GPIOB, DL_GPIO_PIN_18}, {GPIOB, DL_GPIO_PIN_21},
    {GPIOB, DL_GPIO_PIN_22}, {GPIOA, DL_GPIO_PIN_30},
    {GPIOB, DL_GPIO_PIN_0},  {GPIOB, DL_GPIO_PIN_1},
    {GPIOB, DL_GPIO_PIN_10}, {GPIOB, DL_GPIO_PIN_11},
};

static const uint32_t g_TrackPincm[TRACK_NUM_SENSORS] = {
    50, 53, 54, 30, 32, 33, 42, 43
};

void TrackSensor_Init(void)
{
    for (uint8_t i = 0; i < TRACK_NUM_SENSORS; i++) {
        DL_GPIO_initDigitalInput(g_TrackPincm[i]);
    }
}

uint8_t TrackSensor_Read(uint8_t ch)
{
    if (ch >= TRACK_NUM_SENSORS) return 0;
    return (DL_GPIO_readPins(g_TrackPins[ch].port, g_TrackPins[ch].pin) != 0) ? 0 : 1;
}

void TrackSensor_ReadAll(uint8_t *buf)
{
    for (uint8_t i = 0; i < TRACK_NUM_SENSORS; i++) {
        buf[i] = (DL_GPIO_readPins(g_TrackPins[i].port, g_TrackPins[i].pin) != 0) ? 0 : 1;
    }
}

int16_t TrackSensor_GetPosition(void)
{
    uint8_t val[TRACK_NUM_SENSORS];
    TrackSensor_ReadAll(val);
    uint16_t sum = 0, weighted = 0;
    for (uint8_t i = 0; i < TRACK_NUM_SENSORS; i++) {
        sum += val[i];
        weighted += i * val[i];
    }
    if (sum == 0) return 0;
    float center = (float)weighted / (float)sum;
    int16_t pos = (int16_t)((center - 3.5f) * 1000.0f);
    if (pos > 3500) pos = 3500;
    if (pos < -3500) pos = -3500;
    return pos;
}

uint8_t TrackSensor_IsOnLine(void)
{
    uint8_t val[TRACK_NUM_SENSORS];
    TrackSensor_ReadAll(val);
    for (uint8_t i = 0; i < TRACK_NUM_SENSORS; i++)
        if (val[i]) return 1;
    return 0;
}
