/* 
 * File:   pump.h
 * Author: jcaf
 *
 * Created on July 11, 2019, 11:58 AM
 */

#ifndef PUMP_H
#define	PUMP_H

    #define PUMP_TICK_TIME_ON   50//in ms
    #define PUMP_TICK_TIME_OFF  75//in ms
    
    struct _pump
    {
        uint16_t ticks;
        int8_t sm0;
    };

    void pump_setTick(uint16_t ticks);
    uint16_t pump_getTick(void);
    void pump_stop(void);
    int8_t pump_job(void);
    int8_t pump_isIdle(void);

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* PUMP_H */

