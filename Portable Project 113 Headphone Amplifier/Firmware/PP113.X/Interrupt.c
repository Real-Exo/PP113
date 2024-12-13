#include <xc.h>
#include "Timer.h"

void __interrupt()
ISR(void)
{
    if (TMR0IF) { TIMER_ISR(); TMR0IF = 0; }
    if (INTF) { INTF = 0; }
}
