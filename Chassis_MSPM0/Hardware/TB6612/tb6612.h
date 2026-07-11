#ifndef __TB6612_H
#define __TB6612_H

#include <stdint.h>

#define MOTOR_LEFT  0
#define MOTOR_RIGHT 1

#define LEFT_IN1_PINCM  49
#define LEFT_IN2_PINCM  51
#define RIGHT_IN1_PINCM 16
#define RIGHT_IN2_PINCM 56

void TB6612_Init(void);
void TB6612_SetSpeed(uint8_t motor, int16_t speed);
void TB6612_Poll(void);  /* 在主循环中调用, 生成PWM */
void TB6612_Brake(void);

#endif
