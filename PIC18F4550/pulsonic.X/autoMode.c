#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "autoMode.h"
#include "visMode.h"
#include "ikb/ikb.h"
#include "flushAllMode.h"

static struct _autoMode
{
    int8_t numNozzle;   //current nozzle position
    int8_t sm0;
}autoMode;

void autoMode_disp7s_writeSumTotal(void)
{
    if (disp_owner == DISPOWNER_AUTOMODE)
    {
        disp7s_modeDisp_off();
        disp7s_qtyDisp_writeFloat( pulsonic_getTotalSum_mlh() );
    }
}
void autoMode_cmd(int8_t cmd)
{
    if (cmd == JOB_RESTART)
    {
        autoMode.numNozzle = 0x0;
        autoMode.sm0 = 0x1;
        mpap.mode = MPAP_STALL_MODE;
    }
    else if (cmd == JOB_STOP)
    {
        autoMode.sm0 = 0;
        mpap.mode = MPAP_STALL_MODE;
        pump_stop();
    }
}

void autoMode_job(void)
{
    static uint16_t c_ms;
    static uint16_t c_min;

    if (autoMode.sm0 >0)
    {
        if (autoMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                mpap_setupToHomming();
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 2)
        {
            if (mpap_isIdle())
            {
                autoMode.sm0++;
            }
        }
        /* Distribute oil */
        else if (autoMode.sm0 == 3)
        {
            if (nozzle_isEnabled(autoMode.numNozzle))
            {
                mpap_movetoNozzle(autoMode.numNozzle);
                autoMode.sm0++;
            }

            if (++autoMode.numNozzle == NOZZLE_NUMMAX)
                {autoMode.numNozzle = 0x00;}
        }
        else if (autoMode.sm0 == 4)
        {
            if (mpap_isIdle())
            {
                pump_setTick(1);
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 5)
        {
            if (pump_isIdle())
            {
                autoMode.sm0++;
                c_ms = 0x00;
                c_min = 0x00;
            }
        }
        else if (autoMode.sm0 == 6)
        {
            if (smain.f.f1ms)
            {
                if (++c_ms >= 2000)
                {
                    c_ms = 0x0000;
                    //if (++c_min >= 2)
                    {
                        c_min = 0x00;
                        autoMode.sm0 = 0x3;
                    }
                }
            }
        }
    }
}
