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
#include "mpap.h"

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


extern struct _multiplexedDisp multiplexedDisp[DISP_TOTAL_NUMMAX];



int8_t pump_tick(void)
{
    int8_t cod_ret = 0;
    static int8_t sm0;
    static int8_t c;
    
    if (sm0 == 0)
    {
        PUMP_ENABLE();
        c = 0x0;
        sm0++;
    }
    else if (sm0 == 1)
    {
        if (main_flag.f1ms)
        {
            if (++c == 100)
            {
                c = 0;
                PUMP_DISABLE();
                cod_ret = 1;
            }
        }
    }
    return cod_ret;
}

struct _lock
{
    unsigned kb:1;
    unsigned autoMode:1;
    unsigned visualizerMode:1;
    unsigned __a:5;
}lock;

enum _MACHINESTATE
{
    STALL = 0,
    RUNNING,
    CONFIG
};
int8_t machineState;

enum _DISPOWNER_MODE
{
	DISPOWNER_AUTO_MODE = 0,
	DISPOWNER_VISUALIZER_MODE,
	DISPOWNER_CONFIG_MODE,
};
int8_t disp_owner = DISPOWNER_AUTO_MODE;

void main(void) 
{
    int8_t c_access_kb=0;
    int8_t c_access_disp=0;

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
    T0CON = 0B10000111; //16BITS
    //TMR0H = (uint8_t)(TMR16B_OVF(2e-3, 256) >> 8);
    //TMR0L = (uint8_t)(TMR16B_OVF(2e-3, 256));
    TMR0H = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256) >> 8);
    TMR0L = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256));
    TMR0IE = 1;
    //.....
    PUMP_DISABLE();
    ConfigOutputPin(CONFIGIOxPUMP, PINxPUMP);
    
    ConfigOutputPin(CONFIGIOxSTEPPER_A, PINxSTEPPER_A);
    ConfigOutputPin(CONFIGIOxSTEPPER_B, PINxSTEPPER_B);
    ConfigOutputPin(CONFIGIOxSTEPPER_C, PINxSTEPPER_C);
    ConfigOutputPin(CONFIGIOxSTEPPER_D, PINxSTEPPER_D);
    
    STEPPER_ENABLE();
    ConfigOutputPin(CONFIGIOxSTEPPER_ENABLE, PINxSTEPPER_ENABLE);
    //
    ConfigInputPin(CONFIGIOxSTEPPER_SENSOR_HOME, PINxSTEPPER_SENSOR_HOME);

    ConfigInputPin(CONFIGIOxLEVELOIL, PINxLEVELOIL);//ext. pullup
    ConfigInputPin(CONFIGIOxSTARTSIGNAL, PINxSTARTSIGNAL);//ext. pullup
    
    display7s_init();
    ikb_init();
    
    disp_show_quantity(77.7);
    GIE = 1;

    mpap_setupToHomming();
    
    

    while(1)
    {
        if (isr_flag.f1ms)//sync para toda la pasada
        {
            isr_flag.f1ms = 0;
            main_flag.f1ms = 1;
        }

        if (main_flag.f1ms)
        {
            if (++c_access_kb == 20)
            {
                c_access_kb = 0;
            
                //keyboard
                if (!lock.kb)
                {
                    ikb_job();
                    
                    if (1)//en que modo estoy
                    {
                        disp_owner = DISPOWNER_VISUALIZER_MODE;
                    }
                    else if (1)
                    {
                        disp_owner = DISPOWNER_AUTO_MODE;
                    }
                    else //entra y sale de config
                    {
                        machineState = CONFIG; //config
                        machineState = RUNNING;
                    }	
                }
            }
            
            //display
            if (++c_access_disp == 2)
            {
                c_access_disp = 0;
                display7s_job();
            }
        }
        //+--------------------------------------------------------------------
        if (machineState == RUNNING)//solo para darle mas claridad a la lectura del programa, xq podria manejarse con locks
        {
            //sobre estos 2, igual la maquina puede entrar a modo "Config"
            if (!lock.autoMode)//esto es un proceso
            {
                //autoMode()//cada proceso puede apropiarse del display
                {
                    //1) proceso
                    //2) display
                    if (disp_owner == DISPOWNER_AUTO_MODE)
                    {
                    }
                }	 
            }

            if (!lock.visualizerMode)//este es otro proceso
            {
                //visualizerMode();
                //1) proceso
                            //despues de un tiempo sale automaticamente	
                //2) display
                if (disp_owner == DISPOWNER_VISUALIZER_MODE)
                {
                }
            }
        }
        else if (machineState == CONFIG)
        {
            //config()	//para la maquina y espera el start --> aqui va a estar a la espera del start
                        //aqui podria convivir con el error en "paralelo"	
            //1)proceso
            //2)display
            if (disp_owner == DISPOWNER_CONFIG_MODE)
            {
            }
        }
        
        
        
        
        mpap_sych();
        
        //////////
        main_flag.f1ms = 0;
        ikb_flush();
    }
}
void interrupt INTERRUPCION(void)//@1ms 
{
    if (TMR0IF)
    {
        isr_flag.f1ms = 1;
        mpap_job();
        //
        TMR0IF = 0;
        TMR0H = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256) >> 8);
        TMR0L = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256));
    }
}
/*
    int8_t is_levelOil(void);
    int8_t is_startSignal(void);
    int8_t is_homeSensor(void);
    int8_t is_unblocked_nozzle(int8_t nozzle);
    int8_t is_inductiveSensorRPM(void);

 */
//////////////////////////////
void errorHandler_startSignal(void)
{
	static int8_t sm0;
//uint8_t display7s[DISP_TOTAL_NUMMAX];
    
	if (sm0 == 0)
	{
		if (  (machineState == WORKING_MODE) && (!is_startSignal()) ) //cada error decide apropiarse del display
		{
			machineState = STALL;
			sm0++;
		}	
	}
	else if (sm0 == 1) //2 partes: el proceso y el display
	{
		//1) el proceso
        if ( is_startSignal() )//desaparecio el error?
		{
            
		}
		else
		{

		}

		//una una version buffered de los display, para luego volcarse en su debido
		//momento
		//
        flag = 00000010;//su respectivo flag
		
		//2) necesito el display?
	}
}
union _errorFlag
{
    struct _errorFlag_flags
    {
        unsigned startSignal:1;
        unsigned levelOil:1;
        unsigned homeSensor:1;
        unsigned unblockedNozzle:1;
        unsigned inductiveSensorRPM:1;
        unsigned __a:1;
        unsigned __b:1;
        unsigned __c:1;
        //si es mas de 8 - > modificar NUMMAX_ERRORS.. maximo 16
    }f;
    intNumMaxErr_t packed;
};
union _errorFlag errorSignal;//senala requiere
union _errorFlag errorPermission;//aqui el tiene el permiso en la misma ubicacion

////////////////////////////
void errorHandler_queue(void)
{
    //static intNumMaxErr_t i;
    static int8_t i;
    static int8_t sm0;
    static uint16_t c;
	static _errorFlag permission;
    
    if (sm0 == 0)
    {
        permission = errorSignal.packed & (1<<i);
        if (permission)
        {
            errorPermission.packed = permission;//solo 1 tiene el permiso
            c = 0x0;
        }
    }
    else if (sm0 == 1)
    {
        if ( errorSignal.packed & (1<<i) )//aun se mantiene?
        {
            if (main_flag.f1ms)
            {
                if (++c == 2000)//2 ms
                {
                    if (++i == NUMMAX_ERRORS)
                    {
                        i = 0;
                    }
                }
            }
        }
        else//kill 
        {
            if (++i==NUMMAX_ERRORS)
            {
                i = 0;
            }
        }
    }
    
}
