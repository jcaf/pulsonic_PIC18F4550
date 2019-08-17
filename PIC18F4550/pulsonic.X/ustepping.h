/* 
 * File:   newfile.h
 * Author: jcaf
 *
 * Created on August 13, 2019, 12:29 PM
 */

//#define PR2_GET_VAL(T_REQ)( T_REQ/(4*(1/F_CPU)*TMR2_Preescaler) ) -1

#ifndef USTEPPING_H
#define	USTEPPING_H
    
    #define PR2_VAL 149 //20khz-> 50us
    //#define PR2_VAL 74 //40khz-> 25us
    
    #define DC_TOP (PR2_VAL<<2)

    #define MICROSTEP_N 4
    //#define DC_MIN 0.5*1024f
    //#define DC_MAX 0.7*1024f
    
    #define DC_MIN (0.35*DC_TOP)
    #define DC_MAX (0.8*DC_TOP)

    #define MICROSTEP (DC_MAX - DC_MIN)/MICROSTEP_N

    void setdc(uint16_t dc);
    extern const uint16_t ustep_lockup[MICROSTEP_N+1];
    
#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* USTEPPING_H */

