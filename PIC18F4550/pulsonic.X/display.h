/* 
 * File:   display.h
 * Author: jcaf
 *
 * Created on July 11, 2019, 12:56 PM
 */

#ifndef DISPLAY_H
#define	DISPLAY_H

    //DISLAYS DE CATODO COMUN - Todo el puerto de 8bits
    #define PORTWxDISPLAY 	LATB
    #define PORTRxDISPLAY 	PORTB
    #define CONFIGIOxDISPLAY 	TRISB
    //#define PINxDISPLAY 		0

    //TRANSISTORES PARA EL BARRIDO/MULTIPLEXING 
    #define PORTWxENABLE_DISP_0 	LATA
    #define PORTRxENABLE_DISP_0 	PORTA
    #define CONFIGIOxENABLE_DISP_0 	TRISA
    #define PINxENABLE_DISP_0 		0

    #define PORTWxENABLE_DISP_1 	LATA
    #define PORTRxENABLE_DISP_1 	PORTA
    #define CONFIGIOxENABLE_DISP_1 	TRISA
    #define PINxENABLE_DISP_1 		1

    #define PORTWxENABLE_DISP_2 	LATA
    #define PORTRxENABLE_DISP_2 	PORTA
    #define CONFIGIOxENABLE_DISP_2 	TRISA
    #define PINxENABLE_DISP_2 		2

    #define PORTWxENABLE_DISP_3 	LATA
    #define PORTRxENABLE_DISP_3 	PORTA
    #define CONFIGIOxENABLE_DISP_3 	TRISA
    #define PINxENABLE_DISP_3 		3

    #define PORTWxENABLE_DISP_4 	LATA
    #define PORTRxENABLE_DISP_4 	PORTA
    #define CONFIGIOxENABLE_DISP_4 	TRISA
    #define PINxENABLE_DISP_4 		4

     //
    #define DISP7S_MODE_NUMMAX 2
    #define DISP7S_QTY_NUMMAX 3
    #define DISP7S_TOTAL_NUMMAX (DISP7S_MODE_NUMMAX + DISP7S_QTY_NUMMAX)

    void disp7s_qtyDisp_writeFloat(float f);
    void disp7s_modeDisp_writeFloat(float f);
    void disp7s_init(void);
    void disp7s_job(void);
    
    extern const uint8_t DISP7S_NUMS[10];
    extern const uint8_t DISP7S_CHARS[2];

    enum _DISP7S_CHARS {
        OFF,
        RAYA
    };

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* DISPLAY_H */

