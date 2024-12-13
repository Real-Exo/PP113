#include <xc.h>
#include "InputSelect.h"

#define IN_SELECT_A_LAT LATC
#define IN_SELECT_A_TRIS TRISC
#define IN_SELECT_A (1U << 7)

#define IN_SELECT_B_LAT LATB
#define IN_SELECT_B_TRIS TRISB
#define IN_SELECT_B (1U << 7)

static void
Mute(void)
{
    IN_SELECT_A_LAT |= IN_SELECT_A;
    IN_SELECT_B_LAT |= IN_SELECT_B;
}

void
INPUTSELECT_Init(void)
{
    Mute();

    IN_SELECT_A_TRIS &= ~IN_SELECT_A;
    IN_SELECT_B_TRIS &= ~IN_SELECT_B;
}

void
INPUTSELECT_BeforeSleep(void)
{
    IN_SELECT_A_LAT &= ~IN_SELECT_A;
    IN_SELECT_B_LAT &= ~IN_SELECT_B;
}

void
INPUTSELECT_LineIn(void)
{
    IN_SELECT_A_LAT &= ~IN_SELECT_A;
    IN_SELECT_B_LAT &= ~IN_SELECT_B;
}

void
INPUTSELECT_Bluetooth(void)
{
    IN_SELECT_A_LAT |= IN_SELECT_A;
    IN_SELECT_B_LAT &= ~IN_SELECT_B;
}
