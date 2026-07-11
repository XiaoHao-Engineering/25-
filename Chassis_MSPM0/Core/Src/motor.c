#include "motor.h"
#include "tb6612.h"
#include "track_sensor.h"
#include "delay.h"
#include <stddef.h>
#include <math.h>

static PID_Param_t g_PID = {30.0f, 0.0f, 50.0f};
static int16_t g_LastError = 0;
static int32_t g_Integral = 0;

void PID_Init(PID_Param_t *param)
{
    if (param != NULL) { g_PID.Kp = param->Kp; g_PID.Ki = param->Ki; g_PID.Kd = param->Kd; }
    g_LastError = 0; g_Integral = 0;
}

int16_t PID_Calc(int16_t error)
{
    float P = g_PID.Kp * (float)error;
    g_Integral += error;
    if (g_Integral > 5000) g_Integral = 5000;
    if (g_Integral < -5000) g_Integral = -5000;
    float I = g_PID.Ki * (float)g_Integral;
    float D = g_PID.Kd * (float)(error - g_LastError);
    g_LastError = error;
    int16_t output = (int16_t)(P + I + D);
    if (output > 500) output = 500;
    if (output < -500) output = -500;
    return output;
}

void PID_Reset(void) { g_LastError = 0; g_Integral = 0; }
#define BASE_SPEED 500
#define LAP_DELAY_MS 2000
static uint8_t g_TargetLaps = 3, g_CurrentLap = 0, g_Finished = 0, g_IsRunning = 0, g_LapFlag = 0;

void Motor_TrackInit(void) {
    TB6612_Init();
    PID_Reset();
    g_CurrentLap = 0; g_Finished = 0; g_IsRunning = 0; g_LapFlag = 0;
}
void Motor_Start(void) { g_IsRunning = 1; g_Finished = 0; g_CurrentLap = 0; PID_Reset(); }

void Motor_TrackRun(void) {
    if (g_Finished || !g_IsRunning) return;
    int16_t pos = TrackSensor_GetPosition();
    int16_t correction = PID_Calc(pos);
    TB6612_SetSpeed(MOTOR_LEFT, BASE_SPEED + correction);
    TB6612_SetSpeed(MOTOR_RIGHT, BASE_SPEED - correction);
    uint8_t ch6 = TrackSensor_Read(TRACK_CH6);
    if (ch6 && !g_LapFlag) { g_LapFlag = 1; }
    if (!ch6 && g_LapFlag) {
        g_LapFlag = 0; g_CurrentLap++;
        if (g_CurrentLap >= g_TargetLaps) {
            TB6612_SetSpeed(MOTOR_LEFT, 0); TB6612_SetSpeed(MOTOR_RIGHT, 0);
            g_Finished = 1; g_IsRunning = 0;
        } else {
            TB6612_SetSpeed(MOTOR_LEFT, 0); TB6612_SetSpeed(MOTOR_RIGHT, 0);
            Delay_ms(LAP_DELAY_MS);
        }
    }
}
void Motor_TrackStop(void) { g_IsRunning = 0; TB6612_Brake(); }
void Motor_SetLaps(uint8_t n) { if (n>=1&&n<=5) g_TargetLaps = n; }
uint8_t Motor_GetCurrentLap(void) { return g_CurrentLap; }
uint8_t Motor_IsFinished(void) { return g_Finished; }

int8_t Gesture_Detect(float roll, float pitch) {
    static uint8_t lock = 0;
    if (lock) { if (fabs(roll)<15&&fabs(pitch)<15) lock=0; return 0; }
    if (pitch>30) { lock=1; return 2; }
    if (roll>30) { lock=1; return 1; }
    if (roll<-30) { lock=1; return -1; }
    return 0;
}
