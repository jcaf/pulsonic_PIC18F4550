#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "autoMode.h"
#include "flushMode.h"
#include "ikb/ikb.h"

static struct _flushMode
{
    int8_t numNozzle;   //current nozzle position
    int8_t sm0;
}flushMode;

void flushMode_cmd(int8_t cmd)
{
    if (cmd == JOB_RESTART)
    {
        disp7s_modeDisp_off();
        disp7s_qtyDisp_writeText_FLU();
        //
        flushMode.numNozzle = 0x0;
        flushMode.sm0 = 0x1;
        
        mpap.mode = MPAP_STALL_MODE;
    }
    if (cmd == JOB_STOP)
    {
        flushMode.sm0 = 0;
        mpap.mode = MPAP_STALL_MODE;
    }
}

void flushMode_job(void)
{
    if (flushMode.sm0 > 0)
    {
        if (flushMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                mpap_setupToHomming();
                flushMode.sm0++;
            }
        }
        else if (flushMode.sm0 == 2)
        {
            if (mpap_isIdle())
            {
                flushMode.sm0++;
            }
        }
        /* Distribute oil */
        else if (flushMode.sm0 == 3)
        {
            if (nozzle_isEnabled(flushMode.numNozzle))
            {
                mpap_movetoNozzle(flushMode.numNozzle);
                flushMode.sm0++;
            }

            if (++flushMode.numNozzle == NOZZLE_NUMMAX)
                {flushMode.numNozzle = 0x00;}
        }
        else if (flushMode.sm0 == 4)
        {
            if (mpap_isIdle())
            {
                pump_setTick(2);
                flushMode.sm0++;
            }
        }
        else if (flushMode.sm0 == 5)
        {
            if (pump_isIdle())
            {
                flushMode.sm0 = 0x3;
            }
        }
    }
}
