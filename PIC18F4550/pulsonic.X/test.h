/* 
 * File:   test.h
 * Author: jcaf
 *
 * Created on July 2, 2019, 10:34 AM
 */

#ifndef TEST_H
#define	TEST_H
    #define PORTWxKBFIL_1 		LATD
    #define PORTRxKBFIL_1 		PORTD
    #define CONFIGIOxKBFIL_1 	TRISD
    #define PINxKBFIL_1 		2

#define PORTWxKBFIL_KEY1 	PORTWxKBFIL_1
#define PORTRxKBFIL_KEY1 	PORTRxKBFIL_1
#define CONFIGIOxKBFIL_KEY1 	CONFIGIOxKBFIL_1
#define PINxKBFIL_KEY1 		PINxKBFIL_1
//
 
//#define KB_NUM_KEYS 1

void kb_initx(void);

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* TEST_H */

