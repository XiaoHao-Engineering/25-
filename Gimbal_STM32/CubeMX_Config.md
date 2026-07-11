=== Gimbal STM32 - CubeMX配置指南 ===

1. 新建工程
STM32CubeMX -> New Project -> 选 STM32F103C8T6

2. 系统配置 -> SYS: Serial Wire (PA13/PA14)
   RCC: HSE Crystal (8MHz)

3. 引脚配置
PA0(Output) X_STEP | PA1(Output) X_DIR | PA2(Output) X_EN
PA3(Output) Y_STEP | PA4(Output) Y_DIR | PA5(Output) Y_EN
PB12(Output) LASER
PB0(EXTI0) KEY_MODE | PB1(EXTI1) KEY_CONFIRM (Pull-up)
PB10(EXTI10) KEY_UP | PB11(EXTI11) KEY_DOWN (Pull-up)
PA9(USART1_TX) | PA10(USART1_RX)
PB6(I2C1_SCL) | PB7(I2C1_SDA)

4. NVIC: EXTI 0/1/10/11 + USART1 + TIM2 全部 Enable

5. I2C1: Standard 100kHz
   USART1: 115200 baud 8N1
   TIM2: Prescaler=71, Period=49 -> 20kHz

6. 时钟: HSE 8MHz -> PLL x9 -> SYSCLK 72MHz

7. 生成: MDK-ARM, Stack 0x400, Heap 0x200

8. 添加源码: Core/Src/ + Core/Inc/ 下加入
   oled.c/h, oled_font.h, key.c/h, laser.c/h
   tmc2209.c/h, stepper.c/h, uart_protocol.c/h, aim_control.c/h
   main.c (覆盖)

9. Include Paths: .\Core\Inc

10. 编译 + ST-Link 烧录
