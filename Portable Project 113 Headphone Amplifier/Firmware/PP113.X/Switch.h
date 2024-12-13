#ifndef SWITCH_H
#define SWITCH_H

#include <stdbool.h>

void
SWITCH_Init(void);

void
SWITCH_Sleep(void);

bool
SWITCH_LongPress(void);

bool
SWITCH_ShortPress(void);

void
SWITCH_ServiceTimer_100ms(void);

#endif //SWITCH
