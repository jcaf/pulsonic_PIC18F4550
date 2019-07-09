/* 
 * File:   mpap.h
 * Author: jcaf
 *
 * Created on July 6, 2019, 11:53 AM
 */

#ifndef MPAP_H
#define	MPAP_H

    enum MPAP_MODES
    {
        IDLE_MODE=0,
        STALL_MODE,
        HOMMING_MODE,
        NORMAL_MODE
    };
    struct _mpap
    {
        int16_t numSteps_current;//current position absolute
        int16_t counter_steps;
        int16_t numSteps_tomove;
        int8_t 	KI;//+-1

        // struct _mpap_flag
        // {
        // 	unsigned run:1;
        // 	unsigned __a:7;
        // }flag;
        int8_t mode;
    };
    extern volatile struct _mpap mpap;
    
    #define MPAP_NUMSTEP_1NOZZLE 200//200 steps to move to 1 nozzle
    //#define NOZZLE_NUMMAX 18//0,1,-...-(NOZZLE_NUMMAX-1)
    int8_t mpap_job(void);
    void mpap_setupToHomming(void);
    void nozzle_moveto(int8_t nozzle);//0..NOZZLE_NUMMAX-1
//x tests
void mpap_test(void);
void mpap_1(void);
void mpap_setupToTurn(int16_t numSteps_tomove);
    
    //MOTOR PAP UNIPOLAR
    #define PORTWxSTEPPERBIP0_A LATD
    #define PORTRxSTEPPERBIP0_A PORTD
    #define CONFIGIOxSTEPPERBIP0_A TRISD
    #define PINxSTEPPERBIP0_A  0
    //
    #define PORTWxSTEPPERBIP0_B LATD
    #define PORTRxSTEPPERBIP0_B PORTD
    #define CONFIGIOxSTEPPERBIP0_B TRISD
    #define PINxSTEPPERBIP0_B  1
    //
    #define PORTWxSTEPPERBIP0_C LATD
    #define PORTRxSTEPPERBIP0_C PORTD
    #define CONFIGIOxSTEPPERBIP0_C TRISD
    #define PINxSTEPPERBIP0_C  2
    //
    #define PORTWxSTEPPERBIP0_D LATD
    #define PORTRxSTEPPERBIP0_D PORTD
    #define CONFIGIOxSTEPPERBIP0_D TRISD
    #define PINxSTEPPERBIP0_D  3

    #define PORTWxSTEPPERBIP0_ENABLE LATC
    #define PORTRxSTEPPERBIP0_ENABLE PORTC
    #define CONFIGIOxSTEPPERBIP0_ENABLE TRISC
    #define PINxSTEPPERBIP0_ENABLE  1

    //enable is inverted by hardware
    #define STEPPERBIP0_ENABLE() do{PinTo0(PORTWxSTEPPERBIP0_ENABLE, PINxSTEPPERBIP0_ENABLE);}while(0)
    #define STEPPERBIP0_DISABLE() do{PinTo1(PORTWxSTEPPERBIP0_ENABLE, PINxSTEPPERBIP0_ENABLE);}while(0)
    //
    #define PORTWxSTEPPERBIP0_SENSOR_HOME LATE
    #define PORTRxSTEPPERBIP0_SENSOR_HOME PORTE
    #define CONFIGIOxSTEPPERBIP0_SENSOR_HOME TRISE
    #define PINxSTEPPERBIP0_SENSOR_HOME 1


#ifdef	__cplusplus
extern "C" {
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* MPAP_H */

