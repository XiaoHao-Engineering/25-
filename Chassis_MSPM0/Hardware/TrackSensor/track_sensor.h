#ifndef __TRACK_SENSOR_H
#define __TRACK_SENSOR_H

#include <stdint.h>

#define TRACK_CH0 0
#define TRACK_CH1 1
#define TRACK_CH2 2
#define TRACK_CH3 3
#define TRACK_CH4 4
#define TRACK_CH5 5
#define TRACK_CH6 6
#define TRACK_CH7 7

#define TRACK_NUM_SENSORS 8

void TrackSensor_Init(void);
uint8_t TrackSensor_Read(uint8_t ch);
void TrackSensor_ReadAll(uint8_t *buf);
int16_t TrackSensor_GetPosition(void);
uint8_t TrackSensor_IsOnLine(void);

#endif
