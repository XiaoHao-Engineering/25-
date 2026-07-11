#include "ti_msp_dl_config.h"

void SYSCFG_DL_SYSCTL_init(void)
{
    /* Use default 32MHz SYSOSC - configured by boot code */
}

void SYSCFG_DL_initPower(void)
{
    /* Enable GPIOA and GPIOB power */
    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    while (DL_GPIO_isPowerEnabled(GPIOA) != true);
    while (DL_GPIO_isPowerEnabled(GPIOB) != true);

    /* Reset GPIO to default state */
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    while (DL_GPIO_isReset(GPIOA) != true);
    while (DL_GPIO_isReset(GPIOB) != true);
}

void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_SYSCTL_init();
}
