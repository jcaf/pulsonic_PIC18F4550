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

    /*NumMax can be 8 or 16 */
    #define NUMMAX_ERRORS 8//16

    #if NUMMAX_ERRORS == 8
        typedef int8_t intNumMaxErr_t;
    #elif NUMMAX_ERRORS == 16
        typedef int16_t intNumMaxErr_t
    #else
        #error NumMax can be 8 or 16
    #endif
    union _errorFlag
    {
        struct _errorFlag_flags
        {
            unsigned oilLevel:1;
            unsigned homeSensor:1;
            unsigned unblockedNozzle:1;
            unsigned inductiveSensorRPM:1;
            unsigned __a :4;

            //si es mas de 8 - > modificar NUMMAX_ERRORS.. maximo 16
        }f;
        intNumMaxErr_t packed;
    };

    struct _pulsonic
    {
        union _errorFlag error;
        
        uint16_t Kticks_0p1ml;//n ticks per 0.1ml
        uint16_t dist_total_time;//=60min
        uint16_t dist_access_time;//en 60 min, cada cuanto tiempo se accede(en min)

        float sum_all_mlh;                    //sum_all_mlh
        
        struct _pulsonic_nozzle
        {
            float Q_mlh;                //quantity ml/h
            uint16_t total_ticks_mlh;   //total ticks pe "X" mlh
            int8_t nticks_slicetime;    //total ticks in slicetime
        }nozzle[NOZZLE_NUMMAX];

        struct _pulsonic_oil
        {
            int8_t i;
            int8_t viscosity;
        }oil;
        
        struct _pulsonic_display
        {
            uint8_t qty[DISP7S_QTY_NUMMAX];
            uint8_t mode[DISP7S_MODE_NUMMAX];
        }disp7s;
        
        struct _pulsonic_flags
        {
            unsigned homed:1;
            unsigned __a:7;
        }flags;
        
    };
    
    extern volatile struct _pulsonic pulsonic;
    #define OIL_VISCOSITY_NUMMAX 4
    extern const int8_t OIL_VISCOSITY[OIL_VISCOSITY_NUMMAX];
    
    void pulsonic_init(void);
    double pulsonic_getTotalSum_mlh(void);
    
#endif	/* PULSONIC_H */


   
