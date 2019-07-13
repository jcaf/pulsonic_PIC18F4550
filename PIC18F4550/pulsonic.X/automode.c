#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"

static struct _autoMode
{
    int8_t n;   //current nozzle position
    int8_t sm0;
}autoMode;

void autoMode_init(void)
{
    if (disp_owner == DISPOWNER_AUTO_MODE)
    {
        disp7s_qtyDisp_writeFloat( pulsonic_getTotalSum_mlh() );
    }
    autoMode.n = 0x0;
}

int8_t autoMode_job(void)
{
    int8_t cod_ret = 0;
    
    if (autoMode.sm0 == 0)
    {
        mpap_setupToHomming();
        autoMode.sm0++;
    }
    else if (autoMode.sm0 == 1)
    {
        if (mpap_isIdle())
        {
            autoMode.sm0++;
        }
    }
    /* Distribute oil */
    else if (autoMode.sm0 == 2)
    {
        if (nozzle_isEnabled(autoMode.n))
        {
            mpap_movetoNozzle(autoMode.n);
            autoMode.sm0++;
        }
        
        if (++autoMode.n == NOZZLE_NUMMAX)
            {autoMode.n = 0x00;}
    }
    else if (autoMode.sm0 == 3)
    {
        if (mpap_isIdle())
        {
            pump_setTick(1);
            autoMode.sm0++;
        }
    }
    else if (autoMode.sm0 == 4)
    {
        if (pump_isIdle())
        {
            autoMode.sm0 = 0x2;
        }
    }
    return cod_ret;
}
void autoMode_kb(void)
{
    
}
