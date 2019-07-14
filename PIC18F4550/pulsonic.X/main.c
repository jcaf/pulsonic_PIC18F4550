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
#include "inputs.h"
#include "ikb/ikb.h"
#include "pump.h"
#include "display.h"
#include "automode.h"
#include "visualizerMode.h"

#pragma config "PLLDIV=5", "CPUDIV=OSC1_PLL2", "USBDIV=2", "FOSC=HSPLL_HS", "FCMEN=OFF", "IESO=OFF", "PWRT=ON", , "BORV=3", "VREGEN=ON", "WDT=OFF", "PBADEN=OFF", "LVP=OFF"
#pragma config "MCLRE=ON","BOR=OFF"

volatile struct _isr_flag
{
    unsigned f1ms: 1;
    unsigned __a: 7;
} isr_flag = {0};
volatile struct _smain smain;
 
struct _unlock
{
    unsigned kb:1;
    unsigned autoMode:1;
    unsigned visMode:1;
    unsigned __a:5;
};
//extern struct _unlock unlock;
struct _unlock unlock;

enum _MACHSTATE
{
    MACHSTATE_STALL = 0,
    MACHSTATE_RUNNING,
    MACHSTATE_CONFIG
};
int8_t machState;



int8_t disp_owner = DISPOWNER_AUTOMODE;

////////////////////////////////////////////////////////////////////////////////
union _errorDispFlag
{
    struct _errorFlag_flags
    {
        unsigned startSignal:1;
        unsigned oilLevel:1;
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
union _errorDispFlag error_requestToWriteDisp;//senala requiere el display
union _errorDispFlag error_grantedToWriteDisp;//aqui el tiene el permiso en la misma ubicacion


void main(void) 
{
    int8_t c_access_kb=0;
    int8_t c_access_disp=0;
    //int8_t kb_focus=KB_FOCUS_AUTOMODE;
    int8_t codapp;

    LATA = 0x00;
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    LATB = 0x00;    
    
    ADCON1 = 0x0F;  //All analog inputs as DIGITAL
    CMCON=0xCF;     //POR default mode comparators OFF
            
                    //RC4/RC5 config as digital inputs
    UCON = 0;       //USBEN Disable
    UCFG = 1<<3;    //UTRDIS Digital input enable RC4/RC5
    T0CON = 0B10000111; //16BITS
    
    TMR0H = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256) >> 8);//TMR0H = (uint8_t)(TMR16B_OVF(2e-3, 256) >> 8);
    TMR0L = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256));//TMR0L = (uint8_t)(TMR16B_OVF(2e-3, 256));
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

    ConfigInputPin(CONFIGIOxOILLEVEL, PINxOILLEVEL);//ext. pullup
    ConfigInputPin(CONFIGIOxSTARTSIGNAL, PINxSTARTSIGNAL);//ext. pullup
    
    ikb_init();
    disp7s_init();
    pulsonic_init();
    
    //
    unlock.kb = 1;
    unlock.autoMode = 1;
    unlock.visMode = 0;
            
    autoMode_init(AUTOMODE_INIT_RESTART);//RESTART
    disp_owner = DISPOWNER_AUTOMODE;
    
    machState = MACHSTATE_RUNNING;
    
    smain.focus.kb = FOCUS_KB_AUTOMODE;
    
    GIE = 1;
    while(1)
    {
        if (isr_flag.f1ms)//sync para toda la pasada
        {
            isr_flag.f1ms = 0;
            smain.f.f1ms = 1;
        }

        if (smain.f.f1ms)
        {
            if (++c_access_kb == 20)
            {
                c_access_kb = 0;
            
                if (unlock.kb)
                {
                    ikb_job();
                }
            }
            
            //display
            if (++c_access_disp == 3)
            {
                c_access_disp = 0;
                disp7s_job();
            }
        }
        //+--------------------------------------------------------------------
        
        //+--------------------------------------------------------------------
        if (machState == MACHSTATE_RUNNING)//solo para darle mas claridad a la lectura del programa, xq podria manejarse con locks
        {
            //sobre estos 2, igual la maquina puede entrar a modo "Config"
            if (unlock.autoMode)
            {
                codapp = autoMode_job();
                
                if (codapp == 1)
                {
                    smain.focus.kb = FOCUS_KB_VISMODE;
                    disp_owner = DISPOWNER_VISMODE;
                    unlock.visMode = 1;
                }
                else if (codapp == 2)
                {
                    machState = MACHSTATE_CONFIG;
                }
            }
            if (unlock.visMode)//este es otro proceso
            {
                codapp = visMode_job();//despues de un tiempo sale automaticamente
                
                if (codapp == 1)
                {
                    smain.focus.kb = FOCUS_KB_AUTOMODE;
                    disp_owner = DISPOWNER_AUTOMODE;
                    unlock.visMode = 0;
                    
                    autoMode_init(AUTOMODE_INIT_CONTINUE);
                }
                else if (codapp == 2)
                {
                    machState = MACHSTATE_CONFIG;
                }
            }
        }
        else if (machState == MACHSTATE_CONFIG)
        {
            //config()	//para la maquina y espera el start --> aqui va a estar a la espera del start
                        //aqui podria convivir con el error en "paralelo"	
            //1)proceso
            //2)display
            if (disp_owner == DISPOWNER_CONFIGMODE)
            {
            }
        }

        pump_job();
        mpap_sych();
        
        //////////
        smain.f.f1ms = 0;
        //ikb_flush();
    }
}
////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////
void check_startSignal(void)
{
	static int8_t sm0, sm1;
	if (sm0 == 0)
	{
		if ( (machState == MACHSTATE_RUNNING) && (!is_startSignal()) ) //cada error decide apropiarse del display
		{
			error_requestToWriteDisp.f.startSignal = 1;//request write
            //
            machState = MACHSTATE_STALL;//puede ser NOT_RUNNING or IDLE
			sm0++;
		}	
	}
	else if (sm0 == 1) //2 parts: the process and the display
	{
		//1)the process
        if ( is_startSignal() )
		{
            error_requestToWriteDisp.f.startSignal = 0;//kill signal queue
            sm0 = 0x00;
            sm1 = 0x00;
		}
		/*else
		{
		}*/
		
        //2) need to write on display
        if (error_grantedToWriteDisp.f.startSignal == 1)//se concede
        {
            if (sm1 == 0)
            {
//                pulsonic.display7s[MODE_DIG_1] = DISP7S_NUMS[2];
//                pulsonic.display7s[MODE_DIG_0] = DISP7S_NUMS[0];
//                //
//                pulsonic.display7s[QUANT_DIG_2] = DISP7S_CHARS[RAYA];
//                pulsonic.display7s[QUANT_DIG_1] = DISP7S_CHARS[RAYA];
//                pulsonic.display7s[QUANT_DIG_0] = DISP7S_CHARS[RAYA];
                //
                sm1++;
            }
        }
	}
}
////////////////////////////////////////////////////////////////////////////////
void check_oilLevel(void)
{
	static int8_t sm0, sm1;
	if (sm0 == 0)
	{
		if ( !is_oilLevel() )
		{
			error_requestToWriteDisp.f.oilLevel = 1;//request write
            //
            
            machState = MACHSTATE_STALL;
            RELAY_DISABLE();
			sm0++;
		}	
	}
	else if (sm0 == 1) //2 parts: the process and the display
	{
		//1)the process
        if ( is_oilLevel() )
		{
            error_requestToWriteDisp.f.oilLevel = 0;//kill signal queue
            sm0 = 0x00;
            sm1 = 0x00;
		}
		/*else
		{
		}*/
		
        //2) need to write on display
        if (error_grantedToWriteDisp.f.oilLevel == 1)//se concede
        {
            if (sm1 == 0)
            {   //no OIL
//                pulsonic.display7s[MODE_DIG_1] = 0x54;  //0b01010100;//n
//                pulsonic.display7s[MODE_DIG_0] = 0x5C;  //0b01011100;//o
//                //
//                pulsonic.display7s[QUANT_DIG_2] = DISP7S_NUMS[0];//O
//                pulsonic.display7s[QUANT_DIG_1] = 0x30;//I
//                pulsonic.display7s[QUANT_DIG_0] = 0x38;//L
                //
                sm1++;
            }
        }
	}
}


////////////////////////////
void errorHandler_queue(void)
{
    #define ERROR_QUEUE_OWNER_DISP_TIME 2000//2000*1mS
    
    static int8_t i;
    static int8_t sm0;
    static uint16_t c;
	intNumMaxErr_t is_granted;
    
    if (sm0 == 0)
    {
        is_granted = error_requestToWriteDisp.packed & (1<<i);
        if (is_granted)
        {
            error_grantedToWriteDisp.packed = is_granted;//solo 1 tiene el permiso
            c = 0x0;
        }
    }
    else if (sm0 == 1)
    {
        if ( error_requestToWriteDisp.packed & (1<<i) )//aun se mantiene?
        {
            if (smain.f.f1ms)
            {
                if (++c == ERROR_QUEUE_OWNER_DISP_TIME)//2 s
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
            sm0 = 0x00;
        }
    }
    
}
