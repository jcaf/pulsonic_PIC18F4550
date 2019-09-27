/* 
 * File:   nozzle.h
 * Author: jcaf
 *
 * Created on July 11, 2019, 4:52 PM
 */

#ifndef NOZZLE_H
#define	NOZZLE_H

    #define NOZZLE_NUMMAX 18

    #define NOZZLE_OFF 0.0f
    #define NOZZLE_QMLH_MIN 0.3f
    #define NOZZLE_QMLH_MAX 12.0f
    extern double NOZZLE_QTY_DIFF;
    
    int8_t nozzle_isEnabled(int n);
    int8_t nozzle_getPosition(void);
    int8_t nozzle_setPosition(int8_t n);
    void nozzle_setPosition_reset(void);

#ifdef	__cplusplus
extern "C" {
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* NOZZLE_H */

