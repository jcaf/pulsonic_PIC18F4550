#include "main.h"
#include "pulsonic.h"
#include "myeeprom.h"
#include "PIC/eeprom/eeprom.h"

volatile struct _pulsonic pulsonic;

const int8_t OIL_VISCOSITY[OIL_VISCOSITY_NUMMAX]={22, 32, 46, 68};

void pulsonic_init(void)
{
    int8_t i;
    pulsonic.dist_total_time = 60;  //min
    
    for (i=0; i< NOZZLE_NUMMAX; i++)
    {
        pulsonic.nozzle[i].Q_mlh = eepromRead_double(&(((double*)EEPROM_BLOCK_ADDR)[i]));
    }
    pulsonic.oil.viscosity = eepromRead( EEPROM_BLOCK_ADDR + (NOZZLE_NUMMAX*sizeof(double)) );
}

double pulsonic_getTotalSum_mlh(void)
{
    int i;
    double acc = 0;
    
    for (i=0; i<NOZZLE_NUMMAX; i++)
    {
        acc += pulsonic.nozzle[i].Q_mlh;
    }
    return acc;
}
