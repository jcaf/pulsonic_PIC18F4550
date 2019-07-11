#include "main.h"

int8_t is_startSignal(void)
{
    return PinRead(PORTRxSTARTSIGNAL, PINxSTARTSIGNAL);
}
int8_t is_oilLevel(void)
{
    return PinRead(PORTRxoilLevel, PINxoilLevel);
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
