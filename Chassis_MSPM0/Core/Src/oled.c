#include "oled.h"
#include "oled_font.h"
#include "delay.h"
#include "ti_msp_dl_config.h"

/* ===== OLED 硬件接口: 软件 I2C (bit-bang) =====
 * 引脚: SCL = PA31 (PINCM 31), SDA = PA28 (PINCM 28)
 * I2C 地址: OLED_ADDR (oled.h 中定义, 默认 0x78 = 0x3C << 1)
 *
 * 上电前请确认:
 *   1. SCL/SDA 外部上拉电阻 (4.7k~10k 至 3.3V) —— 部分 OLED 模块自带
 *   2. OLED VCC = 3.3V, 某些模块需 5V 供电再由板上 LDO 降压
 *   3. I2C 地址: 默认 0x3C, 若模块背面 R3 跳线焊接则为 0x3D
 *      (对应 OLED_ADDR 分别改为 0x78 或 0x7A)
 */
#define OLED_SCL_HIGH() DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_31)
#define OLED_SCL_LOW()  DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_31)
#define OLED_SDA_HIGH() DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_28)
#define OLED_SDA_LOW()  DL_GPIO_clearPins(GPIOA, DL_GPIO_PIN_28)

/*
 * 引脚初始化 (PA31, PA28 -> GPIO 推挽输出)
 *
 * NOTE: 必须按以下顺序:
 *   1. initDigitalOutput: 配置 PINCM (IOMUX + DO 位)
 *   2. setPins: 设置初始电平 (输出使能前先预设)
 *   3. enableOutput: 打开 DOER, 使能输出驱动 —— 缺这一步引脚无法输出
 *
 * 参考: MSPM0 SDK gpio_toggle_output 例程
 */
static void OLED_GPIO_Init(void)
{
    DL_GPIO_initDigitalOutput(OLED_PIN_SCL_PINCM);
    DL_GPIO_initDigitalOutput(OLED_PIN_SDA_PINCM);
    OLED_SCL_HIGH();
    OLED_SDA_HIGH();
    DL_GPIO_enableOutput(GPIOA, DL_GPIO_PIN_31 | DL_GPIO_PIN_28);
}

/* ===== 软件 I2C 时序 =====
 * 基于 delay_cycles() 实现微秒级延迟, 实际 I2C 速率约 100-200 kHz.
 * SSD1306 无最低通信速率限制, 时序略宽松不影响功能.
 */

/* I2C 起始条件: SDA 下降沿, SCL 保持高 */
static void OLED_I2C_Start(void) {
    OLED_SDA_HIGH(); OLED_SCL_HIGH(); Delay_us(5);
    OLED_SDA_LOW();  Delay_us(5); OLED_SCL_LOW();
}

/* I2C 停止条件: SDA 上升沿, SCL 保持高 */
static void OLED_I2C_Stop(void) {
    OLED_SDA_LOW();  OLED_SCL_HIGH(); Delay_us(5);
    OLED_SDA_HIGH(); Delay_us(5);
}

/* 发送一个字节 (MSB first), 含 9th SCL = 从机 ACK 脉冲 */
static void OLED_I2C_SendByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        (byte & 0x80) ? OLED_SDA_HIGH() : OLED_SDA_LOW();
        Delay_us(2); OLED_SCL_HIGH(); Delay_us(2); OLED_SCL_LOW();
        byte <<= 1;
    }
    /* 释放 SDA, 产生第 9 个 SCL 供从机拉低 ACK */
    OLED_SDA_HIGH(); OLED_SCL_HIGH(); Delay_us(2); OLED_SCL_LOW();
}

/* 向 OLED 写命令 (Co=0, D/C#=0) */
static void OLED_WriteCmd(uint8_t cmd) {
    OLED_I2C_Start(); OLED_I2C_SendByte(OLED_ADDR);   /* 地址 + W */
    OLED_I2C_SendByte(0x00); /* 控制字节: Co=0, D/C#=0 (命令) */
    OLED_I2C_SendByte(cmd);  OLED_I2C_Stop();
}

/* 向 OLED 写数据 (Co=0, D/C#=1) */
static void OLED_WriteData(uint8_t dat) {
    OLED_I2C_Start(); OLED_I2C_SendByte(OLED_ADDR);   /* 地址 + W */
    OLED_I2C_SendByte(0x40); /* 控制字节: Co=0, D/C#=1 (数据) */
    OLED_I2C_SendByte(dat);  OLED_I2C_Stop();
}

/* ===== SSD1306 初始化 =====
 * 标准序列: 显示关 -> 时钟/复用/偏移 -> 电荷泵开 -> 寻址模式
 * -> 段/COM 重映射 -> 对比度 -> 预充电 -> 显示开
 */
void OLED_Init(void) {
    OLED_GPIO_Init(); Delay_ms(100);  /* 等待 OLED 上电稳定 */

    OLED_WriteCmd(0xAE); /* Display OFF */
    OLED_WriteCmd(0xD5); /* 设置显示时钟分频/振荡频率 */
    OLED_WriteCmd(0x80); /* 默认频率 */
    OLED_WriteCmd(0xA8); /* 设置多路复用比 */
    OLED_WriteCmd(0x3F); /* 64 MUX (128x64 显示) */
    OLED_WriteCmd(0xD3); /* 设置显示偏移 */
    OLED_WriteCmd(0x00); /* 无偏移 */
    OLED_WriteCmd(0x40); /* 设置起始行地址 (Row 0) */
    OLED_WriteCmd(0x8D); /* 电荷泵设置 */
    OLED_WriteCmd(0x14); /* 使能电荷泵 */
    OLED_WriteCmd(0x20); /* 设置内存寻址模式 */
    OLED_WriteCmd(0x00); /* 水平寻址模式 */
    OLED_WriteCmd(0xA1); /* 段重映射: Column 127 = SEG0 */
    OLED_WriteCmd(0xC8); /* COM 扫描方向: 从 COM[N-1] 到 COM0 */
    OLED_WriteCmd(0xDA); /* COM 引脚硬件配置 */
    OLED_WriteCmd(0x12); /* 替代引脚配置 (128x64) */
    OLED_WriteCmd(0x81); /* 设置对比度 */
    OLED_WriteCmd(0xCF); /* 对比度值 (0x00~0xFF) */
    OLED_WriteCmd(0xD9); /* 设置预充电周期 */
    OLED_WriteCmd(0xF1); /* Phase1=1 DCLK, Phase2=15 DCLKs */
    OLED_WriteCmd(0xDB); /* 设置 VCOMH 取消选择电平 */
    OLED_WriteCmd(0x40); /* VCOMH 电平 */
    OLED_WriteCmd(0xA4); /* 全局显示: 跟随 RAM 内容 */
    OLED_WriteCmd(0xA6); /* 正常显示 (非反色) */
    OLED_WriteCmd(0xAF); /* Display ON */

    OLED_Clear();        /* 清屏: 确保初始无残留 */
}

/* 清屏 (8 页 x 128 列, 全部写 0x00) */
void OLED_Clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        OLED_WriteCmd(0xB0 + page); /* 设置页地址 */
        OLED_WriteCmd(0x00);        /* 设置列低 4 位 */
        OLED_WriteCmd(0x10);        /* 设置列高 4 位 */
        for (uint8_t col = 0; col < 128; col++) OLED_WriteData(0x00);
    }
}

/* 在指定位置显示一个字符 */
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

/* 显示字符串 (自动换行) */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size) {
    while (*str) {
        OLED_ShowChar(x, y, *str, size);
        x += (size == 8) ? 6 : 8;
        if (x > 122) { x = 0; y += (size == 8) ? 1 : 2; }
        str++;
    }
}

/* 显示无符号整数 (高位补零) */
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size) {
    char buf[12];
    for (uint8_t i = 0; i < len; i++) { buf[len-1-i] = '0' + (num % 10); num /= 10; }
    buf[len] = '\0';
    OLED_ShowString(x, y, buf, size);
}

/* 显示有符号整数 (负号前缀) */
void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t num, uint8_t len, uint8_t size) {
    if (num < 0) { OLED_ShowChar(x, y, '-', size); x += (size==8)?6:8; num = -num; }
    OLED_ShowNum(x, y, num, len, size);
}
