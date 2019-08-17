#include "main.h"
#include "nozzle.h"
#include "myeeprom.h"
#include "PIC/eeprom/eeprom.h"

void myeeprom_init(void)
{
    uint8_t reg;
    for (reg=0; reg<NOZZLE_NUMMAX; reg++)
    {
        eepromWrite_double(&(((double*)EEPROM_BLOCK_ADDR)[reg]), 3.0);//max val
    }
    //next address is for index-of-OIL_VISCOSITY[]
    eepromWrite(EEPROM_BLOCK_ADDR + (NOZZLE_NUMMAX*sizeof(double)), 0);//default first index 
}