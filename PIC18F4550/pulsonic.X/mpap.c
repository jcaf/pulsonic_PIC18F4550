#include "main.h"
#include "system.h"
#include "mpap.h"
#include "types.h"
#include "pulsonic.h"
/////////////////UNIPOLAR /////////////////////////////////////
//Secuencia Full unipolar 
//        LATD = 0B00001100;
//        __delay_ms(1);
//        LATD = 0B00000110;
//        __delay_ms(1);
//        LATD = 0B00000011;
//        __delay_ms(1);
//        LATD = 0B00001001;
//        __delay_ms(1);
//MICROPASO MITAD
//        LATD = 0B00001000;
//        __delay_ms(1);
//        LATD = 0B00001100;
//        __delay_ms(1);
//        LATD = 0B00000100;
//        __delay_ms(1);
//        LATD = 0B00000110;
//        __delay_ms(1);
//        LATD = 0B00000010;
//        __delay_ms(1);
//        LATD = 0B00000011;
//        __delay_ms(1);
//        LATD = 0B00000001;
//        __delay_ms(1);
//        LATD = 0B00001001;
//        __delay_ms(1);
static void _mpap_step1(void)
{
//    PinTo0(PORTWxSTEPPER_A, PINxSTEPPER_A);
//    PinTo0(PORTWxSTEPPER_B, PINxSTEPPER_B);
//    PinTo1(PORTWxSTEPPER_C, PINxSTEPPER_C);
//    PinTo1(PORTWxSTEPPER_D, PINxSTEPPER_D);
    //LATD = 0x0C;
    LATD = 0x08;
}
static void _mpap_step2(void)
{
//    PinTo0(PORTWxSTEPPER_A, PINxSTEPPER_A);
//    PinTo1(PORTWxSTEPPER_B, PINxSTEPPER_B);
//    PinTo1(PORTWxSTEPPER_C, PINxSTEPPER_C);
//    PinTo0(PORTWxSTEPPER_D, PINxSTEPPER_D);
    //LATD = 0x06;
    LATD = 0x04;
}
static void _mpap_step3(void)
{
//    PinTo1(PORTWxSTEPPER_A, PINxSTEPPER_A);
//    PinTo1(PORTWxSTEPPER_B, PINxSTEPPER_B);
//    PinTo0(PORTWxSTEPPER_C, PINxSTEPPER_C);
//    PinTo0(PORTWxSTEPPER_D, PINxSTEPPER_D);
    //LATD = 0x03;
    LATD = 0x02;
}
static void _mpap_step4(void)
{
//    PinTo1(PORTWxSTEPPER_A, PINxSTEPPER_A);
//    PinTo0(PORTWxSTEPPER_B, PINxSTEPPER_B);
//    PinTo0(PORTWxSTEPPER_C, PINxSTEPPER_C);
//    PinTo1(PORTWxSTEPPER_D, PINxSTEPPER_D);
    //LATD = 0x09;
    LATD = 0x01;
}
//PTRFX_retVOID mpap_step[NUM_STEPPER_BIPOLAR][4] = 
PTRFX_retVOID mpap_step[4] = 
{    
    _mpap_step1, _mpap_step2, _mpap_step3, _mpap_step4
};

static inline void _mpap_off(void)
{
    PinTo0(PORTWxSTEPPER_A, PINxSTEPPER_A);
    PinTo0(PORTWxSTEPPER_B, PINxSTEPPER_B);
    PinTo0(PORTWxSTEPPER_C, PINxSTEPPER_C);
    PinTo0(PORTWxSTEPPER_D, PINxSTEPPER_D);
}
PTRFX_retVOID mpap_off= {_mpap_off};//PTRFX_retVOID mpap_off[NUM_STEPPER_BIPOLAR]= {_mpap_off};

volatile struct _mpap mpap;
/*
Deja todo listo para girar N pasos, se llama 1 vez
*/
//void mpap_setupToTurn(int16_t absPos_insteps)//posAbs_insteps: Absoulte position(in num steps)
void mpap_setupToTurn(int16_t numSteps_tomove)//posAbs_insteps: Absoulte position(in num steps)
{
	//mpap.numSteps_tomove = absPos_insteps - mpap.numSteps_current;
	if (numSteps_tomove !=0)
	{
        mpap.KI = (numSteps_tomove<0)? -1: 1; //+-1
        //        
		mpap.counter_steps = 0;
		//mpap.flag.run = 1;	
        mpap.numSteps_tomove = numSteps_tomove; 
	}
}
void mpap_setupToHomming(void)
{
    mpap_setupToTurn( -1 *( (NOZZLE_NUMMAX*MPAP_NUMSTEP_1NOZZLE) +20));//direccion negativa
    //mpap.numSteps_current = 0x0000; //Obligaria al sistema a reponer el sensor
                                      //y arrancar desde 0
    mpap.mode = HOMMING_MODE;
}
void nozzle_moveto(int8_t nozzle)//0..NOZZLE_NUMMAX-1
{
	//mpap_setupToTurn( nozzle * MPAP_NUMSTEP_1NOZZLE);//se escala	
    mpap_setupToTurn( (nozzle*MPAP_NUMSTEP_1NOZZLE) - mpap.numSteps_current);
    
    mpap.mode = NORMAL_MODE;
}
//int8_t nozzle_get_pos(void)//trunca, es solo referencial
//{return (mpap.numSteps_current/MPAP_NUMSTEP_1NOZZLE);
//}
 
/*
 * mpap.numSteps_current se mantiene, no se pierde
 */
//void mpap_stop(void)		
//{
//	mpap.numSteps_tomove = 0x00;//mpap.flag.run = 0;
//	mpap_off();
//    //STEPPER_DISABLE();
//}
void mpap_do1step(int8_t KI)//DIRECION +1 Pos, -1 negativo
{
	static int8_t i = -1;
    
	i= i + KI;
	
    if (i>3){i=0;}
	if (i<0){i=3;}
    
    mpap_step[i]();//trae los pasos 0 1 2 3
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
	if (mpap.numSteps_tomove!=0)//(mpap.flag.run)
	{
        mpap_do1step(mpap.KI);
		//
		mpap.counter_steps+= mpap.KI;   //inc/dec +-1
                                		//mpap.numSteps_current += mpap.counter_steps;//siempre mantiene la posicion 
        if (PinRead(PORTRxSTEPPER_SENSOR_HOME, PINxSTEPPER_SENSOR_HOME) == 0)
        {
            mpap.numSteps_tomove = 0x0000;
            cod_ret = 1;
        }
        if ( mpap.counter_steps == mpap.numSteps_tomove)
		{
            mpap.numSteps_tomove = 0x0000;
            cod_ret = 1;		
            pulsonic.errors.flag.mpap_home_sensor = 1;
		}
	}
	return cod_ret;
}
int8_t mpap_normal_mode(void)//mode ubicacion en los nozzle
{
    int8_t cod_ret = 0;
    if (mpap.numSteps_tomove!=0)//(mpap.flag.run)
    {
        mpap_do1step(mpap.KI);
        //
        mpap.counter_steps+= mpap.KI;               //inc/dec +-1
        
        mpap.numSteps_current+= mpap.KI;// += mpap.counter_steps;//siempre mantiene la posicion 
        
        if ( mpap.counter_steps== mpap.numSteps_tomove)//AQUI PUEDE SER COMPARAR CON < > segun el caso si es negativo o positivo la comparacion
        {
            mpap.numSteps_tomove = 0;//mpap.flag.run = 0;	//end
            cod_ret = 1;		//can be abort external...
        }
    }
    return cod_ret;
}
//la parada debe ser sincronizada
int8_t mpap_job(void)//se ejecuta desde la rutina de interrupcion
{
	int8_t cod_ret;// = 0;
    
    if (mpap.mode == HOMMING_MODE)
        {cod_ret = mpap_homming();}
    
    else if (mpap.mode == NORMAL_MODE)
        {cod_ret = mpap_normal_mode();}
    
    else if (mpap.mode == STALL_MODE)
    {
        mpap.numSteps_tomove = 0x00;//mpap.flag.run = 0;
        mpap_off();
        //
        mpap.mode = IDLE_MODE;
    }
	return cod_ret;
}

 
void mpap_sych(void)//synch ISR
{
    static int8_t sm0;
    static int8_t c;
    if (sm0 == 0)//acepta ordenes
    {
        if ((mpap.mode == NORMAL_MODE) || (mpap.mode == HOMMING_MODE))
        {
            sm0++;
        }
        else if (mpap.mode == STALL_MODE)
        {
            sm0 = 2;
        }
    }
    else if (sm0 == 1)//acabo un movimiento
    {
        if (mpap.numSteps_tomove == 0)//termino de mover?
        {
            if (mpap.mode == HOMMING_MODE)
            {
                if ( pulsonic.errors.flag.mpap_home_sensor == 1)
                {
                    pulsonic.errors.flag.mpap_home_sensor = 0;//clear flag
                    //marcar error de Sensor de posicion en el display
                }
            }
            else if (mpap.mode == NORMAL_MODE)
            {
            }
            sm0++;
            c = 0;
        }
    }
    else if (sm0 == 2)
    {
        if (main_flag.f1ms)
        {
            if (++c == 10)
            {
                //c = 0;
                mpap.mode = STALL_MODE;
                sm0++;
            }
        }
    }
    else if (sm0 == 3)
    {
        if (mpap.mode == IDLE_MODE)
        {
            //libera para estar apto a cualquier orden
            sm0 = 0;
        }
    }
}
/*
void mpap_test(void)
{
    static int8_t k = 10;
    static int8_t sm0;
    
    if (sm0 == 0)//acepta ordenes
    {
        if ((mpap.mode == NORMAL_MODE) || (mpap.mode == HOMMING_MODE))
        {
            sm0++;
        }
        else if (mpap.mode == STALL_MODE)
        {
            sm0 = 2;
        }
    }
    else if (sm0 == 1)//acabo un movimiento
    {
        if (mpap.numSteps_tomove == 0)//termino de mover?
        {
            if (mpap.mode == HOMMING_MODE)
            {
                if ( pulsonic.errors.flag.mpap_home_sensor == 1)
                {
                    pulsonic.errors.flag.mpap_home_sensor = 0;//clear flag
                    //marcar error de Sensor de posicion en el display
                }
            }
            else if (mpap.mode == NORMAL_MODE)
            {
            }
            __delay_ms(20);
            mpap.mode = STALL_MODE;
            sm0++;
        }
    }
    else if (sm0 == 2)
    {
        if (mpap.mode == IDLE_MODE)
        {
            //libera para estar apto a cualquier orden
            sm0 = 0;
            __delay_ms(2000);
            
            k = k*-1;
            mpap_setupToTurn( (k*MPAP_NUMSTEP_1NOZZLE));
            
            mpap.mode = NORMAL_MODE;
        }
    }
}

void mpap_1(void)
{
#define nk 10
#define dd 2
    int i;
    
    LATD = 0;
    
    while (1)
    {
        for (i=0; i<50*nk; i++)
        {
            LATD = 0B00001000;
            __delay_ms(dd);
            LATD = 0B00000100;
            __delay_ms(dd);
            LATD = 0B00000010;
            __delay_ms(dd);
            LATD = 0B00000001;
            __delay_ms(dd);
        }
        __delay_ms(20);
        LATD = 0;
        __delay_ms(1000);

        for (i=0; i<50*nk; i++)
        {
            LATD = 0B00000010;
            __delay_ms(dd);
            LATD = 0B00000100;
            __delay_ms(dd);
            LATD = 0B00001000;
            __delay_ms(dd);
            LATD = 0B00000001;
            __delay_ms(dd);
        }
        __delay_ms(20);
        LATD = 0;
        __delay_ms(1000);
    }
    return;
}
*/

/*
void mpap_1(void)
{
#define nk 1
#define dd 6
    int i;
    LATD = 0;
    while (1)
    {
        for (i=0; i<50*nk; i++)
        {
            LATD = 0B00001100;
            __delay_ms(dd);
            LATD = 0B00000110;
            __delay_ms(dd);
            LATD = 0B00000011;
            __delay_ms(dd);
            LATD = 0B00001001;
            __delay_ms(dd);
        }
        __delay_ms(20);
        LATD = 0;
        __delay_ms(2000);
        
        for (i=0; i<50*nk; i++)
        {
            LATD = 0B00000011;
            __delay_ms(dd);
            LATD = 0B00000110;
            __delay_ms(dd);
            LATD = 0B00001100;
            __delay_ms(dd);
            LATD = 0B00001001;
            __delay_ms(dd);
        }
        __delay_ms(20);
        LATD = 0;
        __delay_ms(2000);
    }
    return;
}

 */