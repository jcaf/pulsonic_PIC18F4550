#include "main.h"

static struct _startSignal
{
    int8_t S;
    int8_t sVar;
}startSignal;

#define STARTSIGNAL_DEBOUNCE_TIME 20//ms

/* 1=Not Signal(pull-up), 0=Signal present! */
void startSignal_init(void)
{
    startSignal.sVar = !PinRead(PORTRxSTARTSIGNAL, PINxSTARTSIGNAL);
    startSignal.S = startSignal.sVar;
}
int8_t is_startSignal(void)
{
    static int8_t c;
    static int8_t sm0;
    int8_t signal;
    
    if  (sm0 == 0)
    {
        signal = !PinRead(PORTRxSTARTSIGNAL, PINxSTARTSIGNAL);
        if (startSignal.sVar != signal)
        {
            startSignal.sVar = signal;
            sm0++;
        }
    }
    else if (sm0 == 1)
    {
        if (smain.f.f1ms)
        {
            if (++c == STARTSIGNAL_DEBOUNCE_TIME)//20 ms
            {
                c=0x0;
                signal = !PinRead(PORTRxSTARTSIGNAL, PINxSTARTSIGNAL);
                if (startSignal.sVar == signal)
                    {startSignal.S = startSignal.sVar;}
                sm0 = 0x00;
            }
        }
    }
    return startSignal.S;
}

#define OILLEVEL_DEBOUNCE_TIME 20//ms
static struct _oilLevel
{
    int8_t level;
    int8_t levelVar;
}oilLevel;
/* 1=OIl ok (pull-up), 0=oil emplty */
void oilLevel_init(void)
{
    oilLevel.levelVar = PinRead(PORTRxOILLEVEL, PINxOILLEVEL);
    oilLevel.level = oilLevel.levelVar;
}

int8_t is_oilLevel(void)
{
    static int8_t c;
    static int8_t sm0;
    int8_t signal;
    
    if  (sm0 == 0)
    {
        signal = PinRead(PORTRxOILLEVEL, PINxOILLEVEL);
        if (oilLevel.levelVar != signal)
        {
            oilLevel.levelVar = signal;
            sm0++;
        }
    }
    else if (sm0 == 1)
    {
        if (smain.f.f1ms)
        {
            if (++c == OILLEVEL_DEBOUNCE_TIME)//ms
            {
                c=0x0;
                signal = PinRead(PORTRxOILLEVEL, PINxOILLEVEL);
                if (oilLevel.levelVar == signal)
                    {oilLevel.level = oilLevel.levelVar;}
                sm0 = 0x00;
            }
        }
    }
    return oilLevel.level;
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
