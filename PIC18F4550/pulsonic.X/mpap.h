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
        MPAP_IDLE_MODE=0,
        MPAP_STALL_MODE,
        MPAP_HOMMING_MODE,
        MPAP_NORMAL_MODE
    };
    struct _mpap
    {
        int16_t numSteps_current;//current position absolute
        int16_t counter_steps;
        int16_t numSteps_tomove;
        int8_t 	KI;//+-1
        int8_t mode;
    };
    extern volatile struct _mpap mpap;
    
    #define MPAP_NUMSTEP_1NOZZLE 200//200 steps to move to 1 nozzle
    
    void mpap_job(void);
    void mpap_setupToHomming(void);
    void mpap_setupToTurn(int16_t numSteps_tomove);    
    void mpap_movetoNozzle(int8_t n);//0..NOZZLE_NUMMAX-1
    int8_t mpap_isIdle(void);
    
    #define MPAP_DELAY_BY_STEPS 1.0E-3 //ms
    void mpap_sych(void);

void autoModexxx_job(void);
//x tests
//void mpap_test(void);
//void mpap_1(void);

/////////////////UNIPOLAR /////////////////////////////////////
//Secuencia Full unipolar 
//        LATD = 0B00001100;
//        __delay_ms(1);
//        LATD = 0B00000110;
//        __delay_ms(1);
//        LATD = 0B00000011;
//        __delay_ms(1);
//        LATD = 0B00001001;
//        __delay_ms(1);
//MICROPASO MITAD
//        LATD = 0B00001000;
//        __delay_ms(1);
//        LATD = 0B00001100;
//        __delay_ms(1);
//        LATD = 0B00000100;
//        __delay_ms(1);
//        LATD = 0B00000110;
//        __delay_ms(1);
//        LATD = 0B00000010;
//        __delay_ms(1);
//        LATD = 0B00000011;
//        __delay_ms(1);
//        LATD = 0B00000001;
//        __delay_ms(1);
//        LATD = 0B00001001;
//        __delay_ms(1);    
    
void l6506d_job(void);
#ifdef	__cplusplus
extern "C" {
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* MPAP_H */

