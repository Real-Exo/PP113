#include <xc.h>
#include <stdint.h>
#include "Timer.h"
#include "LED.h"
#include "Switch.h"
#include "InputLogic.h"
#include "BatteryGuard.h"

#define T0_CMP_VAL 124 //(32 Mhz * 10 ms / Fosc/4 / 1:128 prescaler / 1:5 postscaler) - 1

static volatile uint8_t tickCnt_10ms;
static volatile uint8_t tickCnt_100ms;

void
TIMER_Init(void)
{
    TMR0MD = 0;

    tickCnt_10ms = 0;
    tickCnt_100ms = 0;

    TMR0L = 0;
    TMR0H = T0_CMP_VAL;
    //T0 clock source = Fosc/4, 1:128 prescaler
    T0CON1 = _T0CON1_T0CS1_MASK | _T0CON1_T0CKPS2_MASK | _T0CON1_T0CKPS1_MASK | _T0CON1_T0CKPS0_MASK;
    //Enable T0 in 8 bit mode, 1:5 postscaler
    T0CON0 = _T0CON0_T0EN_MASK | _T0CON0_T0OUTPS2_MASK;

    TMR0IE = 1;
    GIE = 1;
}

void
TIMER_BeforeSleep(void)
{
    TMR0IE = 0;

    TMR0MD = 1;
}

void
TIMER_ISR(void)
{
    if (++tickCnt_10ms >= 10)
    {
        tickCnt_10ms = 0;

        if (++tickCnt_100ms >= 10)
        {
            tickCnt_100ms = 0;
            
            INPUTLOGIC_ServiceTimer_1s();
        }
        
        LED_ServiceTimer_100ms();
        SWITCH_ServiceTimer_100ms();
    }
    
    BATTERYGUARD_ServiceTimer_10ms();
}
