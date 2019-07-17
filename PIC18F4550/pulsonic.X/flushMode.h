/* 
 * File:   flush.h
 * Author: jcaf
 *
 * Created on July 16, 2019, 3:46 PM
 */

#ifndef FLUSH_H
#define	FLUSH_H


void flushMode_cmd(int8_t cmd);
void flushMode_job(void);

enum _FLUSH_CMD
{
    FLUSH_CMD_RESTART = 0,
    FLUSH_CMD_STOP
};

#ifdef	__cplusplus
extern "C" {
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* FLUSH_H */

