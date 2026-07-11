#ifndef __KEY_H
#define __KEY_H

#include <stdint.h>

#define KEY_UP      0
#define KEY_DOWN    1
#define KEY_CONFIRM 2

#define KEY_NONE    0xFF

/* PINCM索引 */
#define KEY_UP_PINCM     18   /* PA18 */
#define KEY_DOWN_PINCM   46   /* PB14 */
#define KEY_CONFIRM_PINCM 2   /* PA2  */

void Key_Init(void);
uint8_t Key_Scan(void);
uint8_t Key_WaitPress(void);

#endif
