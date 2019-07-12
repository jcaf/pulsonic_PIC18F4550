/* 
 * File:   pulsonic.h
 * Author: jcaf
 *
 * Created on July 4, 2019, 9:48 PM
 */

#ifndef PULSONIC_H
#define	PULSONIC_H
  
    #include "nozzle.h"
    #include "display.h"
    //
    struct _pulsonic
    {
        uint16_t Kticks_0p1ml;//n ticks per 0.1ml
        uint16_t dist_total_time;//=60min
        uint16_t dist_access_time;//en 60 min, cada cuanto tiempo se accede(en min)

        struct _pulsonic_nozzle
        {
            float Q_mlh;                //quantity ml/h
            uint16_t total_ticks_mlh;   //total ticks pe "X" mlh
            int8_t nticks_slicetime;    //total ticks in slicetime
        }nozzle[NOZZLE_NUMMAX];

        struct _pulsonic_oil
        {
            int8_t viscosity;
        }oil;
        
        float sum_all_mlh;                    //sum_all_mlh

        union _pulsonic_errors
        {
            struct _pulsonic_flags
            {
                unsigned mpap_home_sensor:1;
                unsigned __a:7;
            }flag;
            uint8_t e;
        }errors;

        struct _pulsonic_display
        {
            uint8_t qty[DISP7S_QTY_NUMMAX];
            uint8_t mode[DISP7S_MODE_NUMMAX];
        }disp7s;
        //uint8_t display7s[DISP_TOTAL_NUMMAX];
        
    };
    extern struct _pulsonic pulsonic;

    void pulsonic_init(void);
    uint16_t pulsonic_getTotalSum_mlh(void);
    
#endif	/* PULSONIC_H */


   
