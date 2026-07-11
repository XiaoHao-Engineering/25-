#include "sysinit.h"
#include "mspm0g350x.h"

void System_Init(void)
{
    GPIOA->GPRCM.GPRCM_PWREN = 0x0A000001;
    GPIOB->GPRCM.GPRCM_PWREN = 0x0A000001;
}
