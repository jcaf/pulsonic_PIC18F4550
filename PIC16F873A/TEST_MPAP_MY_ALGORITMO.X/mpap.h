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
    #define PORTWxSTEPPER_A PORTA
    #define PORTRxSTEPPER_A PORTA
    #define CONFIGIOxSTEPPER_A TRISA
    #define PINxSTEPPER_A 5

    #define PORTWxSTEPPER_B PORTA
    #define PORTRxSTEPPER_B PORTA
    #define CONFIGIOxSTEPPER_B TRISA
    #define PINxSTEPPER_B 4

    #define PORTWxSTEPPER_C PORTA
    #define PORTRxSTEPPER_C PORTA
    #define CONFIGIOxSTEPPER_C TRISA
    #define PINxSTEPPER_C 3

    #define PORTWxSTEPPER_D PORTA
    #define PORTRxSTEPPER_D PORTA
    #define CONFIGIOxSTEPPER_D TRISA
    #define PINxSTEPPER_D 2
//
//    #define PORTWxSTEPPER_ENABLE LATC
//    #define PORTRxSTEPPER_ENABLE PORTC
//    #define CONFIGIOxSTEPPER_ENABLE TRISC
//    #define PINxSTEPPER_ENABLE  1
//
//    //enable is inverted by hardware
//    #define STEPPER_ENABLE() do{PinTo0(PORTWxSTEPPER_ENABLE, PINxSTEPPER_ENABLE);}while(0)
//    #define STEPPER_DISABLE() do{PinTo1(PORTWxSTEPPER_ENABLE, PINxSTEPPER_ENABLE);}while(0)
//    //
//    #define PORTWxSTEPPER_SENSOR_HOME LATE
//    #define PORTRxSTEPPER_SENSOR_HOME PORTE
//    #define CONFIGIOxSTEPPER_SENSOR_HOME TRISE
//    #define PINxSTEPPER_SENSOR_HOME 1


#ifdef	__cplusplus
extern "C" {
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* MPAP_H */

