/* 
 * File:   automode.h
 * Author: jcaf
 *
 * Created on July 11, 2019, 7:14 PM
 */

#ifndef AUTOMODE_H
#define	AUTOMODE_H

    enum _AUTOMODE_INIT_I
    {
        AUTOMODE_INIT_RESTART = 0,
        AUTOMODE_INIT_CONTINUE
    };
    void autoMode_init(int8_t init);
    int8_t autoMode_job(void);
    
    
#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* AUTOMODE_H */

