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




//PUMP MOTOR
#define PORTWxPUMP LATE
#define PORTRxPUMP PORTE
#define CONFIGIOxPUMP TRISE
#define PINxPUMP  0

#define PUMP_DISABLE()  do{PinTo1(PORTWxPUMP, PINxPUMP);}while(0)
#define PUMP_ENABLE()   do{PinTo0(PORTWxPUMP, PINxPUMP);}while(0)
   

extern struct _multiplexedDisp multiplexedDisp[DISP_TOTAL_NUMMAX];


void mpap_sych(void);


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
#define MPAP_DELAY_BY_STEPS 1.0E-3 //ms
    
    T0CON = 0B10000111; //16BITS
//    TMR0H = (uint8_t)(TMR16B_OVF(2e-3, 256) >> 8);
//    TMR0L = (uint8_t)(TMR16B_OVF(2e-3, 256));
    TMR0H = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256) >> 8);
    TMR0L = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256));
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
    
    disp_show_quantity(77.7);
    //
    
    int8_t c=0;
    int8_t c_disp=0;

    mpap_setupToTurn( (4*MPAP_NUMSTEP_1NOZZLE));
    mpap.mode = NORMAL_MODE;
GIE = 1;
//mpap_1();
    
    while (1)
    {
        mpap_test();
//        mpap_job();
//        __delay_ms(2);
    }
    
    mpap_setupToHomming();
    
    while(1)
    {
        if (isr_flag.f1ms)//sync para toda la pasada
        {
            isr_flag.f1ms = 0;
            main_flag.f1ms = 1;
        }

        //keyboard
        if (main_flag.f1ms)
        {
            if (++c==5)
            {
                c = 0;
            
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
                    nozzle_moveto(4);
                }
            }
            //displays
            //if (++c_disp == 2)
            //{
            //    c_disp = 0;
                display7s_job();
            //}
            
        }
        mpap_sych();
        //////////
        main_flag.f1ms = 0;
        ikb_flush();
    }
}

void mpap_sych(void)
{
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
        }
    }
}
    
void interrupt INTERRUPCION(void)//@1ms 
{
    static uint8_t cm = 0;
    if (TMR0IF)
    {
        isr_flag.f1ms = 1;
        
        //if (++cm == 2)
        //{
            cm = 0;
            mpap_job();
        //}
        TMR0IF = 0;
        TMR0H = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256) >> 8);
        TMR0L = (uint8_t)(TMR16B_OVF(MPAP_DELAY_BY_STEPS, 256));
    }
}