#ifndef MAIN_H_
#define	MAIN_H_

    #include <xc.h> // include processor files - each processor file is guarded.  
    #include <stdint.h>
    #define F_CPU 48000000UL//48MHz
    #include "system.h"
    #include "types.h"

    struct _main_flag
    {
        unsigned f1ms:1;
        unsigned __a:7;
    };
    extern volatile struct _main_flag main_flag;
    
    #define myitoa(_integer_, _buffer_, _base_) itoa(_buffer_, _integer_, _base_)

    ////////////////////////////////////////////////////////////////////////////
    //OIL-PUMP MOTOR
    #define PORTWxPUMP LATE
    #define PORTRxPUMP PORTE
    #define CONFIGIOxPUMP TRISE
    #define PINxPUMP  0

    #define PUMP_DISABLE()  do{PinTo1(PORTWxPUMP, PINxPUMP);}while(0)
    #define PUMP_ENABLE()   do{PinTo0(PORTWxPUMP, PINxPUMP);}while(0)
    
    ////////////////////////////////////////////////////////////////////////////
    //MOTOR PAP UNIPOLAR
    #define PORTWxSTEPPER_A LATD
    #define PORTRxSTEPPER_A PORTD
    #define CONFIGIOxSTEPPER_A TRISD
    #define PINxSTEPPER_A  0
    //
    #define PORTWxSTEPPER_B LATD
    #define PORTRxSTEPPER_B PORTD
    #define CONFIGIOxSTEPPER_B TRISD
    #define PINxSTEPPER_B  1
    //
    #define PORTWxSTEPPER_C LATD
    #define PORTRxSTEPPER_C PORTD
    #define CONFIGIOxSTEPPER_C TRISD
    #define PINxSTEPPER_C  2
    //
    #define PORTWxSTEPPER_D LATD
    #define PORTRxSTEPPER_D PORTD
    #define CONFIGIOxSTEPPER_D TRISD
    #define PINxSTEPPER_D  3

    #define PORTWxSTEPPER_ENABLE LATC
    #define PORTRxSTEPPER_ENABLE PORTC
    #define CONFIGIOxSTEPPER_ENABLE TRISC
    #define PINxSTEPPER_ENABLE  1

    //enable is inverted by hardware
    #define STEPPER_ENABLE() do{PinTo0(PORTWxSTEPPER_ENABLE, PINxSTEPPER_ENABLE);}while(0)
    #define STEPPER_DISABLE() do{PinTo1(PORTWxSTEPPER_ENABLE, PINxSTEPPER_ENABLE);}while(0)
    //
    #define PORTWxSTEPPER_SENSOR_HOME LATE
    #define PORTRxSTEPPER_SENSOR_HOME PORTE
    #define CONFIGIOxSTEPPER_SENSOR_HOME TRISE
    #define PINxSTEPPER_SENSOR_HOME 1
    
    ////////////////////////////////////////////////////////////////////////////
    // INPUTS
    #define PORTWxLEVELOIL LATC
    #define PORTRxLEVELOIL PORTC
    #define CONFIGIOxLEVELOIL TRISC
    #define PINxLEVELOIL  0

    #define PORTWxSTARTSIGNAL LATE
    #define PORTRxSTARTSIGNAL PORTE
    #define CONFIGIOxSTARTSIGNAL TRISE
    #define PINxSTARTSIGNAL  2
    
#ifdef	__cplusplus
    extern "C" {
    #endif 
    #ifdef	__cplusplus
    }
    #endif 

#endif	

