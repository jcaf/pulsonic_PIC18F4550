#include "main.h"

int8_t is_levelOil(void)
{
    return PinRead(PORTRxLEVELOIL, PINxLEVELOIL);
}
int8_t is_startSignal(void)
{
    return PinRead(PORTRxSTARTSIGNAL, PINxSTARTSIGNAL);
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
//////////////////
//default
mode = config;

if (error)//background
{
    mode = error;//como se recupera del error?
    
    //como se recupera??
    if last_error()
            -> reset como si de nuevo se tratase
            
            cambiar el foco??
                focous
}
///////////////////////////////////////////////////////////
if ( disp_owner == normal_process)
{
    if (mode == auto)
    {
    }
    else if (mode == config)//prog
    {
    }
}

//lo mejor es que error se maneje independientemente, porque tal vez se 
//necesita que siga trabajando mode prog o auto...
void error_handler(void)
{
    if (mode == error)//aqui puede estar blinking
    {
           //hace lo que tiene que hacer
            if (start)
            {
                if ( disp_owner == error)
                {
                }

                //maquina se para
            }
            else if ()
            {
                relay se abre
            }

    }    
}
