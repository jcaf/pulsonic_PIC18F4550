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

union _errorFlag {

    struct _errorFlag_flags {
        unsigned oilLevel : 1;
        unsigned homeSensor : 1;
        unsigned unblockedNozzle : 1;
        unsigned inductiveSensorRPM : 1;
        unsigned __a : 4;

        //si es mas de 8 - > modificar NUMMAX_ERRORS.. maximo 16
    } f;
    intNumMaxErr_t packed;
};

struct _pulsonic_display {
    uint8_t qty[DISP7S_QTY_NUMMAX];
    uint8_t mode[DISP7S_MODE_NUMMAX];
};

struct _pulsonic {
    union _errorFlag error;

    double ml_x1tick;
    
    //double distTotalTime_min; //=60min
    double distTotalTime_min;
    
    //double distAccessTime_min; //
    double distAccessTime_min;
    
    
    uint32_t distAccessTime_ms; //
    
    double timeslice_min;

    struct _pulsonic_nozzle {
        double Q_mlh; //quantity ml/h
        double nticks_xtimeslice_min;
        double kmax_ticks_xtimeslice;
        //
        double accError;

        //x debug
        //double accTicks;
        int counterTicks_xTotalTime;
        uint16_t nticks_delivered_inThisTimeSlice;

    } nozzle[NOZZLE_NUMMAX];

    struct _pulsonic_oil {
        int8_t i;
        int8_t viscosity;
    } oil;

    double sum_all_mlh; //sum_all_mlh
    int8_t numNozzleAvailable;
    uint32_t kTimeBetweenNozzleAvailable;
    uint32_t countTimeBetweenNozzleAvailable;

    int16_t num_timeslice;
    int8_t countNozzleAvailable;


    struct _pulsonic_display disp7s;

    struct _pulsonic_flags {
        unsigned homed : 1;
        unsigned __a : 7;
    } flags;

    int8_t numNozzle; //current nozzle position
};

extern volatile struct _pulsonic pulsonic;
#define OIL_VISCOSITY_NUMMAX 4
extern const int8_t OIL_VISCOSITY[OIL_VISCOSITY_NUMMAX];

void pulsonic_init(void);
double pulsonic_getTotalSum_mlh(void);
int8_t pulsonic_getNumNozzleAvailable(void);
#endif	/* PULSONIC_H */



