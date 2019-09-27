/* 
 * File:   visualizerMode.h
 * Author: jcaf
 *
 * Created on July 12, 2019, 10:13 PM
 */

#ifndef VISUALIZERMODE_H
#define	VISUALIZERMODE_H

    #define VISMODE_NUMMAX_VISTAS (NOZZLE_NUMMAX+1)//+1=oil viscosity    

    void visMode_init(int8_t numVista);
    void visMode_job(void);

    struct _visMode
    {
        int8_t numVista;        //number of "vista"
        int8_t disp7s_accessReq;//access request
        
        //added:
        int8_t timerRun;
        uint32_t timer;
        int8_t lockTimming;
        
    };
    struct _visMode visMode;

    extern struct _ps ps_visMode;
    
    #define VISMODE_KTIME_INACTIVITY_EXIT_MS (10*1000)//10s
    
#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* VISUALIZERMODE_H */

