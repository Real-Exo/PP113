#include <xc.h>
#include <stdbool.h>
#include "LED.h"

#define LED1_TRIS TRISC
#define LED1_LAT LATC
#define LED1 (1U << 0)

#define LED2_TRIS TRISC
#define LED2_LAT LATC
#define LED2 (1U << 1)

#define TICKS_PER_CYCLE_100MS 20

static volatile uint8_t onTicks_100ms;
static volatile uint8_t currentTick_100ms;
static volatile bool blue;

static void
TurnOn(void)
{
    if (blue)
    {
        LED1_LAT &= ~LED1;
        LED2_LAT |= LED2;
    }
    else
    {
        LED1_LAT |= LED1;
        LED2_LAT &= ~LED2;
    }
}

static void
TurnOff(void)
{
    LED1_LAT &= ~LED1;
    LED2_LAT &= ~LED2;
}

void
LED_Init(void)
{
    LED_SetDutyCycle(0);
    currentTick_100ms = 0;
    blue = true;

    TurnOff();
    LED1_TRIS &= ~LED1;
    LED2_TRIS &= ~LED2;
}

void
LED_BeforeSleep(void)
{
    LED_SetDutyCycle(0);
    TurnOff();
}

void
LED_Blue(void)
{
    blue = true;
}

void
LED_Red(void)
{
    blue = false;
}

void
LED_SetDutyCycle(uint8_t percentage)
{
    if (percentage > 100)
    {
        percentage = 100;
    }
    
    onTicks_100ms = (TICKS_PER_CYCLE_100MS * percentage) / 100;

    if (percentage & !onTicks_100ms)
    {
        onTicks_100ms = 1;
    }
}

void
LED_ServiceTimer_100ms(void)
{
    if (++currentTick_100ms >= TICKS_PER_CYCLE_100MS)
    {
        currentTick_100ms = 0;
    }

    if (currentTick_100ms < onTicks_100ms)
    {
        TurnOn();
    }
    else
    {
        TurnOff();
    }
}
