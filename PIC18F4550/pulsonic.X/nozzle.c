#include "main.h"
#include "mpap.h"
#include "pulsonic.h"

double NOZZLE_QTY_DIFF = 0.1;//inc/dec in 0.1

int8_t nozzle_isEnabled(int n)
{
    if (pulsonic.nozzle[n].Q_mlh > 0.0)
        return 1;
    return 0;
}

int8_t nozzle_get_pos(void)//trunca, es solo referencial
{
    return (mpap.numSteps_current/MPAP_NUMSTEP_1NOZZLE);
}
 