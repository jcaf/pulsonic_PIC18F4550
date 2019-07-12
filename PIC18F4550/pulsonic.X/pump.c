#include "main.h"
#include "pump.h"

#define PUMP_TICK_TIME 50//in ms

static struct _pump pump;

void pump_setTick(uint16_t ticks)
{
    pump.ticks = ticks;
}
uint16_t pump_getTick(void)
{
    return pump.ticks;
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
    static int8_t c;
    
    if (pump.ticks > 0)
    {
        if (pump.sm0 == 0)
        {
            PUMP_ENABLE();
            c = 0x0;
            pump.sm0++;
        }
        else if (pump.sm0 == 1)
        {
            if (main_flag.f1ms)
            {
                if (++c == PUMP_TICK_TIME)
                {
                    c=0x00;
                    pump.sm0++;
                    PUMP_DISABLE();
                }
            }
        }
        else if (pump.sm0 == 2)
        {
            if (main_flag.f1ms)
            {
                if (++c == PUMP_TICK_TIME)
                {
                    c=0x00;
                    pump.sm0 = 0x00;
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
int8_t pump_isIdle(void)
{
    if (pump.ticks == 0)
        {return 1;}
    return 0;
}