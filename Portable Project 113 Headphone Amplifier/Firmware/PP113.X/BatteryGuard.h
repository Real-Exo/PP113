#ifndef BATTERYGUARD_H
#define BATTERYGUARD_H

#include <stdbool.h>

void
BATTERYGUARD_Init(void);

void
BATTERYGUARD_BeforeSleep(void);

bool
BATTERYGUARD_IsEmpty(void);

void
BATTERYGUARD_ServiceTimer_10ms(void);

#endif //BATTERYGUARD_H
