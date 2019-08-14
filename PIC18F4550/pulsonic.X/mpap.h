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
        MPAP_IDLE_MODE=0,   //fixed idx
        MPAP_STALL_MODE=1,    //fixed idx
        MPAP_SEARCH_FIRSTPOINT_HOMESENSOR_MODE,
        MPAP_CROSSING_HOMESENSOR_MODE,
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
    void mpap_setup_searchFirstPointHomeSensor(void);
    
    void mpap_movetoNozzle(int8_t n);//0..NOZZLE_NUMMAX-1
    int8_t mpap_isIdle(void);
    void mpap_do1step(int8_t KI);
    
    void mpap_stall(void);
    //void mpap_setupToTurn(int16_t numSteps_tomove);    
    void mpap_setMode(int8_t mode);
    int8_t mpap_getMode(void);
    void mpap_doMovement(int16_t numSteps_tomove, int8_t mode);
    int16_t mpap_get_numSteps_current(void);
    int8_t mpap_homming_job(void);
    void  mpap_homming_job_reset(void);
    //WAVE
    #define STEP_WAVE_1A (1<<1)//RD1
    #define STEP_WAVE_2A (1<<2)//RD2
    #define STEP_WAVE_1B (1<<3)//RD3
    #define STEP_WAVE_2B (1<<0)//RD0
    //FULL
    #define STEP_FULL_A (STEP_WAVE_1A | STEP_WAVE_2B)
    #define STEP_FULL_B (STEP_WAVE_1A | STEP_WAVE_2A)
    #define STEP_FULL_C (STEP_WAVE_1B | STEP_WAVE_2A)
    #define STEP_FULL_D (STEP_WAVE_1B | STEP_WAVE_2B)

    #define STEP_HALF_1 STEP_WAVE_1A
    #define STEP_HALF_2 STEP_FULL_A
    #define STEP_HALF_3 STEP_WAVE_2A
    #define STEP_HALF_4 STEP_FULL_B
    #define STEP_HALF_5 STEP_WAVE_1B
    #define STEP_HALF_6 STEP_FULL_C
    #define STEP_HALF_7 STEP_WAVE_2B
    #define STEP_HALF_8 STEP_FULL_D

    #define STEP_HALF_1 STEP_WAVE_1A
    #define STEP_HALF_2 STEP_FULL_A //(STEP_WAVE_1A | STEP_WAVE_2B)
    #define STEP_HALF_3 STEP_WAVE_2A
    #define STEP_HALF_4 STEP_FULL_B //(STEP_WAVE_2A | STEP_WAVE_1A)
    #define STEP_HALF_5 STEP_WAVE_1B
    #define STEP_HALF_6 STEP_FULL_C //(STEP_WAVE_1B | STEP_WAVE_2A)
    #define STEP_HALF_7 STEP_WAVE_2B
    #define STEP_HALF_8 STEP_FULL_D //(STEP_WAVE_2B | STEP_WAVE_1B)

#ifdef	__cplusplus
extern "C" {
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* MPAP_H */

