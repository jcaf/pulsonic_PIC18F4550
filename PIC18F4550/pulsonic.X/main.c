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
            
                ikb_job();
                //
                if (ikb_key_is_ready2read(0))
                {
                    nozzle_moveto(0);
                }
                if (ikb_key_is_ready2read(1))
                {
                    nozzle_moveto(1);
                }
                if (ikb_key_is_ready2read(2))
                {
                    nozzle_moveto(2);
                }
                if (ikb_key_is_ready2read(3))
                {
                    nozzle_moveto(3);
                }
                if (ikb_key_is_ready2read(4))
                {
                    //nozzle_moveto(4);
//                    while (pump_tick())
//                        ;
                }
            }
            
            if (++c_access_disp == 2)
            {
                c_access_disp = 0;
                display7s_job();
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