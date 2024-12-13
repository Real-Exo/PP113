#include <xc.h>
#include <stdint.h>
#include "Switch.h"

#define SWITCH_PORT PORTC
#define SWITCH_ANSEL ANSELC
#define SWITCH_PPS_VAL 0x13
#define SWITCH (1U << 3)

#define LONGPRESS_TICKS_100MS 20

static volatile uint8_t pressedTicks_100ms = 0;
static volatile bool wasPressedLong = false;
static volatile bool wasPressedShort = false;
static volatile bool resetRequested = false;

void
SWITCH_Init(void)
{
    SWITCH_ANSEL &= ~SWITCH;
}

//Sleep until switch is pressed
void
SWITCH_Sleep(void)
{
    while (!(SWITCH_PORT & SWITCH));

    INTPPS = SWITCH_PPS_VAL;
    INTEDG = 0;
    INTF = 0;
    INTE = 1;

    SLEEP();
    NOP();
    INTE = 0;
}

bool
SWITCH_LongPress(void)
{
    if (wasPressedLong)
    {
        wasPressedLong = false;
        resetRequested = true;

        return true;
    }

    return false;
}

bool
SWITCH_ShortPress(void)
{
    if (wasPressedShort)
    {
        wasPressedShort = false;
        resetRequested = true;

        return true;
    }

    return false;
}

void
SWITCH_ServiceTimer_100ms(void)
{
    if (resetRequested)
    {
        if (SWITCH_PORT & SWITCH)
        {
            resetRequested = false;
            pressedTicks_100ms = 0;
        }

        return;
    }

    if (wasPressedLong || wasPressedShort)
    {
        return;
    }

    if (!(SWITCH_PORT & SWITCH))
    {
        if (++pressedTicks_100ms >= LONGPRESS_TICKS_100MS)
        {
            wasPressedLong = true;
            wasPressedShort = false;
        }
    }
    else
    {
        if (pressedTicks_100ms)
        {
            wasPressedLong = false;
            wasPressedShort = true;
        }
    }
}
