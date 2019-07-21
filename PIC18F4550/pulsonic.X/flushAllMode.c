#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "autoMode.h"
#include "flushAllMode.h"
#include "ikb/ikb.h"

static struct _flushAllMode
{
    int8_t numNozzle;   //current nozzle position
    int8_t sm0;
}flushAllMode;

void flushAllMode_cmd(int8_t cmd)
{
    if (cmd == JOB_RESTART)
    {
        disp7s_modeDisp_off();
        disp7s_qtyDisp_writeText_FLU();
        //
        flushAllMode.numNozzle = 0x0;
        flushAllMode.sm0 = 0x1;
        
        mpap.mode = MPAP_STALL_MODE;
    }
    if (cmd == JOB_STOP)
    {
        flushAllMode.sm0 = 0;
        mpap.mode = MPAP_STALL_MODE;
        pump_stop();
    }
}

void flushAllMode_job(void)
{
    if (flushAllMode.sm0 > 0)
    {
        if (flushAllMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                mpap_setupToHomming();
                flushAllMode.sm0++;
            }
        }
        else if (flushAllMode.sm0 == 2)
        {
            if (mpap_isIdle())
            {
                flushAllMode.sm0++;
            }
        }
        /* Distribute oil */
        else if (flushAllMode.sm0 == 3)
        {
            if (nozzle_isEnabled(flushAllMode.numNozzle))
            {
                mpap_movetoNozzle(flushAllMode.numNozzle);
                flushAllMode.sm0++;
            }

            if (++flushAllMode.numNozzle == NOZZLE_NUMMAX)
                {flushAllMode.numNozzle = 0x00;}
        }
        else if (flushAllMode.sm0 == 4)
        {
            if (mpap_isIdle())
            {
                pump_setTick(2);
                flushAllMode.sm0++;
            }
        }
        else if (flushAllMode.sm0 == 5)
        {
            if (pump_isIdle())
            {
                flushAllMode.sm0 = 0x3;
            }
        }
    }
}
