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
    //int8_t 
    codapp_t visMode_job(void);

    extern struct _ps ps_visMode;
    
#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* VISUALIZERMODE_H */

