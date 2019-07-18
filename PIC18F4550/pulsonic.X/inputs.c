#include "main.h"
//1=Not Signal(pull-up), 0=Signal present!
//return !PinRead(PORTRxSTARTSIGNAL, PINxSTARTSIGNAL);
static int8_t c;
static int8_t S;
static int8_t sVar;
static int8_t sm0;

void startSignal_init(void)
{
    sVar = !PinRead(PORTRxSTARTSIGNAL, PINxSTARTSIGNAL);
    S = sVar;
}

int8_t is_startSignal(void)
{
    
    int8_t s;
    
    if  (sm0 == 0)
    {
        s = !PinRead(PORTRxSTARTSIGNAL, PINxSTARTSIGNAL);
        if (sVar != s)
        {
            sVar = s;
            sm0++;
        }
    }
    else if (sm0 == 1)
    {
        if (smain.f.f1ms)
        {
            if (++c == 20)//20 ms
            {
                c=0x0;
                s = !PinRead(PORTRxSTARTSIGNAL, PINxSTARTSIGNAL);
                if (sVar == s)
                    {S = sVar;}
                sm0 = 0x00;
            }
        }
    }
    return S;
}
int8_t is_oilLevel(void)
{
    //1=OIl ok (pull-up), 0=oil emplty
    return PinRead(PORTRxOILLEVEL, PINxOILLEVEL);
}
int8_t is_homeSensor(void)
{
    return PinRead(PORTRxSTEPPER_SENSOR_HOME, PINxSTEPPER_SENSOR_HOME);
}
int8_t is_unblocked_nozzle(int8_t nozzle)
{
    int8_t cod_ret=1;
    return cod_ret;
}
int8_t is_inductiveSensorRPM(void)
{
    int8_t cod_ret=1;
    return cod_ret;
}
