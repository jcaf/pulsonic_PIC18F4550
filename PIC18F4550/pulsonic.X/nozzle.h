/* 
 * File:   nozzle.h
 * Author: jcaf
 *
 * Created on July 11, 2019, 4:52 PM
 */

#ifndef NOZZLE_H
#define	NOZZLE_H

    #define NOZZLE_NUMMAX 8//18

    #define NOZZLE_OFF 0
    #define NOZZLE_QMLH_MIN 0.3
    #define NOZZLE_QMLH_MAX 12.0

    
    int8_t nozzle_isEnabled(int n);

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* NOZZLE_H */

