/* 
 * File:   inputs.h
 * Author: jcaf
 *
 * Created on July 10, 2019, 4:13 PM
 */

#ifndef INPUTS_H
#define	INPUTS_H

    void startSignal_init(void);
    int8_t is_startSignal(void);
    
    void oilLevel_init(void);
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

