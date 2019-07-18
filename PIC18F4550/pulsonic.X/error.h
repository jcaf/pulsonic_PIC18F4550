/* 
 * File:   error.h
 * Author: jcaf
 *
 * Created on July 18, 2019, 1:26 PM
 */

#ifndef ERROR_H
#define	ERROR_H

#ifdef	__cplusplus
extern "C" {
#endif

    /*NumMax can be 8 or 16 */
    #define NUMMAX_ERRORS 8//16

    #if NUMMAX_ERRORS == 8
        typedef int8_t intNumMaxErr_t;
    #elif NUMMAX_ERRORS == 16
        typedef int16_t intNumMaxErr_t
    #else
        #error NumMax can be 8 or 16
    #endif

    void error_job(void);


#ifdef	__cplusplus
}
#endif

#endif	/* ERROR_H */

