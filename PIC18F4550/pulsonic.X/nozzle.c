#include "main.h"
#include "mpap.h"
#include "pulsonic.h"

double NOZZLE_QTY_DIFF = 0.1;//inc/dec in 0.1

int8_t nozzle_isEnabled(int n)
{
    if (pulsonic.nozzle[n].Q_mlh > 0.0)
        return 1;
    return 0;
}
 
/*return the truncated current position(left adjusted)*/
int8_t nozzle_getPosition(void)
{
    return (mpap_get_numSteps_current()/MPAP_NUMSTEP_1NOZZLE);
}

static int8_t sm0;

void nozzle_setPosition_reset(void)
{
    sm0 = 0;
}

int8_t nozzle_setPosition(int8_t n)
{
    int8_t cod_ret = 0;
    int16_t numSteps_current;
    
    if (sm0 == 0)
    {
        if (n == 0)//go to nozzle 0
        {
            numSteps_current = mpap_get_numSteps_current();

            if (numSteps_current != 0) //diferente del origen
            {
                if (numSteps_current<= ((NOZZLE_NUMMAX-1)*(MPAP_NUMSTEP_1NOZZLE)))
                {
                    //->18->0(arrastrando)
                    //mpap_homming_job_reset();
                    mpap_movetoNozzle(NOZZLE_NUMMAX-1);
                    sm0 = 4;
                }            
                else//se quedo a medias en el traslado de 18->0
                {
                    //completa con el otro metodo
                    if ( (numSteps_current - ((NOZZLE_NUMMAX-1)*(MPAP_NUMSTEP_1NOZZLE)) ) > 0)
                    {
                        //sale y completa, 
                        mpap_movetoNozzle(NOZZLE_NUMMAX);//completa el giro, forzando a ir "1 nozzle" mas de lo permitido
                        sm0 = 1;
                    }
                }
            }
            else
            {
                cod_ret = 1;
            }
                  
        }
        else
        {
            sm0 = 2;    //usa el metodo convencional de traslado
        }
    }
    else if (sm0 == 1)
    {
        if (mpap_isIdle())
        {
            mpap.numSteps_current = 0x0000;//Set to origin = 0

            //COMENTO PORQUE PUEDE SER Q EN LA MANGUERA 0 EL SENSOR HALL YA NO MARQUE CERO
            //Aqui deberia de haber 1 zero 
            //CONFIRMADO, NO HAY 0 EN LA MANGUERA "0", LO PROBE EN OTRO SENSOR... OK.. ENTONCES ESTO QUEDO FUERA DEL PROGRAMA
//            if (PinRead(PORTRxSTEPPER_SENSOR_HOME, PINxSTEPPER_SENSOR_HOME) == 1)//Error?
//            {
//                pulsonic.error.f.homeSensor = 1;
//                pulsonic.flags.homed = 0;
//            }
            cod_ret = 1;
        }
    }
    else if (sm0 == 2)//usa el metodo convencional de traslado-A
    {
        mpap_movetoNozzle(n);
        sm0++;
    }
    else if (sm0 == 3)//usa el metodo convencional de traslado-A
    {
        if (mpap_isIdle())
        {
            cod_ret = 1;
        }
    }
    else if (sm0 == 4)//->18->0(arrastrando)-A
    {
        if (mpap_isIdle())
        {
            mpap_doMovement(1 * MPAP_NUMSTEP_1NOZZLE, MPAP_CROSSING_HOMESENSOR_MODE);
            sm0++;
        }
    }
    else if (sm0 == 5)//->18->0(arrastrando)-B
    {
        if (mpap_isIdle())
        {
            cod_ret = 1;
        }
    }
    
    //end of fx
    if (cod_ret ==1)
    {
        sm0 = 0x00;
    }
    
    return cod_ret;
}