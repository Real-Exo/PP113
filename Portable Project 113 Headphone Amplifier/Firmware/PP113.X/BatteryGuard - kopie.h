#ifndef BATTERYGUARD_H
#define BATTERYGUARD_H

#include <stdint.h>

struct BATTERYGUARD_CellVoltages
{
    uint16_t cell1MilliVolt;
    uint16_t cell2MilliVolt;
    uint16_t cell3MilliVolt;
    uint16_t cell4MilliVolt;
};

void 
BATTERYGUARD_Init(void);

void
BATTERYGUARD_BeforeSleep(void);

struct BATTERYGUARD_CellVoltages
BATTERYGUARD_ReadCells(void);

#endif //BATTERYUARD_H