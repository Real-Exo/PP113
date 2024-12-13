#include <xc.h>
#include <stdint.h>
#include "BatteryGuard.h"
#include "LED.h"

#define SENSE_ENABLE_TRIS TRISA
#define SENSE_ENABLE_LAT LATA
#define SENSE_ENABLE (1U << 2)

#define CELL1_PCH 0x0E
#define CELL2_PCH 0x12
#define CELL3_PCH 0x0D
#define CELL4_PCH 0x0C

#define ADC_STEP_MICROVOLT ((uint32_t) 4500) //(2048000 µV / 1500 ohm * (1500 ohm * 12000 ohm)) / 4096
#define CELL_EMPTY_MILLIVOLT 3200
#define CELL_REPLACED_MILLIVOLT 3700
#define LED_DUTYCYCLE_STEP 2

#define IDLE_PHASE_TICKS_10MS 18000

struct CellVoltages_MilliVolt
{
    uint16_t cell1;
    uint16_t cell2;
    uint16_t cell3;
    uint16_t cell4;
};

enum ServiceTimerPhase
{
    idle = 0,
    enableSense,
    readCell1,
    readCell2,
    readCell3,
    readCell4,
    postRead
};

static volatile enum ServiceTimerPhase serviceTimerPhase;
static volatile uint16_t serviceTimerTick;
static volatile struct CellVoltages_MilliVolt cellVoltages_MilliVolt;
static volatile bool isEmpty = false;
static volatile bool readCycleDone;

static bool
ReadCell_MilliVolt(uint8_t cellPCH, uint16_t *prevADRES, volatile uint16_t *result)
{
    static bool busy = false;

    if (!busy)
    {
        busy = true;

        ADPCH = cellPCH;
        ADGO = 1;

        return false;
    }

    if (ADGO)
    {
        return false;
    }

    busy = false;

    *result = ((ADRES - *prevADRES) * ADC_STEP_MICROVOLT) / 1000;
    *prevADRES = ADRES;

    return true;
}

static uint16_t
ReturnLowest(uint16_t a, uint16_t b)
{
    if (a < b)
    {
        return a;
    }

    return b;
}

static void
ProcessResults(void)
{
    uint16_t lowestCell_MilliVolt = cellVoltages_MilliVolt.cell1;
    lowestCell_MilliVolt = ReturnLowest(lowestCell_MilliVolt, cellVoltages_MilliVolt.cell2);
    lowestCell_MilliVolt = ReturnLowest(lowestCell_MilliVolt, cellVoltages_MilliVolt.cell3);
    lowestCell_MilliVolt = ReturnLowest(lowestCell_MilliVolt, cellVoltages_MilliVolt.cell4);

    if (lowestCell_MilliVolt <= CELL_EMPTY_MILLIVOLT)
    {
        lowestCell_MilliVolt = CELL_EMPTY_MILLIVOLT;
        isEmpty = true;
    }

    if (lowestCell_MilliVolt >= CELL_REPLACED_MILLIVOLT)
    {
        isEmpty = false;
    }

    uint16_t newDutyCycle = (lowestCell_MilliVolt - CELL_EMPTY_MILLIVOLT) / LED_DUTYCYCLE_STEP;
    if (newDutyCycle > 100)
    {
        newDutyCycle = 100;
    }
    LED_SetDutyCycle((uint8_t) newDutyCycle);
}

void
BATTERYGUARD_Init(void)
{
    FVRMD = 0;
    ADCMD = 0;

    SENSE_ENABLE_LAT &= ~SENSE_ENABLE;
    SENSE_ENABLE_TRIS &= ~SENSE_ENABLE;

    //Enable FVR, ADC FVR = 2.048V
    FVRCON = _FVRCON_FVREN_MASK | _FVRCON_ADFVR1_MASK;
    while (!FVRRDY);

    //AD Positive reference = FVR, negative reference = VSS
    ADREF = _ADREF_PREF1_MASK | _ADREF_PREF0_MASK;
    //Enable ADC, dedicated RC oscillator, results right justified
    ADCON0 = _ADCON0_ADON_MASK | _ADCON0_CS_MASK | _ADCON0_FM_MASK;

    serviceTimerPhase = idle;
    serviceTimerTick = IDLE_PHASE_TICKS_10MS;
    readCycleDone = false;
}

void
BATTERYGUARD_BeforeSleep(void)
{
    ADCMD = 1;
    FVRMD = 1;

    SENSE_ENABLE_LAT &= ~SENSE_ENABLE;
}

bool
BATTERYGUARD_IsEmpty(void)
{
    while (!readCycleDone);
    return isEmpty;
}

void
BATTERYGUARD_ServiceTimer_10ms(void)
{
    static uint16_t prevADRES;

    ++serviceTimerTick;

    switch (serviceTimerPhase)
    {
    case idle:              if (serviceTimerTick >= IDLE_PHASE_TICKS_10MS)
                            {
                                serviceTimerPhase = enableSense;
                            }
                            break;

    case enableSense:       SENSE_ENABLE_LAT |= SENSE_ENABLE;
                            serviceTimerPhase = readCell1;
                            prevADRES = 0;
                            break;

    case readCell1:         if (ReadCell_MilliVolt(CELL1_PCH, &prevADRES, &cellVoltages_MilliVolt.cell1))
                            {
                                serviceTimerPhase = readCell2;
                            }
                            break;

    case readCell2:         if (ReadCell_MilliVolt(CELL2_PCH, &prevADRES, &cellVoltages_MilliVolt.cell2))
                            {
                                serviceTimerPhase = readCell3;
                            }
                            break;

    case readCell3:         if (ReadCell_MilliVolt(CELL3_PCH, &prevADRES, &cellVoltages_MilliVolt.cell3))
                            {
                                serviceTimerPhase = readCell4;
                            }
                            break;

    case readCell4:         if (ReadCell_MilliVolt(CELL4_PCH, &prevADRES, &cellVoltages_MilliVolt.cell4))
                            {
                                serviceTimerPhase = postRead;
                            }
                            break;

    case postRead:          ProcessResults();
    default:
                            SENSE_ENABLE_LAT &= ~SENSE_ENABLE;
                            serviceTimerTick = 0;
                            serviceTimerPhase = idle;
                            readCycleDone = true;
                            break;
    }
}
