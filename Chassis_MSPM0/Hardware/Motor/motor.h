#ifndef __MOTOR_H
#define __MOTOR_H

#include <stdint.h>

typedef struct {
    float Kp;
    float Ki;
    float Kd;
} PID_Param_t;

void PID_Init(PID_Param_t *param);
int16_t PID_Calc(int16_t error);
void PID_Reset(void);

void Motor_TrackInit(void);
void Motor_TrackRun(void);
void Motor_TrackStop(void);
void Motor_Start(void);

void Motor_SetLaps(uint8_t n);
uint8_t Motor_GetCurrentLap(void);
uint8_t Motor_IsFinished(void);

int8_t Gesture_Detect(float roll, float pitch);

#endif
