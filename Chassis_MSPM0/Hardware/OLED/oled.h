#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

#define OLED_ADDR    0x78

/* PINCM索引 (用于DL_GPIO_initDigitalOutput) */
#define OLED_PIN_SCL_PINCM  31   /* PA31 */
#define OLED_PIN_SDA_PINCM  28   /* PA28 */

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size);

#endif
