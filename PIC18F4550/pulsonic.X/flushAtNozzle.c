#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "autoMode.h"
#include "flushAtNozzle.h"
#include "ikb/ikb.h"

static struct _flushAtNozzle
{
    int8_t numNozzle;   
    int8_t sm0;
}flushAtNozzle;

void flushAtNozzle_setNozzle(int8_t nozzle)
{
    flushAtNozzle.numNozzle = nozzle;
}

volatile uint16_t counterTicks_debug=0;

void flushAtNozzle_cmd(int8_t cmd)
{
    if (cmd == JOB_RESTART)
    {
        disp7s_qtyDisp_writeText_FLU();
        //
        flushAtNozzle.sm0 = 0x1;
        counterTicks_debug = 0;
    }
    if (cmd == JOB_STOP)
    {
        flushAtNozzle.sm0 = 0;
    }
    pump_stop();
    mpap_setMode(MPAP_STALL_MODE);
}


void x()
{
    if (flushAtNozzle.numNozzle == 0)//go to 0 
    {
        if (mpap_getNozzlePosition() > 0)
        {
            mpap_movetoNozzle(17)
        }
    }
    //luego toca el arrastre sobre el sensor...
    que hago entre el 18 y el 0
}

void flushAtNozzle_job(void)
{
    if (flushAtNozzle.sm0 > 0)
    {
        if (flushAtNozzle.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                flushAtNozzle.sm0++;
            }
        }
        else if (flushAtNozzle.sm0 == 2)
        {
            if (nozzle_isEnabled(flushAtNozzle.numNozzle))
            {
                mpap_movetoNozzle(flushAtNozzle.numNozzle);
                flushAtNozzle.sm0++;
            }
        }
        else if (flushAtNozzle.sm0 == 3)
        {
            if (mpap_isIdle())
            {
                pump_setTick(1);
                flushAtNozzle.sm0++;
                //
                counterTicks_debug++;
            }
        }
        else if (flushAtNozzle.sm0 == 4)
        {
            if (pump_isIdle())
            {
                flushAtNozzle.sm0--;
            }
        }
    }
}
