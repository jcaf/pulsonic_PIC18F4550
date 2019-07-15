#ifndef MAIN_H_
#define	MAIN_H_

    #include <xc.h> // include processor files - each processor file is guarded.  
    #include <stdint.h>
    #define F_CPU 48000000UL//48MHz
    #include "system.h"
    #include "types.h"

    #pragma warning disable 752
    #pragma warning disable 356
    #pragma warning disable 373//implicit signed to unsigned conversion
    #pragma warning disable 520//function is never called

    struct _smain
    {
        struct _main_flag
        {
            unsigned f1ms:1;
            unsigned __a:7;
        }f;
        
        struct _main_focus
        {
            int8_t kb;
            int8_t disp;
        }focus;
    };
    extern volatile struct _smain smain;
    
    #define myitoa(_integer_, _buffer_, _base_) itoa(_buffer_, _integer_, _base_)

    extern int8_t disp_owner;
    enum _DISPOWNER_MODE
    {
        DISPOWNER_AUTOMODE = 0,
        DISPOWNER_VISMODE,
        DISPOWNER_CONFIGMODE,
    };
   
    enum _FOCUS
    {
        FOCUS_KB_NONE,
        FOCUS_KB_AUTOMODE,
        FOCUS_KB_VISMODE,
        FOCUS_KB_CONFIGMODE
    };
    
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
    ////////////////////////////////////////////////////////////////////////////
    //OIL-PUMP MOTOR
    #define PORTWxPUMP LATE
    #define PORTRxPUMP PORTE
    #define CONFIGIOxPUMP TRISE
    #define PINxPUMP  0

    #define PUMP_DISABLE()  do{PinTo1(PORTWxPUMP, PINxPUMP);}while(0)
    #define PUMP_ENABLE()   do{PinTo0(PORTWxPUMP, PINxPUMP);}while(0)
    ////////////////////////////////////////////////////////////////////////////
    // INPUTS
    #define PORTWxOILLEVEL LATC
    #define PORTRxOILLEVEL PORTC
    #define CONFIGIOxOILLEVEL TRISC
    #define PINxOILLEVEL  0

    #define PORTWxSTEPPER_SENSOR_HOME LATE
    #define PORTRxSTEPPER_SENSOR_HOME PORTE
    #define CONFIGIOxSTEPPER_SENSOR_HOME TRISE
    #define PINxSTEPPER_SENSOR_HOME 1

    #define PORTWxSTARTSIGNAL LATE
    #define PORTRxSTARTSIGNAL PORTE
    #define CONFIGIOxSTARTSIGNAL TRISE
    #define PINxSTARTSIGNAL  2
    
    #define PORTWxRELAY LATA
    #define PORTRxRELAY PORTA
    #define CONFIGIOxRELAY TRISA
    #define PINxRELAY  5

    #define RELAY_ENABLE() do{PinTo0(PORTWxRELAY, PINxRELAY);}while(0)
    #define RELAY_DISABLE() do{PinTo1(PORTWxRELAY, PINxRELAY);}while(0)
    
    ////////////////////////////////////////////////////////////////////////////
    #define KB_LYOUT_KEY_UP      0
    #define KB_LYOUT_KEY_DOWN    1
    #define KB_LYOUT_KEY_PLUS    2
    #define KB_LYOUT_KEY_MINUS   3
    #define KB_LYOUT_KEY_ENTER_F 4//Enter/Flush

#ifdef	__cplusplus
    extern "C" {
    #endif 
    #ifdef	__cplusplus
    }
    #endif 

#endif	

