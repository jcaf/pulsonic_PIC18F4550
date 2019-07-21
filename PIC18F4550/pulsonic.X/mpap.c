#include "main.h"
#include "mpap.h"
#include "pulsonic.h"

static void _mpap_step1(void)
{
    PinTo0(PORTWxSTEPPER_A, PINxSTEPPER_A);
    PinTo0(PORTWxSTEPPER_B, PINxSTEPPER_B);
    PinTo0(PORTWxSTEPPER_C, PINxSTEPPER_C);
    PinTo1(PORTWxSTEPPER_D, PINxSTEPPER_D);
    //LATD = 0x08;//LATD = 0x0C;
}
static void _mpap_step2(void)
{
    PinTo0(PORTWxSTEPPER_A, PINxSTEPPER_A);
    PinTo0(PORTWxSTEPPER_B, PINxSTEPPER_B);
    PinTo1(PORTWxSTEPPER_C, PINxSTEPPER_C);
    PinTo0(PORTWxSTEPPER_D, PINxSTEPPER_D);
    //LATD = 0x04;//LATD = 0x06;
}
static void _mpap_step3(void)
{
    PinTo0(PORTWxSTEPPER_A, PINxSTEPPER_A);
    PinTo1(PORTWxSTEPPER_B, PINxSTEPPER_B);
    PinTo0(PORTWxSTEPPER_C, PINxSTEPPER_C);
    PinTo0(PORTWxSTEPPER_D, PINxSTEPPER_D);
    //LATD = 0x02;//LATD = 0x03;
}
static void _mpap_step4(void)
{
    PinTo1(PORTWxSTEPPER_A, PINxSTEPPER_A);
    PinTo0(PORTWxSTEPPER_B, PINxSTEPPER_B);
    PinTo0(PORTWxSTEPPER_C, PINxSTEPPER_C);
    PinTo0(PORTWxSTEPPER_D, PINxSTEPPER_D);
    //LATD = 0x01;//LATD = 0x09;
}

PTRFX_retVOID mpap_step[4] = 
{    
    _mpap_step1, _mpap_step2, _mpap_step3, _mpap_step4
};

static void _mpap_off(void)
{
    PinTo0(PORTWxSTEPPER_A, PINxSTEPPER_A);
    PinTo0(PORTWxSTEPPER_B, PINxSTEPPER_B);
    PinTo0(PORTWxSTEPPER_C, PINxSTEPPER_C);
    PinTo0(PORTWxSTEPPER_D, PINxSTEPPER_D);
}
PTRFX_retVOID mpap_off= {_mpap_off};

volatile struct _mpap mpap;

/* Deja todo listo para girar N pasos, se llama 1 vez */
void mpap_setupToTurn(int16_t numSteps_tomove)
{
	if (numSteps_tomove !=0)
	{
        mpap.KI = (numSteps_tomove<0)? -1: 1; //+-1
		mpap.counter_steps = 0;
        mpap.numSteps_tomove = numSteps_tomove; 
	}
}
void mpap_setupToHomming(void)
{
    //mpap_setupToTurn( -1 *( (NOZZLE_NUMMAX*MPAP_NUMSTEP_1NOZZLE) +20));//direccion negativa
    mpap_setupToTurn( -1 *( (18*MPAP_NUMSTEP_1NOZZLE) +20));//direccion negativa
    
    mpap.mode = MPAP_HOMMING_MODE;
}

/* mpap.numSteps_current se mantiene, no se pierde 
 * DIRECION +1 Pos, -1 negativo
 */
void mpap_do1step(int8_t KI)
{
	static int8_t i = -1;
    
	i= i + KI;
    if (i>3){i=0;}
	if (i<0){i=3;}
    
    mpap_step[i]();
}

/*
 * Da por hecho que desconoce la posicon actual
 * el numero maximo de vueltas en todo el recorrido con un + 10% 
 * Si se para externamente un homming, el sistema tiene que retomar el pendiente
 * mpap.numSteps_current no es relevante mantener la posicon actual porque podria escapar 
 * No se hace uno de timeout, sino de pasos, el sistema el libre de atascamientos 
 * en las partes mecanicas conectadas al motor
 */
int8_t mpap_homming(void)
{
	int8_t cod_ret = 0;
	if (mpap.numSteps_tomove !=0 )
	{
        mpap_do1step(mpap.KI);
		//
		mpap.counter_steps+= mpap.KI;   //inc/dec +-1
                                		
        if (PinRead(PORTRxSTEPPER_SENSOR_HOME, PINxSTEPPER_SENSOR_HOME) == 0)
        {
            mpap.numSteps_tomove = 0x0000;
            mpap.numSteps_current = 0x0000; 
            cod_ret = 1;
        }
        if ( mpap.counter_steps == mpap.numSteps_tomove)//max. num. vueltas
		{
            mpap.numSteps_tomove = 0x0000;
            cod_ret = 2;		
		}
	}
	return cod_ret;
}

/* mode ubicacion en los nozzle */
int8_t mpap_normal_mode(void)
{
    int8_t cod_ret = 0;
    if (mpap.numSteps_tomove!=0)
    {
        mpap_do1step(mpap.KI);
        //
        mpap.counter_steps+= mpap.KI;   //inc/dec +-1
        
        mpap.numSteps_current+= mpap.KI;//+= mpap.counter_steps;//siempre mantiene la posicion 
        
        if ( mpap.counter_steps== mpap.numSteps_tomove)//AQUI PUEDE SER COMPARAR CON < > segun el caso si es negativo o positivo la comparacion
        {
            mpap.numSteps_tomove = 0;
            cod_ret = 1;		//can be abort external...
        }
    }
    return cod_ret;
}

/*la parada debe ser sincronizada en la rutina de interrupcion*/
void mpap_job(void)
{
	int8_t cod_ret;
    if (mpap.mode == MPAP_HOMMING_MODE)
    {
        cod_ret = mpap_homming();
        if (cod_ret == 1)
        {
            pulsonic.flags.homed = 1;
            mpap_off();
            mpap.mode = MPAP_STALL_MODE;
        }
        else if (cod_ret == 2)
        {
            mpap_off();
            mpap.mode = MPAP_STALL_MODE;
            
            //pulsonic.error.f.homeSensor = 1;
        }
    }
    else if (mpap.mode == MPAP_NORMAL_MODE)
    {
        if (mpap_normal_mode())
            mpap.mode = MPAP_STALL_MODE;
    }
    //
    if (mpap.mode == MPAP_STALL_MODE)
    {
        mpap_off();
        mpap.numSteps_tomove = 0x00;
        mpap.mode = MPAP_IDLE_MODE;
    }
}

///* acepta ordenes desde el hilo main*/
//void mpap_sych(void)
//{
//    static int8_t sm0;
//    static int8_t c;
//    if (sm0 == 0)
//    {
//        if ((mpap.mode == MPAP_NORMAL_MODE) || (mpap.mode == MPAP_HOMMING_MODE))
//            sm0++;
//        else if (mpap.mode == MPAP_STALL_MODE)
//            sm0 = 3;
//    }
//    else if (sm0 == 1)//acabo un movimiento
//    {
//        if (mpap.numSteps_tomove == 0)//termino de mover?
//        {
//            if (mpap.mode == MPAP_HOMMING_MODE)
//            {
//                if ( pulsonic.errors.flag.mpap_home_sensor == 1)
//                {
//                    pulsonic.errors.flag.mpap_home_sensor = 0;//clear flag //marcar error de Sensor de posicion en el display
//                }
//            }
//            else if (mpap.mode == MPAP_NORMAL_MODE)
//            {
//            }
//            sm0++;
//            c = 0;
//        }
//    }
//    else if (sm0 == 2)
//    {
//        if (smain.f.f1ms)
//        {
//            if (++c == 10)
//            {
//                c = 0;
//                mpap.mode = MPAP_STALL_MODE;
//                sm0++;
//            }
//        }
//    }
//    else if (sm0 == 3)
//    {
//        if (mpap.mode == MPAP_IDLE_MODE)
//        {
//            sm0 = 0;
//        }
//    }
//}
void mpap_movetoNozzle(int8_t numNozzle)//0..NOZZLE_NUMMAX-1
{
                                                            //mpap_setupToTurn( nozzle * MPAP_NUMSTEP_1NOZZLE);//se escala	
    int16_t numSteps_tomove = (numNozzle*MPAP_NUMSTEP_1NOZZLE) - mpap.numSteps_current;
    if (numSteps_tomove != 0 )
    {
        mpap_setupToTurn(numSteps_tomove);
        mpap.mode = MPAP_NORMAL_MODE;
    }
    
}
int8_t mpap_isIdle(void)
{
    if (mpap.mode == MPAP_IDLE_MODE)
        {return 1;}
    return 0;
}
