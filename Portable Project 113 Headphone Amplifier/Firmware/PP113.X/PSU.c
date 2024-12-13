#include <xc.h>
#include "PSU.h"

#define H_PWM_FET_TRIS TRISA
#define H_PWM_FET_LAT LATA
#define H_PWM_FET_PPS RA5PPS
#define H_PWM_FET (1U << 5)

#define L_PWM_FET_TRIS TRISC
#define L_PWM_FET_LAT LATC
#define L_PWM_FET_PPS RC4PPS
#define L_PWM_FET (1U << 4)

#define H_ENABLE_FET_TRIS TRISC
#define H_ENABLE_FET_LAT LATC
#define H_ENABLE_FET (1U << 5)

#define L_ENABLE_FET_TRIS TRISA
#define L_ENABLE_FET_LAT LATA
#define L_ENABLE_FET (1U << 4)

#define BT_DISABLE_LAT LATC
#define BT_DISABLE_TRIS TRISC
#define BT_DISABLE (1U << 6)

#define CWG1A_PPS_VAL 0x05
#define CWG1B_PPS_VAL 0x06

#define PR_VAL 39   //32 MHz / 25 kHz / 1:4 Fosc / 1:8 prescaler - 1
#define DC_VAL 20   //50% duty cycle
#define DB_VAL 32   //Dead band cycles

static void
AllTransistorsOff(void)
{
    H_ENABLE_FET_LAT |= H_ENABLE_FET;
    L_ENABLE_FET_LAT &= ~L_ENABLE_FET;
    H_ENABLE_FET_TRIS &= ~H_ENABLE_FET;
    L_ENABLE_FET_TRIS &= ~L_ENABLE_FET;

    H_PWM_FET_TRIS |= H_PWM_FET;
    L_PWM_FET_TRIS |= L_PWM_FET;
    H_PWM_FET_PPS = 0;
    L_PWM_FET_PPS = 0;
    H_PWM_FET_LAT |= H_PWM_FET;
    L_PWM_FET_LAT &= ~L_PWM_FET;
    H_PWM_FET_TRIS &= ~H_PWM_FET;
    L_PWM_FET_TRIS &= ~L_PWM_FET;
}

static void
TurnOff(void)
{
    AllTransistorsOff();

    CWG1EN = 0;
    PWM6EN = 0;

    BT_DISABLE_LAT &= ~BT_DISABLE;
}

static void
ConfigurePWM(void)
{
    TMR2MD = 0;
    PWM6MD = 0;

    T2PR = PR_VAL;
    PWM6DCL = 0;
    PWM6DCH = DC_VAL;

    //T2 clock source = Fosc / 4
    T2CLKCON = _T2CLKCON_CS0_MASK;

    //Enable T2, 1:8 prescaler
    T2CON = _T2CON_ON_MASK | _T2CON_CKPS1_MASK | _T2CON_CKPS0_MASK;
}

static void
ConfigureCWG(void)
{
    AllTransistorsOff();

    CWG1MD = 0;
    //Set half-bridge mode
    CWG1CON0 = _CWG1CON0_MODE2_MASK;
    //Invert POLA
    CWG1CON1 = _CWG1CON1_POLA_MASK;
    //Set data source to PWM6
    CWG1ISM = _CWG1ISM_CWG1DAT2_MASK | _CWG1ISM_CWG1DAT0_MASK;
    //Tri-state output pins on auto-shutdown (Required because startup will be from shutdown state)
    CWG1AS0 = _CWG1AS0_LSBD0_MASK | _CWG1AS0_LSAC0_MASK;
    //Set rising and falling deadband cycles
    CWG1DBR = DB_VAL;
    CWG1DBF = DB_VAL;
}

void
PSU_Init(void)
{
    ConfigurePWM();
    ConfigureCWG();

    BT_DISABLE_LAT &= ~BT_DISABLE;
    BT_DISABLE_TRIS &= ~BT_DISABLE;
}

void
PSU_BeforeSleep(void)
{
    TurnOff();

    CWG1MD = 1;
    PWM6MD = 1;
    TMR2MD = 1;
}

void
PSU_TurnOn(void)
{
    H_PWM_FET_TRIS |= H_PWM_FET;
    L_PWM_FET_TRIS |= L_PWM_FET;
    H_PWM_FET_PPS = CWG1A_PPS_VAL;
    L_PWM_FET_PPS = CWG1B_PPS_VAL;

    TMR2IF = 0;
    while (!TMR2IF);

    PWM6EN = 1;
    CWG1EN = 1;

    H_PWM_FET_TRIS &= ~H_PWM_FET;
    L_PWM_FET_TRIS &= ~L_PWM_FET;

    H_ENABLE_FET_LAT &= ~H_ENABLE_FET;
    L_ENABLE_FET_LAT |= L_ENABLE_FET;
}

void
PSU_TurnBluetoothOn(void)
{
    BT_DISABLE_LAT &= ~BT_DISABLE;
}

void
PSU_TurnBluetoothOff(void)
{
    BT_DISABLE_LAT |= BT_DISABLE;
}
