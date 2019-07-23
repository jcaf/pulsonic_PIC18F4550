/*
 * File:   main.c
 * Author: jcaf
 *
 * Created on July 19, 2019, 12:01 PM
 */
#include <xc.h>
#include <stdint.h>
#include "nozzle.h"
#include "PIC/eeprom/eeprom.h"

void main(void) 
{
    uint16_t i;
    for (i=0; i<NOZZLE_NUMMAX; i++)
    {
        eepromWrite_double(i, 12.0);//max val
    }
    //next address is for Viscosity
    eepromWrite(i,22);//default viscosity
    
    while(1)
        ;
}
