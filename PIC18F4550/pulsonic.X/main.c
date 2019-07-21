/*
 * File:   main.c
 * Author: jcaf
 *
 * Created on June 29, 2019, 12:51 PM
 * O.S Antergos Linux
 * MPLAB X IDE v4.05
 * Microchip MPLAB XC8 C Compiler V2.05
 * * Microchip MPLAB XC8 C Compiler V1.33 [PRO]+ Speed Option 
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
#include "flushAllMode.h"
#include "error.h"
#include "myeeprom.h"
#include "PIC/eeprom/eeprom.h"
#include "flushAtNozzle.h"

#pragma config "PLLDIV=5", "CPUDIV=OSC1_PLL2", "USBDIV=2", "FOSC=HSPLL_HS", "FCMEN=OFF", "IESO=OFF", "PWRT=ON", , "BORV=3", "VREGEN=ON", "WDT=OFF", "PBADEN=OFF", "LVP=OFF"
#pragma config "MCLRE=ON","BOR=OFF"

volatile struct _isr_flag
{
    unsigned f1ms: 1;
    unsigned __a: 7;
} isr_flag = {0};
volatile struct _smain smain;

int8_t funcMach;

int8_t disp_owner;
int8_t disp_owner_last = -1;
                    
enum _LOCK_STATE
{
    UNLOCKED = 0,
    LOCKED
};
int8_t autoMode_lock = UNLOCKED;
int8_t checkNewStart =1;
int8_t checkNoStart;
int8_t autoMode_toreturn_disp7s=0;

void main(void) 
{
    int8_t c_access_kb=0;
    int8_t c_access_disp=0;
    int8_t START_SIG=0;
    int8_t flushKb;
    static int8_t flushKb_last;
    
    //myeeprom_init();
    
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
    RELAY_ENABLE();
    ConfigOutputPin(CONFIGIOxRELAY, PINxRELAY);
    
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
    startSignal_init();
    oilLevel_init();
    //
    disp_owner = DISPOWNER_AUTOMODE;
    disp_owner_last = -1;
    autoMode_lock = UNLOCKED;
    checkNewStart = 1;
    checkNoStart =1;
    funcMach = FUNCMACH_NORMAL;
    //
    mpap.mode = MPAP_STALL_MODE;
    //
    GIE = 1;
    while(1)
    {
        if (isr_flag.f1ms)
        {
            isr_flag.f1ms = 0;
            smain.f.f1ms = 1;
        }
        if (smain.f.f1ms)
        {
            if (++c_access_kb == KB_PERIODIC_ACCESS)
            {
                c_access_kb = 0;
                ikb_job();
            }
            if (++c_access_disp == 3)
            {
                c_access_disp = 0;
                disp7s_job();
            }
        }
        error_job();
        
        START_SIG = is_startSignal();
        
        if (funcMach == FUNCMACH_NORMAL)
        {
            if (autoMode_lock == UNLOCKED)
            {
                if (START_SIG == 1)
                {
                    if (checkNewStart)
                    {
                        checkNewStart = 0;
                        checkNoStart =1;
                        //
                        autoMode_cmd(JOB_RESTART);
                    }
                }
                else if (START_SIG == 0)
                {
                    if (checkNoStart)
                    {
                        checkNoStart = 0;
                        checkNewStart = 1;
                        //
                        autoMode_cmd(JOB_STOP);
                    }
                }
                //
                /*
                if (disp_owner_last != disp_owner)
                {
                    disp_owner_last = disp_owner;
                    
                    if (disp_owner_last == DISPOWNER_AUTOMODE)
                    {
                        if (START_SIG == 1 )
                            autoMode_disp7s_writeSumTotal();
                        else if (START_SIG == 0)
                            disp7s_qtyDisp_writeText_20_3RAYAS();
                    }
                }
                */
                //esto es automatico
                if (disp_owner == DISPOWNER_AUTOMODE)
                {
                    //ha habido un cambio y debe ser atendido
                    if (START_SIG == 1 )
                        autoMode_disp7s_writeSumTotal();
                    else if (START_SIG == 0)
                        disp7s_qtyDisp_writeText_20_3RAYAS();
                }
                
                //bajo demanda
                if (autoMode_toreturn_disp7s)
                {
                    autoMode_toreturn_disp7s = 0;

                    disp_owner = DISPOWNER_AUTOMODE;

                    if (START_SIG == 1 )
                            autoMode_disp7s_writeSumTotal();
                        else if (START_SIG == 0)
                            disp7s_qtyDisp_writeText_20_3RAYAS();
                }
                //
                autoMode_job();
            }

            /* keyboard*/
            flushKb = ikb_key_is_ready2read(KB_LYOUT_KEY_ENTER_F);
            if (flushKb_last != flushKb)
            {
                if (flushKb)
                {
                    autoMode_lock = LOCKED;
                    flushAllMode_cmd(JOB_RESTART);
                }
                else
                {
                    flushAllMode_cmd(JOB_STOP);

                    /* disp_owner hold the current*/
                    
                    autoMode_lock = UNLOCKED;
                    checkNewStart = 1;
                    
                    if (disp_owner == DISPOWNER_AUTOMODE)
                    {   //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                        //XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                    }
                    
                    if (disp_owner == DISPOWNER_VISMODE)       
                        {visMode.disp7s_accessReq = 1;}
                }
                flushKb_last = flushKb;
            }
            //
            if (ikb_key_is_ready2read(KB_LYOUT_KEY_UP))
            {
                //ikb_key_was_read(KB_LYOUT_KEY_UP);
                //
                if (++visMode.numVista >= VISMODE_NUMMAX_VISTAS)
                {
                    visMode.numVista = -1;
                    autoMode_toreturn_disp7s = 1;
                }
                else
                {
                    disp_owner = DISPOWNER_VISMODE;
                    visMode.disp7s_accessReq = 1;
                }
            }
            else if (ikb_key_is_ready2read(KB_LYOUT_KEY_DOWN))
            {
                //ikb_key_was_read(KB_LYOUT_KEY_DOWN);
                //
                if (visMode.numVista == -1)
                    {visMode.numVista = VISMODE_NUMMAX_VISTAS;}

                if (--visMode.numVista < 0)
                {
                    autoMode_toreturn_disp7s = 1;
                }
                else
                {
                    disp_owner = DISPOWNER_VISMODE;
                    visMode.disp7s_accessReq = 1;
                }
            }
            ikb_key_was_read(KB_LYOUT_KEY_UP);
            ikb_key_was_read(KB_LYOUT_KEY_DOWN);
            //
            
            if ((ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_PLUS)==KB_AFTER_THR) &&
                ikb_key_is_ready2read(KB_LYOUT_KEY_PLUS) &&
                (ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_MINUS)==KB_AFTER_THR) &&
                ikb_key_is_ready2read(KB_LYOUT_KEY_MINUS))
            {
                ikb_key_was_read(KB_LYOUT_KEY_PLUS);
                ikb_key_was_read(KB_LYOUT_KEY_MINUS);
                //
                mpap.mode = MPAP_STALL_MODE;
                
                funcMach = FUNCMACH_CONFIG;
                disp_owner = DISPOWNER_CONFIGMODE;
                configMode_init(0x0);
                RELAY_DISABLE();
                //
                key[4].bf.OnKeyPressed = 1;
                key[4].bf.whilePressing = 0;
                //
            }
            visMode_job();
        }
        else if (funcMach == FUNCMACH_CONFIG)
        {
            if (configMode_job())
            {
                //
                funcMach = FUNCMACH_NORMAL;
                autoMode_lock = UNLOCKED;
                checkNewStart = 1;
                autoMode_toreturn_disp7s = 1;
                //
                RELAY_ENABLE();
            }
            flushAtNozzle_job();
        }
        else if (funcMach == FUNCMACH_ERROR)
        {
            ikb_flush();
        }
        
        //
        flushAllMode_job();
        pump_job();
        
        smain.f.f1ms = 0;
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


static union _errorFlag error_grantedToWriteDisp;//aqui el tiene el permiso en la misma ubicacion
static void errorHandler_queue(void);
static void check_oilLevel(void);

void error_job(void)
{
    static intNumMaxErr_t errorPacked_last = -1;
    
    check_oilLevel();
    //check_inductiveSensorRPM();
    //check_unblocked_nozzle();
    errorHandler_queue();
    
    if (errorPacked_last != pulsonic.error.packed)
    {
        errorPacked_last = pulsonic.error.packed;   
    
        if (errorPacked_last != 0)
        {
            mpap.mode = MPAP_STALL_MODE;
            pump_stop();
            flushAllMode_cmd(JOB_STOP);
            funcMach = FUNCMACH_ERROR;
            RELAY_DISABLE();
        }
        else
        {
            funcMach = FUNCMACH_NORMAL;
            autoMode_lock = UNLOCKED;
            checkNewStart = 1;
            autoMode_toreturn_disp7s = 1;
            pulsonic.flags.homed = 0;
            //
            RELAY_ENABLE();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
static void check_oilLevel(void)
{
	static int8_t sm0, sm1;
    
    int8_t oilLevel;
    
    oilLevel= is_oilLevel();
    
	if (sm0 == 0)
	{
		if ( !oilLevel )
		{
			pulsonic.error.f.oilLevel = 1;                                               //request write
			sm0++;
            sm1 = 0x00;
		}	
	}
	else if (sm0 == 1)                                                          //2 parts: the process and the display
	{
        if ( oilLevel )                                                         //1)the process
		{
            pulsonic.error.f.oilLevel = 0; 
            sm0 = 0x00;
		}
        else
        {
            if (error_grantedToWriteDisp.f.oilLevel == 1)                           //2) need to write on display
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
}
static void errorHandler_queue(void)
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
        is_granted = pulsonic.error.packed & (1<<i);
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
        if ( pulsonic.error.packed & (1<<i) )//aun se mantiene?
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
