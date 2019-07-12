#include "main.h"
#include "pulsonic.h"

struct _pulsonic pulsonic;

void pulsonic_init(void)
{
    int8_t i;
    pulsonic.dist_total_time = 60;  //min
    
    /*save and restore from EEPROM*/
    
    for (i=0; i< NOZZLE_NUMMAX; i++)
    {
        pulsonic.nozzle[i].Q_mlh = NOZZLE_QMLH_MAX;
    }
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
