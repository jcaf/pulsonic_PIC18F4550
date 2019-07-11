/* 
 * File:   pulsonic.h
 * Author: jcaf
 *
 * Created on July 4, 2019, 9:48 PM
 */

#ifndef PULSONIC_H
#define	PULSONIC_H

    #define NOZZLE_NUMMAX 18
    //
    #define DISP_MODE_NUMMAX 2
    #define QUANTITY_NUMAX 3
    
    enum _QUANT_DIG
    {
        QUANT_DIG_0,
        QUANT_DIG_1,
        QUANT_DIG_2,
    };
    enum _MODE_DIG
    {
        MODE_DIG_0,
        MODE_DIG_1,
    };

    #define DISP_TOTAL_NUMMAX (DISP_MODE_NUMMAX + QUANTITY_NUMAX)
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

        uint8_t display7s[DISP_TOTAL_NUMMAX];
    };
    extern struct _pulsonic pulsonic;

    void display7s_init(void);
    void display7s_job(void);

struct _multiplexedDisp
{
    PTRFX_retVOID Qonoff[2];//0=..off(), 1=..on()
};
extern struct _multiplexedDisp multiplexedDisp[DISP_TOTAL_NUMMAX];

void disp_show_quantity(double f);

extern const uint8_t DISP7S_NUMS[10];
extern const uint8_t DISP7S_CHARS[2];

enum _DISP7S_CHARS {
    OFF,
    RAYA
};
#endif	/* PULSONIC_H */


   
