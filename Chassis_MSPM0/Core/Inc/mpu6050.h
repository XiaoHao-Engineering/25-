#ifndef __MPU6050_H
#define __MPU6050_H

#include <stdint.h>

#define MPU6050_ADDR    0xD0

void MPU6050_Init(void);
void MPU6050_GetAccel(int16_t *ax, int16_t *ay, int16_t *az);
void MPU6050_GetGyro(int16_t *gx, int16_t *gy, int16_t *gz);
float MPU6050_GetRoll(void);
float MPU6050_GetPitch(void);

#endif
