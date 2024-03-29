/*
 * File:   main.c
 * Author: jcaf
 *
 * Created on June 29, 2019, 12:51 PM
 * O.S Antergos Linux
 * MPLAB X IDE v4.05
 * Microchip MPLAB XC8 C Compiler V2.05
 * * Microchip MPLAB XC8 C Compiler V1.33 [PRO]+ Speed Option 
 * PIC18F4550 @ 48MHz Code protected + EEprom extern read/write protected
 * 
 * Project: Pulsonic (aceitera)
 * 
 * OBSERVACIONES: 
 * 1) XC8 itoa(buff, int, base) no es "standard" en el orden de los argumentos
 *      en main.h: 
 * #define myitoa(_integer_, _buffer_, _base_) itoa(_buffer_, _integer_, _base_) 
 * 
 * 2) Pickit2: Program with preserve device EEPROM after flash with myeeprom_init()
 
 EXTRAS:
 *1) 20 + Suma total
 *2) Flush enable
 *3) Modo automatico en 1 sola pasada, luego espera por el tiempo, e inicia un nuevo ciclo
  
 Para probar en la aceitera donde el mosfet recalienta, hacer
 * mpap.c
 * linea 238
         if (1)//(counterZeros >= (MPAP_NUMSTEP_1NOZZLE*0.1))
 * 2) para probar el tiempo de salida en inactividad en modo visualizacion..
 * main.c
        linea 345 //vismode.h     #define VISMODE_KTIME_INACTIVITY_EXIT_MS (30*1000)//30s

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
#include "ustepping.h"

#pragma config "PLLDIV=5", "CPUDIV=OSC1_PLL2", "USBDIV=2", "FOSC=HSPLL_HS", "FCMEN=OFF", "IESO=OFF", "PWRT=ON", , "BORV=3", "VREGEN=ON", "WDT=OFF", "PBADEN=OFF", "LVP=OFF"
#pragma config "MCLRE=ON","BOR=OFF", 
#pragma config "CP0=ON", "CP1=ON", "CP2=ON", "CP3=ON", "CPD=OFF"

volatile static struct _isr_flag
{
    unsigned f1ms : 1;
    unsigned __a : 7;
} isr_flag = {0};

volatile struct _smain smain;

enum _LOCK_STATE
{
    UNLOCKED = 0,
    LOCKED
};

static struct _psFlag
{
    unsigned checkNewStart : 1;
    unsigned checkNoStart : 1;
    unsigned autoMode_lock : 1;
    unsigned autoMode_toreturn_disp7s : 1;
    unsigned __a : 4;
} psFlag;
int8_t funcMach;
int8_t disp_owner;

void mykb_layout0(void)
{
    struct _key_prop prop = {0};

    prop = propEmpty;
    prop.uFlag.f.onKeyPressed = 1;
    prop.uFlag.f.reptt = 1;
    prop.numGroup = 1;
    prop.repttTh.breakTime = (uint16_t) 500.0 / KB_PERIODIC_ACCESS;
    prop.repttTh.period = (uint16_t) 300.0 / KB_PERIODIC_ACCESS;

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

/* only for test MOSFET + MOTOR TEMPERATURE*/
void loop_test_motor_mosfet(void)
{
    autoMode_cmd(JOB_RESTART);
    while (1)
    {
        if (isr_flag.f1ms)
        {
            isr_flag.f1ms = 0;
            smain.f.f1ms = 1;
        }
        //
        autoMode_job_temperature();
        pump_job();
        smain.f.f1ms = 0;
    }
}
static void check_oilLevel_reset(void);
static void check_homeSensor_reset(void);

struct _pulsonic_display disp7s_bk;

int8_t startSig_last = 0;

void main(void)
{
    int8_t sm0 = 0;
    int8_t c_access_kb = 0;
    int8_t c_access_disp = 0;
    int8_t startSig = 0;
    //int8_t startSig_last = 0;
    int8_t flushKb;
    static int8_t flushKb_last;
    struct _key_prop prop = {0};

    //myeeprom_init();/*uncomment and Pickit2 preserve device EEPROM disabled*/

    LATA = 0x00;
    LATC = 0x00;
    LATD = 0x00;
    LATE = 0x00;
    LATB = 0x00;

    ADCON1 = 0x0F; //All analog inputs as DIGITAL
    CMCON = 0xCF; //POR default mode comparators OFF

    //RC4/RC5 config as digital inputs
    UCON = 0; //USBEN Disable
    UCFG = 1 << 3; //UTRDIS Digital input enable RC4/RC5
    //
    //    T0CON = 0B10000100; //16BITS
    //    TMR0H = (uint8_t)(TMR16B_OVF(1e-3, 32) >> 8);
    //    TMR0L = (uint8_t)(TMR16B_OVF(1e-3, 32));
    //    TMR0IE = 1;
    //.....

    RELAY_DISABLE();
    ConfigOutputPin(CONFIGIOxRELAY, PINxRELAY);

    PUMP_DISABLE();
    ConfigOutputPin(CONFIGIOxPUMP, PINxPUMP);

    ConfigOutputPin(CONFIGIOxSTEPPER_A, PINxSTEPPER_A);
    ConfigOutputPin(CONFIGIOxSTEPPER_B, PINxSTEPPER_B);
    ConfigOutputPin(CONFIGIOxSTEPPER_C, PINxSTEPPER_C);
    ConfigOutputPin(CONFIGIOxSTEPPER_D, PINxSTEPPER_D);

    //+-
    PR2 = PR2_VAL; //255;
    setdc(DC_TOP - DC_MIN);
    STEPPER_ENABLE(); //output pin
    T2CON = 0b00000101; //T2CKPS1:T2CKPS0: Timer2 Clock Prescale Select bits
    //00 = Prescaler is 1    //01 = Prescaler is 4    //1x = Prescaler is 16
    CCP2CON = 0B00001100; //PWM
    //+-

    ConfigInputPin(CONFIGIOxSTEPPER_SENSOR_HOME, PINxSTEPPER_SENSOR_HOME);
    ConfigInputPin(CONFIGIOxOILLEVEL, PINxOILLEVEL); //ext. pullup
    ConfigInputPin(CONFIGIOxSTARTSIGNAL, PINxSTARTSIGNAL); //ext. pullup

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

    // loop_test_motor_mosfet();//for debug temperature of mosfet + motor
    while (1)
    {
        if (isr_flag.f1ms)
        {
            isr_flag.f1ms = 0;
            smain.f.f1ms = 1;
        }
        //
        if (sm0 == 0)
        {
            //autoMode_jobX();
            //pump_job();
            if (mpap_homming_job())
            {
                sm0++;
            }
        }
        else
        {
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

            error_job(); //change the flow 

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
                            psFlag.checkNoStart = 1;
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

                    /* Display: 2 cases     */
                    /* 1st case: in auto mode without change disp_owner = DISPOWNER_AUTOMODE */
                    if (disp_owner == DISPOWNER_AUTOMODE)
                    {
                        if (startSig_last != startSig)
                        {
                            startSig_last = startSig;
                            if (startSig)
                                autoMode_disp7s_writeSumTotal();
                            else
                                disp7s_qtyDisp_writeText_20_3RAYAS();
                        }
                    }

                    /* 2nd case: on demand, from other process */
                    //                    if (psFlag.autoMode_toreturn_disp7s)
                    //                    {
                    //                        psFlag.autoMode_toreturn_disp7s = 0;
                    //
                    //                        disp_owner = DISPOWNER_AUTOMODE;
                    //
                    //                        if (startSig)
                    //                        {
                    //                            autoMode_disp7s_writeSumTotal();
                    //                        }
                    //                        else
                    //                        {
                    //                            disp7s_qtyDisp_writeText_20_3RAYAS();
                    //                        }
                    //
                    //                        startSig_last = startSig;
                    //                    }
                    //
                    autoMode1_job();
                }

                /* keyboard*/
                flushKb = ikb_key_is_ready2read(KB_LYOUT_KEY_FLUSHENTER); //key[k].prop.uFlag.f.whilePressing
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
                        {
                            //psFlag.autoMode_toreturn_disp7s = 1;
                            //
                            disp_owner = DISPOWNER_AUTOMODE;
                            startSig_last = -1;
                            //
                            //kill visualization timer
                            visMode.timerRun = 0;
                            visMode.timer = 0;
                            visMode.lockTimming = 0;
                            //

                        }

                        if (disp_owner == DISPOWNER_VISMODE)
                        {
                            visMode.disp7s_accessReq = 1;
                        }
                    }
                    flushKb_last = flushKb;
                }
                
                ///////////////////////////////////////////////////////////////////
                //Added
                if (visMode.timerRun == 1)
                {
                    if (smain.f.f1ms)
                    {
                        if (++visMode.timer >= VISMODE_KTIME_INACTIVITY_EXIT_MS)
                        {
                            //kill visualization timer
                            visMode.timerRun = 0;
                            visMode.timer = 0;
                            visMode.lockTimming = 0;
                            //
                            visMode.numVista = -1;//x key_up
                            
                            //
                            disp_owner = DISPOWNER_AUTOMODE;
                            startSig_last = -1;
                            //
                        }
                    }
                }
                if (ikb_key_is_ready2read(KB_LYOUT_KEY_UP))
                {
                    //ikb_key_was_read(KB_LYOUT_KEY_UP);
                    //
                    if (++visMode.numVista >= VISMODE_NUMMAX_VISTAS)
                    {
                        visMode.numVista = -1;
                        //psFlag.autoMode_toreturn_disp7s = 1;
                        //
                        disp_owner = DISPOWNER_AUTOMODE;
                        startSig_last = -1;
                        //
                        //Added: kill timer
                        visMode.timerRun = 0;
                        visMode.timer = 0;
                        visMode.lockTimming = 0;
                        //    
                    }
                    else
                    {
                        disp_owner = DISPOWNER_VISMODE;
                        visMode.disp7s_accessReq = 1;
                        
                        
                        //Added
                        if (!visMode.lockTimming)
                        {
                            visMode.timerRun = 1;
                            visMode.lockTimming = 1;
                        }
                        
                        visMode.timer = 0x00;//reset the timer
                    }
                }
                else if (ikb_key_is_ready2read(KB_LYOUT_KEY_DOWN))
                {
                    //ikb_key_was_read(KB_LYOUT_KEY_DOWN);
                    //
                    if (visMode.numVista == -1)
                    {
                        visMode.numVista = VISMODE_NUMMAX_VISTAS;
                    }

                    if (--visMode.numVista < 0)
                    {
                        //psFlag.autoMode_toreturn_disp7s = 1;
                        //
                        disp_owner = DISPOWNER_AUTOMODE;
                        startSig_last = -1;
                        //
                        //Added: kill timer
                        visMode.timerRun = 0;
                        visMode.timer = 0;
                        visMode.lockTimming = 0;
                        //    
                    }
                    else
                    {
                        disp_owner = DISPOWNER_VISMODE;
                        visMode.disp7s_accessReq = 1;
                        
                        //Added
                        if (!visMode.lockTimming)
                        {
                            visMode.timerRun = 1;
                            visMode.lockTimming = 1;
                        }
                        
                        visMode.timer = 0x00;//reset the timer
                    }
                }
                ikb_key_was_read(KB_LYOUT_KEY_UP);
                ikb_key_was_read(KB_LYOUT_KEY_DOWN);
                //

                //+-bug fixed:
                if ((ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_PLUS) == KB_BEFORE_THR) &&
                        ikb_key_is_ready2read(KB_LYOUT_KEY_PLUS))
                {
                    ikb_key_was_read(KB_LYOUT_KEY_PLUS);
                }
                if ((ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_MINUS) == KB_BEFORE_THR) &&
                        ikb_key_is_ready2read(KB_LYOUT_KEY_MINUS))
                {
                    ikb_key_was_read(KB_LYOUT_KEY_MINUS);
                }
                //-+

                if ((ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_PLUS) == KB_AFTER_THR) &&
                        ikb_key_is_ready2read(KB_LYOUT_KEY_PLUS) &&
                        (ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_MINUS) == KB_AFTER_THR) &&
                        ikb_key_is_ready2read(KB_LYOUT_KEY_MINUS))
                {
                    ikb_key_was_read(KB_LYOUT_KEY_PLUS);
                    ikb_key_was_read(KB_LYOUT_KEY_MINUS);
                    //
                    pump_stop();
                    mpap_stall();

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
                    //psFlag.autoMode_toreturn_disp7s = 1;
                    //
                    disp_owner = DISPOWNER_AUTOMODE;
                    startSig_last = -1;
                    //
                    //kill visualization timer
                    visMode.timerRun = 0;
                    visMode.timer = 0;
                    visMode.lockTimming = 0;
                    //

                    //
                    RELAY_ENABLE();
                }
                flushAtNozzle_job();
            }
            else if (funcMach == FUNCMACH_ERROR)
            {
                /* NOTA: Para borrar un mensaje de error, previamente se comprueba si
                 start esta desenganchado */
                if (!startSig)
                {
                    if (ikb_key_is_ready2read(KB_LYOUT_KEY_FLUSHENTER))
                    {
                        pulsonic.error.packed = 0x00; //clear all error flags
                        //
                        //disp_owner = DISPOWNER_AUTOMODE;
                        //startSig_last = -1;
                        //
                        /* error_job() is responsible for restore everything to its place!*/
                        //kill visualization timer
                        visMode.timerRun = 0;
                        visMode.timer = 0;
                        visMode.lockTimming = 0;
                        //
                    }
                }
                //
                ikb_flush();
            }
            //
            pump_job();

        }
        smain.f.f1ms = 0;
    }
}

/*
 * PWM = 20KHz
 * uStepping: 4 + 32 + 4 = 40 usteps = 40 * 50e-6 = 2 ms
  
 */
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
            if (mpap_getMode() > MPAP_STALL_MODE)
            {
                ustep_sm0++;
            }
        }
        if (ustep_sm0 == 1)
        {
            setdc(ustep_lockup[ustep_c]);
            if (++ustep_c == (MICROSTEP_N + 1))
            {
                ustep_c = 0x00;
                ustep_sm0++;
            }
        }
        else if (ustep_sm0 == 2)
        {
            //if (++ustep_c == 12)
            if (++ustep_c == 32)
            {
                ustep_c = MICROSTEP_N - 1; //ustep_c = 0x3;
                ustep_sm0++;
            }
        }
        else if (ustep_sm0 == 3)
        {
            setdc(ustep_lockup[ustep_c]);

            if (--ustep_c < 0)
            {
                ustep_c = 1;
                ustep_sm0 = 0x0;
                //LATC0 =!LATC0;//aqui deberia tener 1ms: OK Correcto!
            }
        }
        //
        if (++cticks == 20)//50us*20 = 1ms
            //if (++cticks == 40)//25us*40 = 1ms
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

static union _errorFlag error_grantedToWriteDisp; //Always in the same bit position like error
static void errorHandler_queue(void);
static void check_oilLevel(void);
static void check_homeSensor(void);

void error_job(void)
{
    struct _key_prop prop = {0};
    //
    static intNumMaxErr_t errorPacked_last = -1;

    check_oilLevel();
    check_homeSensor();
    //check_inductiveSensorRPM();
    //check_unblocked_nozzle();
    errorHandler_queue();

    if (errorPacked_last != pulsonic.error.packed)
    {
        errorPacked_last = pulsonic.error.packed;

        if (errorPacked_last != 0)
        {
            mpap_stall();
            pump_stop();
            flushAllMode_cmd(JOB_STOP);
            funcMach = FUNCMACH_ERROR;
            RELAY_DISABLE();

            //added:
            disp7s_bk = pulsonic.disp7s; //save a copy from last display
            //ikey_clear_all_flag();
            ikb_flush();
            // in this moment any key configured as whilepressing is cleared from main()
            // clear all last_states... but is better to change only to "normal function"
            prop = propEmpty;
            prop.uFlag.f.onKeyPressed = 1;
            ikb_setKeyProp(KB_LYOUT_KEY_FLUSHENTER, prop);
        }
        else
        {
            funcMach = FUNCMACH_NORMAL;
            psFlag.autoMode_lock = UNLOCKED;
            psFlag.checkNewStart = 1;
            pulsonic.flags.homed = 0;
            //psFlag.autoMode_toreturn_disp7s = 1;
            RELAY_ENABLE();
            //
            disp_owner = DISPOWNER_AUTOMODE;
            startSig_last = -1;
            //
            //Added
            //kill visualization timer
            visMode.timerRun = 0;
            visMode.timer = 0;
            visMode.lockTimming = 0;
            //
            //
            check_oilLevel_reset();
            check_homeSensor_reset();
            mykb_layout0();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

struct _chek_homeSensor_data
{
    int8_t sm0;
    int8_t sm1;
} chek_homeSensor_data;

static void check_homeSensor_reset(void)
{
    chek_homeSensor_data.sm0 = 0;
    chek_homeSensor_data.sm1 = 0;
}

static void check_homeSensor(void)
{
    if (chek_homeSensor_data.sm0 == 0)
    {
        if (pulsonic.error.f.homeSensor == 1)
        {
            //ISR flagged
            //pulsonic.error.f.homeSensor = 1; /*request write*/
            chek_homeSensor_data.sm0++;
            chek_homeSensor_data.sm1 = 0x00;
        }
    }
    else if (chek_homeSensor_data.sm0 == 1)/*2 parts: the process and the display*/
    {
        //cannot recovery from this, User needs to reset the system
        if (0) /*1)the process*/
        {
            //pulsonic.error.f.homeSensor = 0;
            //chek_homeSensor_data.sm0 = 0x00;
        }
        else
        {
            if (error_grantedToWriteDisp.f.homeSensor == 1)/*2) need to write on display*/
            {
                if (chek_homeSensor_data.sm1 == 0)
                {
                    disp7s_qtyDisp_writeText_NO_HOME_SENSOR();
                    chek_homeSensor_data.sm1++;
                }
            }
            else
            {
                chek_homeSensor_data.sm1 = 0x0;
            }
        }
    }
}

struct _check_oilLevel_data
{
    int8_t sm0;
    int8_t sm1;
} check_oilLevel_data;

static void check_oilLevel_reset(void)
{
    check_oilLevel_data.sm0 = 0x00;
    check_oilLevel_data.sm1 = 0x00;
}

static void check_oilLevel(void)
{
    int8_t oilLevel;
    oilLevel = is_oilLevel();

    if (check_oilLevel_data.sm0 == 0)
    {
        if (!oilLevel)
        {
            pulsonic.error.f.oilLevel = 1; /*request write*/
            check_oilLevel_data.sm0++;
            check_oilLevel_data.sm1 = 0x00;
        }
    }
    else if (check_oilLevel_data.sm0 == 1)/*2 parts: the process and the display*/
    {
        if (0)//(oilLevel)   /*1)the process*/
        {
            pulsonic.error.f.oilLevel = 0;
            check_oilLevel_data.sm0 = 0x00;
        }
        else
        {
            if (error_grantedToWriteDisp.f.oilLevel == 1) /*2) need to write on display*/
            {
                if (check_oilLevel_data.sm1 == 0)
                {
                    disp7s_qtyDisp_writeText_NO_OIL();
                    check_oilLevel_data.sm1++;
                }
            }
            else
            {
                check_oilLevel_data.sm1 = 0x0;
            }
        }
    }
}

static void errorHandler_queue(void)
{
#define ERROR_QUEUE_OWNER_DISP_TIME 2000//2000*1mS

    static int8_t i = -1;
    static int8_t sm0;
    static uint16_t c;
    intNumMaxErr_t is_granted;

    if (sm0 == 0)
    {
        if (++i == NUMMAX_ERRORS)
        {
            i = 0x00;
        }
        //
        is_granted = pulsonic.error.packed & (1 << i);
        if (is_granted)
        {
            error_grantedToWriteDisp.packed = is_granted; //Permission was granted
            c = 0x0;
            sm0++;
        }
        //
    }
    else if (sm0 == 1)
    {
        if (pulsonic.error.packed & (1 << i))//this error persist?
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
