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
    
    #define MPAP_DELAY_BY_STEPS 1.0E-3 //ms
    void mpap_sych(void);

//x tests
void mpap_test(void);
void mpap_1(void);
void mpap_setupToTurn(int16_t numSteps_tomove);
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
    

#ifdef	__cplusplus
extern "C" {
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* MPAP_H */

