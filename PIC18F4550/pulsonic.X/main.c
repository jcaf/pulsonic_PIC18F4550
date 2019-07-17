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
#include "autoMode.h"
#include "visMode.h"
#include "configMode.h"
#include "flushMode.h"

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
struct _unlock unlock;

enum _FUNMACH
{
    MACHSTATE_STALL = 0,
    FUNCMACH_NORMAL,
    FUNCMACH_CONFIG
};
int8_t funcMach;

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

void errorHandler_queue(void);

void check_startSignal(void);
void check_oilLevel(void);
////////////////////////////////////////////////////////////////////////
void ps_autoMode_start(void)
{
    unlock.autoMode = 1;
    unlock.visMode = 0;
    autoMode_init(AUTOMODE_INIT_RESTART);
    smain.focus.kb = FOCUS_KB_AUTOMODE;
    disp_owner = DISPOWNER_AUTOMODE;
    funcMach = FUNCMACH_NORMAL;
    //
    ikb_flush();
}
void main(void) 
{
    int8_t c_access_kb=0;
    int8_t c_access_disp=0;
    //int8_t codapp;
    codapp_t cod;

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
    
    unlock.kb = 1;
    
    unlock.visMode = 0;

    //
    unlock.autoMode = 1;
    autoMode_init(AUTOMODE_INIT_RESTART);
    smain.focus.kb = FOCUS_KB_AUTOMODE;
    disp_owner = DISPOWNER_AUTOMODE;

    funcMach = FUNCMACH_NORMAL;
    //machState = MACHSTATE_STALL;
    
    ps_autoMode.unlock.kb = 1;
    ps_autoMode.unlock.disp = 1;
    ps_autoMode.unlock.ps = 1;
    
    
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
            if (++c_access_kb == KB_PERIODIC_ACCESS)
            {
                c_access_kb = 0;
                if (unlock.kb)
                    {ikb_job();}
            }
            if (++c_access_disp == 3)
            {
                c_access_disp = 0;
                disp7s_job();
            }
        }
        
        //+--------------------------
        //check_startSignal();
        //check_oilLevel();
        //errorHandler_queue();
        //+--------------------------
        if (funcMach == FUNCMACH_NORMAL)
        {
            if (1)//start
            {
                cod = autoMode_job();
                if (cod.ret == 1)
                {
                   //ps_autoMode.unlock.kb = 0;
                   smain.focus.kb = FOCUS_KB_VISMODE;
                   disp_owner = DISPOWNER_VISMODE;
                   
                   if (cod.param0 == '+')
                        {visMode_init(0x00);}
                   else if (cod.param0 == '-')
                        {visMode_init(VISMODE_NUMMAX_VISTAS-1);}
                   
                   //
                   ps_visMode.unlock.ps = 1;
                   
                }
                else if (cod.ret == 2)
                {
                    mpap.mode = MPAP_STALL_MODE;
                    
                    funcMach = FUNCMACH_CONFIG;
                    //
                    smain.focus.kb = FOCUS_KB_CONFIGMODE;
                    disp_owner = DISPOWNER_CONFIGMODE;
                    unlock.visMode = 0;
                    unlock.autoMode = 0;
                    configMode_init(0x0);
                    RELAY_DISABLE();
                    
                }
                //---------------------
                cod = visMode_job();
                if (cod.ret == 1)
                {
                    smain.focus.kb = FOCUS_KB_AUTOMODE;
                    disp_owner = DISPOWNER_AUTOMODE;
                    unlock.visMode = 0;
                    
                    autoMode_init(AUTOMODE_INIT_CONTINUE);
                    
                    ps_visMode.unlock.ps = 0;
                }
                else if (cod.ret == 2)
                {
                    
                }
                //
                
            }
            else
            {
                
            }
        }
        else if (funcMach == FUNCMACH_CONFIG)
        {
            
        }
        flushMode_job();
        
        //////////
        //////////
        pump_job();
        mpap_sych();
        //////////
        smain.f.f1ms = 0;
        //ikb_flush();//->c/ps es responsable de limpiar su buffer de teclado
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
    static struct _lock_startSignal
    {
        unsigned startIs:1;
        unsigned startIsnt:1;
        unsigned __a:6;
    }lock={0};
	
    if (is_startSignal())
    {
        if (!lock.startIs)
        {
            lock.startIs = 1;
            lock.startIsnt = 0;
            //
            ps_autoMode_start(); 
        }
        
    }
    else
    {
        if (!lock.startIsnt)
        {
            lock.startIsnt = 1;
            lock.startIs = 0;
            //
            mpap.mode = MPAP_STALL_MODE;
            disp7s_qtyDisp_writeText_20_3RAYAS();
            
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
/*
void error_man(void)
{
    if (error !=last)
    {
        error = last;   
    
        if (error =! 0)
        {
            //pone el sistema en modo STALL
        }
        else
        {
            //pone el sistema en modo RUNNING
        }
    }
    //
}
 * 
 * check check_startSignal
if (existe_start)
{
	if (error()== cambios)
	{
		if (error == 0)
		{
			//ahora 
			//salio de errores totalmente-->> ahora va a
		}
	}	
}

  */
void check_inductiveSensorRPM(void)
{
	static int8_t sm0, sm1;
	if (sm0 == 0)
	{
		if ( !is_inductiveSensorRPM() )
		{
			error_requestToWriteDisp.f.inductiveSensorRPM = 1;//request write
            //
            funcMach = MACHSTATE_STALL;
            RELAY_DISABLE();
			sm0++;
		}	
	}
	else if (sm0 == 1) //2 parts: the process and the display
	{
		//1)the process
        if ( is_inductiveSensorRPM() )
		{
            error_requestToWriteDisp.f.inductiveSensorRPM = 0;//kill signal queue
            sm0 = 0x00;
            sm1 = 0x00;
            //
            RELAY_ENABLE();
		}
		/*else{}*/
		
        //2) need to write on display
        if (error_grantedToWriteDisp.f.inductiveSensorRPM == 1)
        {
            if (sm1 == 0)
            {
                //disp7s_qtyDisp_writeText_NO_OIL();
                sm1++;
            }
        }
        else
        {
            sm1 = 0x0;
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
            funcMach = MACHSTATE_STALL;
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
            //
            ps_autoMode_start();
            RELAY_ENABLE();
		}
		/*else{}*/
		
        //2) need to write on display
        if (error_grantedToWriteDisp.f.oilLevel == 1)
        {
            if (sm1 == 0)
            {
                disp7s_qtyDisp_writeText_NO_OIL();
                
                sm1++;
            }
        }
        else
        {
            sm1 = 0x0;
        }
	}
}

void errorHandler_queue(void)
{
    #define ERROR_QUEUE_OWNER_DISP_TIME 2000//2000*1mS
    
    static int8_t i =-1;
    static int8_t sm0;
    static uint16_t c;
	intNumMaxErr_t is_granted;
    
    if (sm0 == 0)
    {
        if (++i == NUMMAX_ERRORS)
            {i = 0x00;}
        //
        is_granted = error_requestToWriteDisp.packed & (1<<i);
        if (is_granted)
        {
            error_grantedToWriteDisp.packed = is_granted;//solo 1 tiene el permiso
            c = 0x0;
            sm0++;
        }
        //
    }
    else if (sm0 == 1)
    {
        if ( error_requestToWriteDisp.packed & (1<<i) )//aun se mantiene?
        {
            if (smain.f.f1ms)
            {
                if (++c >= ERROR_QUEUE_OWNER_DISP_TIME)//2 s
                {
                    c = 0;
                    sm0 = 0;
                }
            }
        }
        else//kill 
        {
            sm0 = 0x00;
        }
    }
}
