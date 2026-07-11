#include "mpu6050.h"
#include "delay.h"
#include "ti_msp_dl_config.h"

/* 软件I2C: PA0=SDA, PA1=SCL */
#define I2C_SDA_PORT GPIOA
#define I2C_SDA_PIN  DL_GPIO_PIN_0
#define I2C_SCL_PORT GPIOA
#define I2C_SCL_PIN  DL_GPIO_PIN_1

#define SDA_HIGH() DL_GPIO_setPins(I2C_SDA_PORT, I2C_SDA_PIN)
#define SDA_LOW()  DL_GPIO_clearPins(I2C_SDA_PORT, I2C_SDA_PIN)
#define SCL_HIGH() DL_GPIO_setPins(I2C_SCL_PORT, I2C_SCL_PIN)
#define SCL_LOW()  DL_GPIO_clearPins(I2C_SCL_PORT, I2C_SCL_PIN)
#define SDA_READ() DL_GPIO_readPins(I2C_SDA_PORT, I2C_SDA_PIN)

#define MPU_ADDR 0xD0
#define SMPLRT_DIV 0x19
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

static void I2C_Start(void) {
    SDA_HIGH(); SCL_HIGH(); Delay_us(5);
    SDA_LOW(); Delay_us(5); SCL_LOW();
}
static void I2C_Stop(void) {
    SDA_LOW(); SCL_HIGH(); Delay_us(5);
    SDA_HIGH(); Delay_us(5);
}
static void I2C_SendByte(uint8_t byte) {
    for (uint8_t i=0;i<8;i++) {
        (byte&0x80)?SDA_HIGH():SDA_LOW();
        Delay_us(2); SCL_HIGH(); Delay_us(2); SCL_LOW();
        byte<<=1;
    }
    SDA_HIGH(); SCL_HIGH(); Delay_us(2);
    SCL_LOW();
}
static void I2C_ReadBytes(uint8_t *buf, uint8_t len) {
    for (uint8_t i=0;i<len;i++) {
        uint8_t byte=0;
        for (uint8_t b=0;b<8;b++) {
            SCL_HIGH(); Delay_us(2);
            byte = (byte<<1)|(SDA_READ()?1:0);
            SCL_LOW(); Delay_us(2);
        }
        SDA_HIGH(); SCL_HIGH(); Delay_us(2); SCL_LOW();
        buf[i]=byte;
    }
}

static void MPU_WriteReg(uint8_t reg, uint8_t data) {
    I2C_Start(); I2C_SendByte(MPU_ADDR);
    I2C_SendByte(reg); I2C_SendByte(data); I2C_Stop();
}
static void MPU_ReadRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
    I2C_Start(); I2C_SendByte(MPU_ADDR);
    I2C_SendByte(reg); I2C_Stop();
    I2C_Start(); I2C_SendByte(MPU_ADDR|1);
    I2C_ReadBytes(buf, len); I2C_Stop();
}

void MPU6050_Init(void) {
    DL_GPIO_initDigitalOutput(0); SDA_HIGH();
    DL_GPIO_initDigitalOutput(1); SCL_HIGH();
    Delay_ms(50);
    MPU_WriteReg(PWR_MGMT_1, 0x00); Delay_ms(10);
    MPU_WriteReg(GYRO_CONFIG, 0x18);
    MPU_WriteReg(ACCEL_CONFIG, 0x00);
    MPU_WriteReg(CONFIG, 0x03);
    MPU_WriteReg(SMPLRT_DIV, 0x07);
}

void MPU6050_GetAccel(int16_t *ax, int16_t *ay, int16_t *az) {
    uint8_t buf[6];
    MPU_ReadRegs(ACCEL_XOUT_H, buf, 6);
    *ax=(int16_t)(buf[0]<<8)|buf[1];
    *ay=(int16_t)(buf[2]<<8)|buf[3];
    *az=(int16_t)(buf[4]<<8)|buf[5];
}
void MPU6050_GetGyro(int16_t *gx, int16_t *gy, int16_t *gz) {
    uint8_t buf[6];
    MPU_ReadRegs(0x43, buf, 6);
    *gx=(int16_t)(buf[0]<<8)|buf[1];
    *gy=(int16_t)(buf[2]<<8)|buf[3];
    *gz=(int16_t)(buf[4]<<8)|buf[5];
}
float MPU6050_GetRoll(void) {
    int16_t ax,ay,az; MPU6050_GetAccel(&ax,&ay,&az);
    return (float)(atan2((double)ay,(double)az)*180.0/3.14159);
}
float MPU6050_GetPitch(void) {
    int16_t ax,ay,az; MPU6050_GetAccel(&ax,&ay,&az);
    return (float)(atan2(-(double)ax,sqrt((double)ay*ay+(double)az*az))*180.0/3.14159);
}
