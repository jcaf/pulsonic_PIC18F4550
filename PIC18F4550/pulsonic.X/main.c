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
 * 2) Pickit2: Program with preserve device EEPROM
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

volatile static struct _isr_flag
{
    unsigned f1ms: 1;
    unsigned __a: 7;
} isr_flag = {0};

volatile struct _smain smain;

enum _LOCK_STATE
{
    UNLOCKED = 0,
    LOCKED
};

static struct _psFlag
{
    unsigned checkNewStart:1;
    unsigned checkNoStart:1;
    unsigned autoMode_lock:1;
    unsigned autoMode_toreturn_disp7s:1;
    unsigned __a:4;
}psFlag;
int8_t funcMach;
int8_t disp_owner;

void mykb_layout0(void)
{
    struct _key_prop prop = {0};
    
    prop = propEmpty;
    prop.uFlag.f.onKeyPressed = 1;
    prop.uFlag.f.reptt = 1;
    prop.numGroup = 1;
    prop.repttTh.breakTime = (uint16_t)500.0/KB_PERIODIC_ACCESS;
    prop.repttTh.period = (uint16_t)300.0/KB_PERIODIC_ACCESS;
    ikb_setKeyProp(KB_LYOUT_KEY_UP, prop);
    ikb_setKeyProp(KB_LYOUT_KEY_DOWN, prop);
    //
    prop = propEmpty;
    prop.uFlag.f.atTimeExpired2 = 1;
    ikb_setKeyProp(KB_LYOUT_KEY_PLUS, prop);
    ikb_setKeyProp(KB_LYOUT_KEY_MINUS, prop);
    //
    prop = propEmpty;
    prop.uFlag.f.whilePressing = 1;
    ikb_setKeyProp(KB_LYOUT_KEY_FLUSHENTER, prop);
}

void setdc(uint16_t dc)
{
    CCP2CON = (uint8_t)  ( ((dc&0x03)<<4)| (CCP2CON & 0xCF));
    CCPR2L = (uint8_t)(dc>>2);
}

//#define PR2_VAL 255
#define PR2_VAL 149 //20khz-> 50us
#define DC_TOP (PR2_VAL<<2)

#define MICROSTEP_N 4
//#define DC_MIN 0.5*1024f
//#define DC_MAX 0.7*1024f
#define DC_MIN (0.35*DC_TOP)
#define DC_MAX (0.8*DC_TOP)
#define MICROSTEP (DC_MAX - DC_MIN)/MICROSTEP_N


void main(void) 
{
    int8_t c_access_kb=0;
    int8_t c_access_disp=0;
    int8_t startSig=0;
    int8_t startSig_last=0;
    int8_t flushKb;
    static int8_t flushKb_last;
    struct _key_prop prop = {0};
    
    //myeeprom_init();/*once with pickit2 preserve device EEPROM disabled*/
    
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
    //
    //    T0CON = 0B10000100; //16BITS
    //    TMR0H = (uint8_t)(TMR16B_OVF(1e-3, 32) >> 8);
    //    TMR0L = (uint8_t)(TMR16B_OVF(1e-3, 32));
    //    TMR0IE = 1;
    //.....
    RELAY_ENABLE();
    ConfigOutputPin(CONFIGIOxRELAY, PINxRELAY);
    
    PUMP_DISABLE();
    ConfigOutputPin(CONFIGIOxPUMP, PINxPUMP);
    
    ConfigOutputPin(CONFIGIOxSTEPPER_A, PINxSTEPPER_A);
    ConfigOutputPin(CONFIGIOxSTEPPER_B, PINxSTEPPER_B);
    ConfigOutputPin(CONFIGIOxSTEPPER_C, PINxSTEPPER_C);
    ConfigOutputPin(CONFIGIOxSTEPPER_D, PINxSTEPPER_D);
    
    //+-
    PR2 = PR2_VAL;      //255;
    setdc(DC_TOP-DC_MIN);
    STEPPER_ENABLE();   //output pin
    T2CON = 0b00000101;
    //T2CKPS1:T2CKPS0: Timer2 Clock Prescale Select bits
    //00 = Prescaler is 1
    //01 = Prescaler is 4
    //1x = Prescaler is 16
    CCP2CON = 0B00001100;//PWM
    //+-

    ConfigInputPin(CONFIGIOxSTEPPER_SENSOR_HOME, PINxSTEPPER_SENSOR_HOME);
    ConfigInputPin(CONFIGIOxOILLEVEL, PINxOILLEVEL);//ext. pullup
    ConfigInputPin(CONFIGIOxSTARTSIGNAL, PINxSTARTSIGNAL);//ext. pullup
    
    ikb_init();
    mykb_layout0();
    //
    disp7s_init();
    pulsonic_init();
    startSignal_init();
    oilLevel_init();
    //
    psFlag.autoMode_lock = UNLOCKED;
    psFlag.checkNewStart = 1;
    psFlag.checkNoStart = 1;
    disp_owner = DISPOWNER_AUTOMODE;
    funcMach = FUNCMACH_NORMAL;
    autoMode_cmd(JOB_STOP);
    
    startSig = is_startSignal();
    startSig_last = startSig; 
    //
    TMR2IE = 1;
    PEIE = 1;
    GIE = 1;

    //mpap.mode = MPAP_HOMMING_MODE;
    //while(1);
    
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
        
//        if (smi == 0)
//        {
//            
//        }
//        else
//        {
//            
//        }
        
        error_job();//change the program-flow 
        
        startSig = is_startSignal();

        if (funcMach == FUNCMACH_NORMAL)
        {
            if (psFlag.autoMode_lock == UNLOCKED)
            {
                if (startSig)
                {
                    if (psFlag.checkNewStart)
                    {
                        psFlag.checkNewStart = 0;
                        psFlag.checkNoStart =1;
                        //
                        autoMode_cmd(JOB_RESTART);
                    }
                }
                else
                {
                    if (psFlag.checkNoStart)
                    {
                        psFlag.checkNoStart = 0;
                        psFlag.checkNewStart = 1;
                        //
                        autoMode_cmd(JOB_STOP);
                    }
                }
                
                /*Display: 2 cases     */
                /*1st case: in auto mode without change disp_owner = DISPOWNER_AUTOMODE*/
                if (disp_owner == DISPOWNER_AUTOMODE)
                {
                    if (startSig_last != startSig)
                    {
                        startSig_last = startSig;
                        if (startSig)
                            {autoMode_disp7s_writeSumTotal();}
                        else
                            {disp7s_qtyDisp_writeText_20_3RAYAS();}
                    }
                }
                
                /*2nd case: on demand, from other process*/
                if (psFlag.autoMode_toreturn_disp7s)
                {
                    psFlag.autoMode_toreturn_disp7s = 0;

                    disp_owner = DISPOWNER_AUTOMODE;

                    if (startSig)
                        {autoMode_disp7s_writeSumTotal();}
                    else
                        {disp7s_qtyDisp_writeText_20_3RAYAS();}
                
                    startSig_last = startSig;
                }
                //
                autoMode_job();
            }

            /* keyboard*/
            flushKb = ikb_key_is_ready2read(KB_LYOUT_KEY_FLUSHENTER);
            if (flushKb_last != flushKb)
            {
                if (flushKb)
                {
                    psFlag.autoMode_lock = LOCKED;
                    flushAllMode_cmd(JOB_RESTART);
                }
                else
                {
                    flushAllMode_cmd(JOB_STOP);

                    /* disp_owner hold the current*/
                    
                    psFlag.autoMode_lock = UNLOCKED;
                    psFlag.checkNewStart = 1;
                    
                    if (disp_owner == DISPOWNER_AUTOMODE)
                        {psFlag.autoMode_toreturn_disp7s = 1;}
                    
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
                    psFlag.autoMode_toreturn_disp7s = 1;
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
                    psFlag.autoMode_toreturn_disp7s = 1;
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
                pump_stop();
                mpap.mode = MPAP_STALL_MODE;
                
                funcMach = FUNCMACH_CONFIG;
                disp_owner = DISPOWNER_CONFIGMODE;
                configMode_init(0x0);
                RELAY_DISABLE();
                //
                /*change layout for FLush/Enter key*/
                prop = propEmpty;
                prop.uFlag.f.onKeyPressed = 1;
                ikb_setKeyProp(KB_LYOUT_KEY_FLUSHENTER, prop);
            }
            visMode_job();
            flushAllMode_job();
        }
        else if (funcMach == FUNCMACH_CONFIG)
        {
            if (configMode_job())
            {
                //
                funcMach = FUNCMACH_NORMAL;
                psFlag.autoMode_lock = UNLOCKED;
                psFlag.checkNewStart = 1;
                psFlag.autoMode_toreturn_disp7s = 1;
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
        pump_job();
        smain.f.f1ms = 0;
    }
}

/*DC_TOP - (x) is for complement to inverted Enable chip */
const uint16_t ustep_lockup[MICROSTEP_N+1]=
{   DC_TOP -(DC_MIN+(MICROSTEP*0)), //MIN
    DC_TOP -(DC_MIN+(MICROSTEP*1)), 
    DC_TOP -(DC_MIN+(MICROSTEP*2)), 
    DC_TOP -(DC_MIN+(MICROSTEP*3)), 
    DC_TOP -(DC_MIN+(MICROSTEP*4)), //MAX
    //
    //DC_TOP -(DC_MIN+(MICROSTEP*5)), //MAX
    //DC_TOP -(DC_MIN+(MICROSTEP*6)), //MAX
    //DC_TOP -(DC_MIN+(MICROSTEP*7)), //MAX
    //DC_TOP -(DC_MIN+(MICROSTEP*8)), //MAX
};

void interrupt INTERRUPCION(void)
{
    static uint8_t ustep_sm0;
    static int8_t ustep_c = 1;
    
    static int8_t cticks;
    
    if (TMR2IF)
    {
        if (ustep_sm0 == 0)
        {
            mpap_job();
            //mpap_do1step(+1);
            if (mpap.mode > MPAP_STALL_MODE)
            {
                ustep_sm0++;
            }
        }
        if (ustep_sm0 == 1)
        {
            setdc(ustep_lockup[ustep_c]);
            if (++ustep_c == (MICROSTEP_N+1) )
            {
                ustep_c = 0x00;
                ustep_sm0++;
            }
        }
        else if (ustep_sm0 == 2)
        {
            if (++ustep_c == 12)//12
            {
                ustep_c = MICROSTEP_N-1;//ustep_c = 0x3;
                ustep_sm0++;
            }
        }
        else if (ustep_sm0 == 3)
        {
            setdc( ustep_lockup[ustep_c] );

            if (--ustep_c < 0)
            {
                ustep_c = 1;
                ustep_sm0 = 0x0;
                //LATC0 =!LATC0;//aqui deberia tener 1ms: OK Correcto!
            }
        }
        //
        if (++cticks == 20)//50us*20 = 1ms
        {
            isr_flag.f1ms = 1;
            cticks = 0x00;
        }
        //
        TMR2IF = 0;
    }

    //TMMR0:
    //octave:34> (65536) - (1e-3*48e6/(32*4)) 
    //ans =  65161    
    //    if (TMR0IF)
    //    {
    //        isr_flag.f1ms = 1;
    //        TMR0IF = 0;
    //        TMR0H = (uint8_t)(TMR16B_OVF(1e-3, 32) >> 8);
    //        TMR0L = (uint8_t)(TMR16B_OVF(1e-3, 32));
    //    }
    
}

static union _errorFlag error_grantedToWriteDisp;//Always in the same bit position like error
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
            psFlag.autoMode_lock = UNLOCKED;
            psFlag.checkNewStart = 1;
            psFlag.autoMode_toreturn_disp7s = 1;
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
			pulsonic.error.f.oilLevel = 1;/*request write*/
			sm0++;
            sm1 = 0x00;
		}	
	}
    else if (sm0 == 1)/*2 parts: the process and the display*/
	{
        if ( oilLevel )/*1)the process*/
		{
            pulsonic.error.f.oilLevel = 0; 
            sm0 = 0x00;
		}
        else
        {
            if (error_grantedToWriteDisp.f.oilLevel == 1)/*2) need to write on display*/
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
            error_grantedToWriteDisp.packed = is_granted;//Permission was granted
            c = 0x0;
            sm0++;
        }
        //
    }
    else if (sm0 == 1)
    {
        if ( pulsonic.error.packed & (1<<i) )//this error persist?
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
