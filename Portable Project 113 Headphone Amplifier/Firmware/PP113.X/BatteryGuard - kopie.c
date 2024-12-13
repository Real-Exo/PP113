#include <xc.h>
#include "BatteryGuard.h"

#define SENSE_ENABLE_TRIS TRISA
#define SENSE_ENABLE_LAT LATA
#define SENSE_ENABLE (1U << 2)

#define CELL1_TRIS TRISB
#define CELL1_ANSEL ANSELB
#define CELL1 (1U << 6)
#define CELL1_PCH 0x0E

#define CELL2_TRIS TRISC
#define CELL2_ANSEL ANSELC
#define CELL2 (1U << 2)
#define CELL2_PCH 0x12

#define CELL3_TRIS TRISB
#define CELL3_ANSEL ANSELB
#define CELL3 (1U << 5)
#define CELL3_PCH 0x0D

#define CELL4_TRIS TRISB
#define CELL4_ANSEL ANSELB
#define CELL4 (1U << 4)
#define CELL4_PCH 0x0C

#define SENSE_ENABLE_DELAY_MS 25
#define DIVIDER_R1_OHM 12000
#define DIVIDER_R2_OHM 1500
#define ADC_MAXCOUNT 4096
#define VREF_MICROVOLT 2048000
#define MAX_MICROVOLT (VREF_MICROVOLT / DIVIDER_R2_OHM * (DIVIDER_R1_OHM + DIVIDER_R2_OHM))
#define ADC_STEP_MICROVOLT (MAX_MICROVOLT / ADC_MAXCOUNT)

static uint16_t
ReadCellMilliVolt(uint8_t cellPCH, uint16_t *prevADRES)
{   
    ADPCH = cellPCH;
    ADGO = 1;
    while (ADGO);
    
    uint16_t resultMilliVolt = ((ADRES - *prevADRES) * ADC_STEP_MICROVOLT) / 1000;
    *prevADRES = ADRES;
 
    return resultMilliVolt;
}

void
BATTERYGUARD_Init(void)
{
    // Re-enable ADC and FVR module after sleep.
    FVRMD = 0;
    ADCMD = 0;
    
    SENSE_ENABLE_LAT &= ~SENSE_ENABLE;
    SENSE_ENABLE_TRIS &= ~SENSE_ENABLE;
    
    CELL1_TRIS |= CELL1;
    CELL2_TRIS |= CELL2;
    CELL3_TRIS |= CELL3;
    CELL4_TRIS |= CELL4;
    
    CELL1_ANSEL |= CELL1;
    CELL2_ANSEL |= CELL2;
    CELL3_ANSEL |= CELL3;
    CELL4_ANSEL |= CELL4;
    
    // Enable FVR, ADC FVR = 2.048V
    FVRCON = _FVRCON_FVREN_MASK | _FVRCON_ADFVR1_MASK;
    while (!FVRRDY);
    
    // AD Positive reference = FVR, negative reference = VSS
    ADREF = _ADREF_PREF1_MASK | _ADREF_PREF0_MASK;
    // Enable ADC, dedicated RC oscillator, result right justified.
    ADCON0 = _ADCON0_ADON_MASK | _ADCON0_CS_MASK | _ADCON0_FM_MASK;   
}

void
BATTERYGUARD_BeforeSleep(void)
{
    // Disable ADC and FVR module.
    ADCMD = 1;
    FVRMD = 1;
}

struct BATTERYGUARD_CellVoltages
BATTERYGUARD_ReadCells(void)
{   
    SENSE_ENABLE_LAT |= SENSE_ENABLE;
    __delay_ms(SENSE_ENABLE_DELAY_MS);
 
    uint16_t prevADRES = 0;
    struct BATTERYGUARD_CellVoltages cellVoltages;
    cellVoltages.cell1MilliVolt = ReadCellMilliVolt(CELL1_PCH, &prevADRES);
    cellVoltages.cell2MilliVolt = ReadCellMilliVolt(CELL2_PCH, &prevADRES);
    cellVoltages.cell3MilliVolt = ReadCellMilliVolt(CELL3_PCH, &prevADRES);
    cellVoltages.cell4MilliVolt = ReadCellMilliVolt(CELL4_PCH, &prevADRES);
    
    SENSE_ENABLE_LAT &= ~SENSE_ENABLE;
    
    return cellVoltages;  
}
