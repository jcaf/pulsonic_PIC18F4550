/* 
 * File:   inputs.h
 * Author: jcaf
 *
 * Created on July 10, 2019, 4:13 PM
 */

#ifndef INPUTS_H
#define	INPUTS_H

/*NumMax can be 8 or 16 */
#define NUMMAX_ERRORS 8//16

#if NUMMAX_ERRORS == 8
    typedef int8_t intNumMaxErr_t;
#elif NUMMAX_ERRORS == 16
    typedef int16_t intNumMaxErr_t
#else
    #error NumMax can be 8 or 16
#endif
    void startSignal_init(void);
    int8_t is_startSignal(void);
    int8_t is_oilLevel(void);
    int8_t is_homeSensor(void);
    int8_t is_unblockedNozzle(int8_t nozzle);
    int8_t is_inductiveSensorRPM(void);

#ifdef	__cplusplus
extern "C" {
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* INPUTS_H */

