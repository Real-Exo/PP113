#ifndef LED_H
#define LED_H

#include <stdint.h>

void
LED_Init(void);

void
LED_BeforeSleep(void);

void
LED_Blue(void);

void
LED_Red(void);

void
LED_SetDutyCycle(uint8_t percentage);

void
LED_ServiceTimer_100ms(void);

#endif //LED_H
