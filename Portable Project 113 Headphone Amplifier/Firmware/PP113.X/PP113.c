// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator mode selection bits (Oscillator not enabled)
#pragma config RSTOSC = HFINT32 // Power-up default value for COSC bits (HFINTOSC with OSCFRQ= 32 MHz and CDIV = 1:1)
#pragma config CLKOUTEN = OFF   // Clock Out Enable bit (CLKOUT function is disabled; i/o or oscillator function on OSC2)
#pragma config CSWEN = ON       // Clock Switch Enable bit (Writing to NOSC and NDIV is allowed)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (FSCM timer disabled)

// CONFIG2
#pragma config MCLRE = ON       // Master Clear Enable bit (MCLR pin is Master Clear function)
#pragma config PWRTS = PWRT_64  // Power-up Timer Enable bit (PWRT set at 64 ms)
#pragma config LPBOREN = ON     // Low-Power BOR enable bit (ULPBOR enabled)
#pragma config BOREN = ON       // Brown-out reset enable bits (Brown-out Reset Enabled, SBOREN bit is ignored)
#pragma config BORV = HI        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (VBOR) is set to 2.7V)
#pragma config ZCD = OFF        // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR.)
#pragma config PPS1WAY = OFF    // Peripheral Pin Select one-way control (The PPSLOCK bit can be set and cleared repeatedly by software)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable bit (Stack Overflow or Underflow will cause a reset)
#pragma config DEBUG = ON      // Background Debugger (Background Debugger disabled)

// CONFIG3
#pragma config WDTCPS = WDTCPS_31// WDT Period Select bits (Divider ratio 1:65536; software control of WDTPS)
#pragma config WDTE = OFF       // WDT operating mode (WDT Disabled, SWDTEN is ignored)
#pragma config WDTCWS = WDTCWS_7// WDT Window Select bits (window always open (100%); software control; keyed access not required)
#pragma config WDTCCS = SC      // WDT input clock selector (Software Control)

// CONFIG4
#pragma config BBSIZE = BB512   // Boot Block Size Selection bits (512 words boot block size)
#pragma config BBEN = OFF       // Boot Block Enable bit (Boot Block disabled)
#pragma config SAFEN = OFF      // SAF Enable bit (SAF disabled)
#pragma config WRTAPP = OFF     // Application Block Write Protection bit (Application Block not write protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block not write protected)
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration Register not write protected)
#pragma config WRTD = OFF       // Data EEPROM write protection bit (Data EEPROM NOT write protected)
#pragma config WRTSAF = OFF     // Storage Area Flash Write Protection bit (SAF not write protected)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (High Voltage on MCLR/Vpp must be used for programming)

// CONFIG5
#pragma config CP = OFF         // UserNVM Program memory code protection bit (UserNVM code protection disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "InputSelect.h"
#include "InputLogic.h"
#include "PSU.h"
#include "BatteryGuard.h"
#include "Timer.h"
#include "LED.h"
#include "Switch.h"

static void
Init(void)
{
    BATTERYGUARD_Init();
    INPUTSELECT_Init();
    PSU_Init();
    LED_Init();
    TIMER_Init();  
    SWITCH_Init();
}

static void
Sleep(void)
{
    BATTERYGUARD_BeforeSleep();
    INPUTSELECT_BeforeSleep();   
    PSU_BeforeSleep(); 
    LED_BeforeSleep();  
    TIMER_BeforeSleep();
    
    SWITCH_Sleep();
    
    // Re-initialise all systems after sleep...
    Init();
}



void 
main(void)
{
    Init();
    
    while (1)
    {
        Sleep();
        
        if (BATTERYGUARD_IsEmpty())
        {
            continue;
        }
        
        INPUTLOGIC_RestoreLastMode();
        PSU_TurnOn();
        
        while (1)
        {
            if (BATTERYGUARD_IsEmpty())
            {
                break;
            }
            
            if (SWITCH_ShortPress())
            {
                break;
            }
            
            if (SWITCH_LongPress())
            {
                INPUTLOGIC_ChangeMode();
            }
        }
    }
}
