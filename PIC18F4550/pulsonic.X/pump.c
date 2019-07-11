#include "main.h"
#include "pump.h"

#define PUMP_TIME_TICK 50//in ms
static struct _pump pump;

void pump_setTick(uint16_t ticks)
{
    pump.ticks = ticks;
}
void pump_stop(void)
{
    pump.ticks = 0;
    pump.sm0 = 0;//reset
    PUMP_DISABLE();
}
int8_t pump_job(void)
{
    int8_t cod_ret = 0;
    static int8_t sm0;
    static int8_t c;
    
    if (pump.ticks > 0)
    {
        if (pump.sm0 == 0)
        {
            PUMP_ENABLE();
            c = 0x0;
            sm0++;
        }
        else if (pump.sm0 == 1)
        {
            if (main_flag.f1ms)
            {
                if (++c == PUMP_TIME_TICK)
                {
                    pump.sm0 = 0x00;
                    PUMP_DISABLE();
                    //
                    if (--pump.ticks == 0)
                    {
                        cod_ret = 1;
                    }
                }
            }
        }
    }
    return cod_ret;
}
