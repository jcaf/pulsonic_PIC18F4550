/*
 * File:   main.c
 * Author: jcaf
 *
 * Created on June 29, 2019, 12:51 PM
 * O.S Antergos Linux
 * MPLAB X IDE v4.05
 * Microchip MPLAB XC8 C Compiler V2.05
 * PIC18F4550 @ 48MHz
 * Project: Pulsonic (aceitera)
 * 
 * OBSERVACIONES: 
 * 1) XC8 itoa(buff, int, base) no es "standard" en el orden de los argumentos
 *      en main.h: 
 * #define myitoa(_integer_, _buffer_, _base_) itoa(_buffer_, _integer_, _base_) 
 * 
 */
#include "main.h"
#include "pulsonic.h"

#pragma config "PLLDIV=5", "CPUDIV=OSC1_PLL2", "USBDIV=2", "FOSC=HSPLL_HS", "FCMEN=OFF", "IESO=OFF", "PWRT=ON", , "BORV=3", "VREGEN=ON", "WDT=OFF", "PBADEN=OFF", "LVP=OFF"
#pragma config "MCLRE=ON","BOR=OFF"
#pragma warning disable 752
#pragma warning disable 356
#pragma warning disable 373//warning: (373) implicit signed to unsigned conversion
#include "ikb/ikb.h"

volatile struct _isr_flag
{
    unsigned f1ms: 1;
    unsigned __a: 7;
} isr_flag = {0};
volatile struct _main_flag main_flag;



#define MPAP_NUMSTEP_1NOZZLE 200//200 steps to move to 1 nozzle
//#define NOZZLE_NUMMAX 18//0,1,-...-(NOZZLE_NUMMAX-1)


//PUMP MOTOR
#define PORTWxPUMP LATE
#define PORTRxPUMP PORTE
#define CONFIGIOxPUMP TRISE
#define PINxPUMP  0

#define PUMP_DISABLE()  do{PinTo1(PORTWxPUMP, PINxPUMP);}while(0)
#define PUMP_ENABLE()   do{PinTo0(PORTWxPUMP, PINxPUMP);}while(0)
   
//MOTOR PAP UNIPOLAR
#define PORTWxSTEPPERBIP0_A LATD
#define PORTRxSTEPPERBIP0_A PORTD
#define CONFIGIOxSTEPPERBIP0_A TRISD
#define PINxSTEPPERBIP0_A  0
//
#define PORTWxSTEPPERBIP0_B LATD
#define PORTRxSTEPPERBIP0_B PORTD
#define CONFIGIOxSTEPPERBIP0_B TRISD
#define PINxSTEPPERBIP0_B  1
//
#define PORTWxSTEPPERBIP0_C LATD
#define PORTRxSTEPPERBIP0_C PORTD
#define CONFIGIOxSTEPPERBIP0_C TRISD
#define PINxSTEPPERBIP0_C  2
//
#define PORTWxSTEPPERBIP0_D LATD
#define PORTRxSTEPPERBIP0_D PORTD
#define CONFIGIOxSTEPPERBIP0_D TRISD
#define PINxSTEPPERBIP0_D  3

#define PORTWxSTEPPERBIP0_ENABLE LATC
#define PORTRxSTEPPERBIP0_ENABLE PORTC
#define CONFIGIOxSTEPPERBIP0_ENABLE TRISC
#define PINxSTEPPERBIP0_ENABLE  1
//enable is inverted by hardware
#define STEPPERBIP0_ENABLE() do{PinTo0(PORTWxSTEPPERBIP0_ENABLE, PINxSTEPPERBIP0_ENABLE);}while(0)
#define STEPPERBIP0_DISABLE() do{PinTo1(PORTWxSTEPPERBIP0_ENABLE, PINxSTEPPERBIP0_ENABLE);}while(0)
//
#define PORTWxSTEPPERBIP0_SENSOR_HOME LATE
#define PORTRxSTEPPERBIP0_SENSOR_HOME PORTE
#define CONFIGIOxSTEPPERBIP0_SENSOR_HOME TRISE
#define PINxSTEPPERBIP0_SENSOR_HOME 1

//////////////////////////////////////////////////////
static void _mpap_step1(void)
{
    PinTo0(PORTWxSTEPPERBIP0_A, PINxSTEPPERBIP0_A);
    PinTo0(PORTWxSTEPPERBIP0_B, PINxSTEPPERBIP0_B);
    PinTo0(PORTWxSTEPPERBIP0_C, PINxSTEPPERBIP0_C);
    PinTo1(PORTWxSTEPPERBIP0_D, PINxSTEPPERBIP0_D);
}
static void _mpap_step2(void)
{
    PinTo0(PORTWxSTEPPERBIP0_A, PINxSTEPPERBIP0_A);
    PinTo0(PORTWxSTEPPERBIP0_B, PINxSTEPPERBIP0_B);
    PinTo1(PORTWxSTEPPERBIP0_C, PINxSTEPPERBIP0_C);
    PinTo0(PORTWxSTEPPERBIP0_D, PINxSTEPPERBIP0_D);
}
static void _mpap_step3(void)
{
    PinTo0(PORTWxSTEPPERBIP0_A, PINxSTEPPERBIP0_A);
    PinTo1(PORTWxSTEPPERBIP0_B, PINxSTEPPERBIP0_B);
    PinTo0(PORTWxSTEPPERBIP0_C, PINxSTEPPERBIP0_C);
    PinTo0(PORTWxSTEPPERBIP0_D, PINxSTEPPERBIP0_D);
}
static void _mpap_step4(void)
{
    PinTo1(PORTWxSTEPPERBIP0_A, PINxSTEPPERBIP0_A);
    PinTo0(PORTWxSTEPPERBIP0_B, PINxSTEPPERBIP0_B);
    PinTo0(PORTWxSTEPPERBIP0_C, PINxSTEPPERBIP0_C);
    PinTo0(PORTWxSTEPPERBIP0_D, PINxSTEPPERBIP0_D);
}

/////////////////////////
//PTRFX_retVOID mpap_step[NUM_STEPPER_BIPOLAR][4] = 
PTRFX_retVOID mpap_step[4] = 
{
    //{
        _mpap_step1, _mpap_step2, _mpap_step3, _mpap_step4
//},
};
//mpap_off is replaced by the hardware via STEPPERBIP0_DISABLE()
static inline void _mpap_off(void)
{
    PinTo0(PORTWxSTEPPERBIP0_A, PINxSTEPPERBIP0_A);
    PinTo0(PORTWxSTEPPERBIP0_B, PINxSTEPPERBIP0_B);
    PinTo0(PORTWxSTEPPERBIP0_C, PINxSTEPPERBIP0_C);
    PinTo0(PORTWxSTEPPERBIP0_D, PINxSTEPPERBIP0_D);
}
//PTRFX_retVOID mpap_off[NUM_STEPPER_BIPOLAR]= {_mpap_off};
PTRFX_retVOID mpap_off= {_mpap_off};
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
enum MPAP_MODES{
    IDLE_MODE=0,
    STALL_MODE,
    HOMMING_MODE,
    NORMAL_MODE
};

struct _mpap
{
	int16_t numSteps_current;//current position absolute
	int16_t counter_steps;
	int16_t numSteps_tomove;
	int8_t 	KI;//+-1
    
	// struct _mpap_flag
	// {
	// 	unsigned run:1;
	// 	unsigned __a:7;
	// }flag;
    int8_t mode;
};
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
        mpap.numSteps_tomove = numSteps_tomove; 
        mpap.KI = (mpap.numSteps_tomove<0)? -1: 1; //+-1
        //        
		mpap.counter_steps = 0;
		//mpap.flag.run = 1;	
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
}
//int8_t nozzle_get_pos(void)//trunca, es solo referencial
//{
//	return (mpap.numSteps_current/MPAP_NUMSTEP_1NOZZLE);
//}
/*
 * mpap.numSteps_current se mantiene, no se pierde
 */
void mpap_stop(void)		
{
	mpap.numSteps_tomove = 0x00;//mpap.flag.run = 0;
	mpap_off();
    //STEPPERBIP0_DISABLE();
}

/////////// PROBAR SI 1 PULSO DESDE 0000 A 1 SE MUEVE EN UN PASITO!!!
void mpap_do1step(int8_t KI)//DIRECION +1 Pos, -1 negativo
{
	static int8_t i;
	mpap_step[i]();//trae los pasos 0 1 2 3
	i+=KI;
	if (i>3){i=0;}
	if (i<0){i=3;}
}


/*
 * Da por hecho que desconoce la posicon actual
 * el numero maximo de vueltas en todo el recorrido con un + 10% 
 */

/* Si se para externamente un homming, el sistema tiene que retomar el pendiente
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
		mpap.counter_steps+= mpap.KI;               //inc/dec +-1
                                		//mpap.numSteps_current += mpap.counter_steps;//siempre mantiene la posicion 
        if (PinRead(PORTRxSTEPPERBIP0_SENSOR_HOME, PINxSTEPPERBIP0_SENSOR_HOME) == 0)
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
        
        if ( mpap.counter_steps == mpap.numSteps_tomove)//AQUI PUEDE SER COMPARAR CON < > segun el caso si es negativo o positivo la comparacion
        {
            mpap.numSteps_tomove = 0;//mpap.flag.run = 0;	//end
            cod_ret = 1;		//can be abort external...
        }
    }
    return cod_ret;
}
//la parada debe ser sincronizada
int8_t mpap_job(void)
{
	int8_t cod_ret;// = 0;
    
    if (mpap.mode == HOMMING_MODE)
        cod_ret = mpap_homming();
    else if (mpap.mode == NORMAL_MODE)
        cod_ret = mpap_normal_mode();
    
    else if (mpap.mode == STALL_MODE)
        mpap.mode = IDLE_MODE;
    
	return cod_ret;
}
uint16_t t;

extern struct _multiplexedDisp multiplexedDisp[DISP_TOTAL_NUMMAX];

void main(void) 
{
    LATA = 0x00;
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    LATB = 0x00;    //0B00000111;
    //TRISB= 0x00;   //All segments controlled by NPN    
    
    //All analog inputs as DIGITAL
    ADCON1 = 0x0F;
    CMCON=0xCF; //POR default mode comparators OFF
            
    //RC4/RC5 config as digital inputs
    UCON = 0;   //USBEN Disable
    UCFG = 1<<3;//UTRDIS Digital input enable RC4/RC5
    
    //LATA = 0x0FF;
    //TRISA = 0;
    
    //.....
    T0CON = 0B10000111; //16BITS
    t=TMR16B_OVF(1e-3, 256);
    TMR0H = (uint8_t)(TMR16B_OVF(1e-3, 256) >> 8);
    TMR0L = (uint8_t)(TMR16B_OVF(1e-3, 256));
    TMR0IE = 1;
    //.....

    PUMP_DISABLE();
    ConfigOutputPin(CONFIGIOxPUMP, PINxPUMP);
    
    ConfigOutputPin(CONFIGIOxSTEPPERBIP0_A, PINxSTEPPERBIP0_A);
    ConfigOutputPin(CONFIGIOxSTEPPERBIP0_B, PINxSTEPPERBIP0_B);
    ConfigOutputPin(CONFIGIOxSTEPPERBIP0_C, PINxSTEPPERBIP0_C);
    ConfigOutputPin(CONFIGIOxSTEPPERBIP0_D, PINxSTEPPERBIP0_D);
    
    STEPPERBIP0_ENABLE();
    ConfigOutputPin(CONFIGIOxSTEPPERBIP0_ENABLE, PINxSTEPPERBIP0_ENABLE);
    //
    ConfigInputPin(CONFIGIOxSTEPPERBIP0_SENSOR_HOME, PINxSTEPPERBIP0_SENSOR_HOME);

    display7s_init();
    ikb_init();
    
    disp_show_quantity(99.5);
    //
    GIE = 1;
    int8_t c=0;
    int8_t c_disp=0;
    while(1)
    {
        if (isr_flag.f1ms)//sync para toda la pasada
        {
            isr_flag.f1ms = 0;
            main_flag.f1ms = 1;
        }

        if (main_flag.f1ms)
        {
            if (++c==20)
            {
                c = 0;
            
                ikb_job();
                //
                if (ikb_key_is_ready2read(0))
                {
                    PinToggle(LATB,0);
                }
                if (ikb_key_is_ready2read(1))
                {
                    PinToggle(LATB,1);
                }
                if (ikb_key_is_ready2read(2))
                {
                    PinToggle(LATB,2);
                }
                if (ikb_key_is_ready2read(3))
                {
                    PinToggle(LATB,3);
                }
                if (ikb_key_is_ready2read(4))
                {
                    PinToggle(LATB,4);
                }
            }
            //
            if (++c_disp == 2)
            {
                c_disp = 0;
                display7s_job();
            }
        }
        //
        //////////
        main_flag.f1ms = 0;
        ikb_flush();
    }
    
    
    //+-Secuencia para entrar a homming
    //mpap.mode = STALL_MODE;
    //while (mpap.mode!= IDLE_MODE)
    //    ;
    mpap_setupToHomming();
    //-+
    while (1)
    {
        mpap_job();
        if (mpap.numSteps_tomove == 0)
        {
            if ( pulsonic.errors.flag.mpap_home_sensor != 1)
            {
                mpap.mode = NORMAL_MODE;//llego al inicio sin problemas
                break;
            }
            else
            {
                //limpiar el error
                //marcar error de Sensor de posicion
            }
        }
         __delay_ms(2);
    }
    __delay_ms(1000);

    //mpap.mode = STALL_MODE;
    
    nozzle_moveto(1);
    while (1)
    {
        mpap_job();
        if (mpap.numSteps_tomove == 0)
        {
            break;
        }
        __delay_ms(2);
    }
    mpap_stop();
    
    //while(1);
        
    while (1)
    {
        PinTo1(PORTWxPUMP, PINxPUMP);
        __delay_ms(50);
        PinTo0(PORTWxPUMP, PINxPUMP);
        __delay_ms(50);
    }
    
    while(1);
    
    __delay_ms(1000);
    
    nozzle_moveto(2);
    while (1)
    {
        mpap_job();
        if (mpap.numSteps_tomove == 0)
        {
            break;
        }
        __delay_ms(2);
    }

    
    while (1)
    {;}
    
    return;
}

void interrupt INTERRUPCION(void)//@1ms 
{
    //static uint8_t counter = 0;
    if (TMR0IF)
    {
        isr_flag.f1ms = 1;
        
        TMR0IF = 0;
        TMR0H = (uint8_t)(TMR16B_OVF(1e-3, 256) >> 8);
        TMR0L = (uint8_t)(TMR16B_OVF(1e-3, 256));
    }
}