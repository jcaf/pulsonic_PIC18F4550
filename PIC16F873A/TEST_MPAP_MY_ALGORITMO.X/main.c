/* PIC16F873A - PROBANDO MI ALGORITMO EN ELMOTOR PAPA BIPOLAR
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

#pragma config "WDTE = OFF", "DEBUG = OFF","BOREN = OFF", "CPD = OFF", "WRT = OFF", "FOSC = XT", "CP = OFF", "LVP = OFF", "PWRTE = ON"

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "system.h"
#include "types.h"

volatile struct _isr_flag
{
    unsigned f1ms: 1;
    unsigned __a: 7;
} isr_flag = {0};
volatile struct _main_flag main_flag;

#define MPAP_DELAY_BY_STEPS 2E-3 //ms
#define TMR0_VALUE (uint8_t)(TMR8B_OVF(MPAP_DELAY_BY_STEPS, 256))
void main(void) 
{
    ADCON1 = 0x06;

    TRISA = 0;
    PORTA = 0;
    
//    ConfigOutputPin(CONFIGIOxSTEPPER_A, PINxSTEPPER_A);
//    ConfigOutputPin(CONFIGIOxSTEPPER_B, PINxSTEPPER_B);
//    ConfigOutputPin(CONFIGIOxSTEPPER_C, PINxSTEPPER_C);
//    ConfigOutputPin(CONFIGIOxSTEPPER_D, PINxSTEPPER_D);

    
    TMR0 = TMR0_VALUE;
    OPTION_REG = 0B00000111; 
    
    
    ConfigOutputPin(CONFIGIOxSTEPPER_A, PINxSTEPPER_A);
    ConfigOutputPin(CONFIGIOxSTEPPER_B, PINxSTEPPER_B);
    ConfigOutputPin(CONFIGIOxSTEPPER_C, PINxSTEPPER_C);
    ConfigOutputPin(CONFIGIOxSTEPPER_D, PINxSTEPPER_D);
    
    mpap_setupToTurn( (1*MPAP_NUMSTEP_1NOZZLE));
    mpap.mode = NORMAL_MODE;

    #define ddd 2

    int i;    
    INTCON = 0B10100000;        //TMR0IE = 1 AND GIE=1


/*
while (1)    
{
    for (i=0; i<50; i++)
    {
        PORTA = 0B00000101 <<2;
        __delay_ms(ddd);
        PORTA = 0B00000110 <<2;
        __delay_ms(ddd);
        PORTA = 0B00001010 <<2;
        __delay_ms(ddd);
        PORTA = 0B00001001 <<2;
        __delay_ms(ddd);    
    }
    __delay_ms(1000);
    PORTA = 0;
    __delay_ms(1000);
    
    for (i=0; i<50; i++)
    {
        PORTA = 0B00001010 <<2;
        __delay_ms(ddd);
        PORTA = 0B00000110 <<2;
        __delay_ms(ddd);
        PORTA = 0B00000101 <<2;
        __delay_ms(ddd);
        PORTA = 0B00001001 <<2;
        __delay_ms(ddd);    
    }
    __delay_ms(1000);
    PORTA=0;
    __delay_ms(1000);
}
*/
    while (1)
    {
        mpap_test();
    }
}
/*******************************************************************************
 1ms 
 * 4Mhz
 * -->T=256- ( (1E-3*4e6)/ (4*4) )
 T  =     6.  
  ******************************************************************************/
void interrupt INTERRUPCION(void)//@1ms 
{
    if (TMR0IF)
    {
        mpap_job();
        
        TMR0IF = 0;
        TMR0 = TMR0_VALUE;
        
    }
}
