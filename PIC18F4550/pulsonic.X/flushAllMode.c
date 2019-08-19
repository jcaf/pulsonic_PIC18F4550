#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "autoMode.h"
#include "flushAllMode.h"
#include "ikb/ikb.h"

static struct _flushAllMode
{
    int8_t numNozzle; //current nozzle position
    int8_t sm0;
} flushAllMode;

void flushAllMode_cmd(int8_t cmd)
{
    if (cmd == JOB_RESTART)
    {
        disp7s_modeDisp_off();
        disp7s_qtyDisp_writeText_FLU();
        //
        /*get current position*/
        pulsonic.numNozzle = nozzle_getPosition();
        //
        flushAllMode.sm0 = 0x1;
    }
    if (cmd == JOB_STOP)
    {
        flushAllMode.sm0 = 0;
    }
    pump_stop();
    
    mpap_stall();
}

void flushAllMode_job(void)
{
    if (flushAllMode.sm0 > 0)
    {
        if (flushAllMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                pulsonic.numNozzle++;

                if (pulsonic.numNozzle >= NOZZLE_NUMMAX)
                {
                    pulsonic.numNozzle = 0x00;
                }
                flushAllMode.sm0++;
            }
        }
        else if (flushAllMode.sm0 == 2)
        {
            if (nozzle_setPosition(pulsonic.numNozzle))
            {
                flushAllMode.sm0++;
            }
        }
        
        else if (flushAllMode.sm0 == 3)
        {
            if (mpap_isIdle())
            {
                if (nozzle_isEnabled(pulsonic.numNozzle))
                {
                    flushAllMode.sm0++;
                }
                else
                {
                    flushAllMode.sm0 -= 2;//-> corre a otro nozzle
                }
            }
        }
        else if (flushAllMode.sm0 == 4)
        {
            if (mpap_isIdle())
            {
                pump_setTick(6);//buen bombeo segun Bryan
                flushAllMode.sm0++;
            }
        }
        else if (flushAllMode.sm0 == 5)
        {
            if (pump_isIdle())
            {
                flushAllMode.sm0 = 0x1;
            }
        }
    }
}
