#include "main.h"
#include "pulsonic.h"


struct _pulsonic pulsonic;
const int8_t OIL_VISCOSITY[4]={22, 32, 46, 68};

void pulsonic_init(void)
{
    int8_t i;
    pulsonic.dist_total_time = 60;  //min
    
    /*save and restore from EEPROM*/
    
    for (i=0; i< NOZZLE_NUMMAX; i++)
    {
        pulsonic.nozzle[i].Q_mlh = NOZZLE_QMLH_MAX;
    }
    pulsonic.oil.viscosity = OIL_VISCOSITY[0];
    //
    pulsonic.nozzle[3].Q_mlh = 0;//Simulo OFF, NOzzle 0 cannot be disabled!
    pulsonic.nozzle[5].Q_mlh = 0;
}

uint16_t pulsonic_getTotalSum_mlh(void)
{
    int i;
    uint16_t acc = 0;
    for (i=0; i<NOZZLE_NUMMAX; i++)
    {
        acc += pulsonic.nozzle[i].Q_mlh;
    }
    return acc;
}
