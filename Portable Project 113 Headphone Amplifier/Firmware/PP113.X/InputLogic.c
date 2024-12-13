#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "InputLogic.h"
#include "PSU.h"
#include "InputSelect.h"
#include "LED.h"

#define BT_TURNOFF_DELAY_TICKS_1S 60 

static bool bluetoothMode = true;
static volatile uint8_t bluetoothTurnOffTicks_1s;

void
INPUTLOGIC_RestoreLastMode(void)
{
    if (bluetoothMode)
    {
        PSU_TurnBluetoothOn();
        INPUTSELECT_Bluetooth();
        LED_Blue();
    }
    else
    {
        INPUTSELECT_LineIn();
        LED_Red();
    }
}

void
INPUTLOGIC_ChangeMode(void)
{ 
    bluetoothMode = !bluetoothMode;
    bluetoothTurnOffTicks_1s = BT_TURNOFF_DELAY_TICKS_1S;

    INPUTLOGIC_RestoreLastMode();
}

void
INPUTLOGIC_ServiceTimer_1s(void)
{
    if (bluetoothTurnOffTicks_1s)
    {
        --bluetoothTurnOffTicks_1s;
    }
    else
    {
        if (!bluetoothMode)
        {
            PSU_TurnBluetoothOff();
        }
    }
}
