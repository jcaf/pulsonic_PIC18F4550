/* 
 * File:   configMode.h
 * Author: jcaf
 *
 * Created on July 14, 2019, 1:15 PM
 */

#ifndef CONFIGMODE_H
#define	CONFIGMODE_H

    #define CONFIGMODE_NUMMAX_REGISTROS (NOZZLE_NUMMAX+1)//+1=oil viscosity
    void configMode_init(int8_t numRegistro);
    int8_t configMode_job(void);
#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* CONFIGMODE_H */

