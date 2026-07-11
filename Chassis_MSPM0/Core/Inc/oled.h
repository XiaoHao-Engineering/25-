#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

/* ===== OLED (SSD1306, 128x64) 软件 I2C 驱动 =====
 *
 * I2C 引脚: SCL = PA31, SDA = PA28 (软件 bit-bang)
 * 默认 I2C 地址: 0x3C (7-bit) / 0x78 (8-bit 写地址)
 *
 * 如果 OLED 模块背面 R3 电阻已焊接, 则地址为 0x3D / 0x7A,
 * 请将下方 OLED_ADDR 改为 0x7A 并重新编译.
 *
 *   #define OLED_ADDR    0x78    // OLED 地址 0x3C (默认)
 *   #define OLED_ADDR    0x7A    // OLED 地址 0x3D (备选)
 */
#define OLED_ADDR    0x78

/* PINCM 索引 (PA31, PA28) */
#define OLED_PIN_SCL_PINCM  31
#define OLED_PIN_SDA_PINCM  28

/* ---------- 基本函数 ---------- */
void OLED_Init(void);                           /* 初始化并清屏 */
void OLED_Clear(void);                          /* 全屏清空 */

/* ---------- 显示函数 ---------- */
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size);
/* 在 (x, y) 处显示字符 ch, size=8 (6x8) 或 16 (8x16) */

void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size);
/* 在 (x, y) 处显示字符串, 自动换行 */

void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
/* 在 (x, y) 处显示 len 位无符号整数, 高位补零 */

void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size);
/* 在 (x, y) 处显示有符号整数, 负号前缀 */

#endif
