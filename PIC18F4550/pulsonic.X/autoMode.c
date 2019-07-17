#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "autoMode.h"
#include "visMode.h"
#include "ikb/ikb.h"
#include "flushMode.h"

struct _ps ps_autoMode;

static struct _autoMode
{
    int8_t numNozzle;   //current nozzle position
    int8_t sm0;
}autoMode;


void autoMode_init(int8_t init)
{
    if (disp_owner == DISPOWNER_AUTOMODE)
    {
        disp7s_modeDisp_off();
        disp7s_qtyDisp_writeFloat( pulsonic_getTotalSum_mlh() );
    }
    if (init == AUTOMODE_INIT_RESTART)
    {
        autoMode.numNozzle = 0x0;
        autoMode.sm0 = 0x0;
    }
}

void autoMode_job(void)
{
    static uint16_t c_ms;
    static uint16_t c_min;
    
    if (ps_autoMode.unlock.ps)
    {
        if (autoMode.sm0 == 0)
        {
            if (mpap_isIdle())
            {
                mpap_setupToHomming();
                autoMode.sm0++;
            }
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
            if (nozzle_isEnabled(autoMode.numNozzle))
            {
                mpap_movetoNozzle(autoMode.numNozzle);
                autoMode.sm0++;
            }

            if (++autoMode.numNozzle == NOZZLE_NUMMAX)
                {autoMode.numNozzle = 0x00;}
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
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 5)
        {
            if (smain.f.f1ms)
            {
                if (++c_ms >= 2000)
                {
                    c_ms = 0x0000;
                    //if (++c_min >= 2)
                    {
                        c_min = 0x00;
                        autoMode.sm0 = 0x2;
                    }
                }
            }

        }
    }
    
}
