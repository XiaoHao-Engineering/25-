#include "oled.h"
#include "oled_font.h"
#include "delay.h"
#include "ti_msp_dl_config.h"

#define OLED_SCL_HIGH() DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_31)
#define OLED_SCL_LOW()  DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_31)
#define OLED_SDA_HIGH() DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_28)
#define OLED_SDA_LOW()  DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_28)
#define OLED_SDA_READ() DL_GPIO_readPins(GPIOA, DL_GPIO_PIN_28)

static void OLED_GPIO_Init(void)
{
    DL_GPIO_initDigitalOutput(OLED_PIN_SCL_PINCM);
    DL_GPIO_initDigitalOutput(OLED_PIN_SDA_PINCM);
    OLED_SCL_HIGH();
    OLED_SDA_HIGH();
}

static void OLED_I2C_Start(void) {
    OLED_SDA_HIGH(); OLED_SCL_HIGH(); Delay_us(5);
    OLED_SDA_LOW();  Delay_us(5); OLED_SCL_LOW();
}
static void OLED_I2C_Stop(void) {
    OLED_SDA_LOW();  OLED_SCL_HIGH(); Delay_us(5);
    OLED_SDA_HIGH(); Delay_us(5);
}
static void OLED_I2C_SendByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        (byte & 0x80) ? OLED_SDA_HIGH() : OLED_SDA_LOW();
        Delay_us(2); OLED_SCL_HIGH(); Delay_us(2); OLED_SCL_LOW();
        byte <<= 1;
    }
    OLED_SDA_HIGH(); OLED_SCL_HIGH(); Delay_us(2); OLED_SCL_LOW();
}
static void OLED_WriteCmd(uint8_t cmd) {
    OLED_I2C_Start(); OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x00); OLED_I2C_SendByte(cmd); OLED_I2C_Stop();
}
static void OLED_WriteData(uint8_t dat) {
    OLED_I2C_Start(); OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x40); OLED_I2C_SendByte(dat); OLED_I2C_Stop();
}

void OLED_Init(void) {
    OLED_GPIO_Init(); Delay_ms(100);
    OLED_WriteCmd(0xAE); OLED_WriteCmd(0xD5); OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8); OLED_WriteCmd(0x3F); OLED_WriteCmd(0xD3); OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40); OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14);
    OLED_WriteCmd(0x20); OLED_WriteCmd(0x00); OLED_WriteCmd(0xA1); OLED_WriteCmd(0xC8);
    OLED_WriteCmd(0xDA); OLED_WriteCmd(0x12); OLED_WriteCmd(0x81); OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9); OLED_WriteCmd(0xF1); OLED_WriteCmd(0xDB); OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA4); OLED_WriteCmd(0xA6); OLED_WriteCmd(0xAF);
    OLED_Clear();
}

void OLED_Clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        OLED_WriteCmd(0xB0 + page); OLED_WriteCmd(0x00); OLED_WriteCmd(0x10);
        for (uint8_t col = 0; col < 128; col++) OLED_WriteData(0x00);
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size) {
    uint8_t c = ch - 0x20; if (c >= 95) c = 0;
    if (size == 8) {
        OLED_WriteCmd(0xB0 + y); OLED_WriteCmd(((x&0xF0)>>4)|0x10); OLED_WriteCmd(x&0x0F);
        for (uint8_t i = 0; i < 6; i++) OLED_WriteData(g_F6x8[c][i]);
    } else {
        OLED_WriteCmd(0xB0 + y); OLED_WriteCmd(((x&0xF0)>>4)|0x10); OLED_WriteCmd(x&0x0F);
        for (uint8_t i = 0; i < 8; i++) OLED_WriteData(g_F8x16[c][i]);
        OLED_WriteCmd(0xB0 + y + 1); OLED_WriteCmd(((x&0xF0)>>4)|0x10); OLED_WriteCmd(x&0x0F);
        for (uint8_t i = 8; i < 16; i++) OLED_WriteData(g_F8x16[c][i]);
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size) {
    while (*str) {
        OLED_ShowChar(x, y, *str, size);
        x += (size == 8) ? 6 : 8;
        if (x > 122) { x = 0; y += (size == 8) ? 1 : 2; }
        str++;
    }
}

void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size) {
    char buf[12];
    for (uint8_t i = 0; i < len; i++) { buf[len-1-i] = '0' + (num % 10); num /= 10; }
    buf[len] = '\0';
    OLED_ShowString(x, y, buf, size);
}

void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size) {
    if (num < 0) { OLED_ShowChar(x, y, '-', size); x += (size==8)?6:8; num = -num; }
    OLED_ShowNum(x, y, num, len, size);
}
