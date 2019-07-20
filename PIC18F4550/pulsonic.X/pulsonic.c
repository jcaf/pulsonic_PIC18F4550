#include "main.h"
#include "pulsonic.h"
#include "PIC/eeprom/eeprom.h"
struct _pulsonic pulsonic;

const int8_t OIL_VISCOSITY[OIL_VISCOSITY_NUMMAX]={22, 32, 46, 68};

void pulsonic_init(void)
{
    int8_t i;
    pulsonic.dist_total_time = 60;  //min
    
    /*save and restore from EEPROM*/
    //EEPROM addr block = 0x0000
    for (i=0; i< NOZZLE_NUMMAX; i++)
    {
        pulsonic.nozzle[i].Q_mlh = eepromRead_double(i);
    }
    pulsonic.oil.viscosity = eepromRead( EEPROM_BLOCK_ADDR + (NOZZLE_NUMMAX*sizeof(double)) );
    
    
//    for (i=0; i< NOZZLE_NUMMAX; i++)
//    {
//        pulsonic.nozzle[i].Q_mlh = 2.4f;
//    }
//    pulsonic.oil.viscosity = OIL_VISCOSITY[0];
    //
//    pulsonic.nozzle[3].Q_mlh = 0;//Simulo OFF, NOzzle 0 cannot be disabled!
//    pulsonic.nozzle[5].Q_mlh = 0;
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
