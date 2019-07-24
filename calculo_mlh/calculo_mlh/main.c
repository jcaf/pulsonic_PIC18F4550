/*
Con Ronald calculamos que 300 ticks = 10 ml
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

    #define NOZZLE_NUMMAX 8
    #define DISP7S_QTY_NUMMAX 3
    #define DISP7S_MODE_NUMMAX 2


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


        double ml_x1tick;


        double dist_total_time;//=60min
        double dist_access_time;//en 60 min, cada cuanto tiempo se accede(en min)
        double timeslice;

        struct _pulsonic_nozzle
        {
            double Q_mlh;                //quantity ml/h
            double nticks_xtimeslice;
            double kmax_ticks_xtimeslice;
            //
            double accError;
            //double accTicks;

        }nozzle[NOZZLE_NUMMAX];


        struct _pulsonic_oil
        {
            int8_t i;
            int8_t viscosity;
        }oil;

        double sum_all_mlh;                    //sum_all_mlh

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

    }pulsonic;
//-----------------------------------------------------------------------
    //experimental
//    double Qmhl_measured = 10;//ml
//    double nTicks_measured = 300;//#ticks
    double Qmhl_measured = 5.0;//ml
    double nTicks_measured = 231;//#ticks
int main()
{
    int c;
    int i;
    double nticksReq_xTotalTime;
    double e = 0;
    int counterTicks_xTotalTime = 0;
    uint16_t nticks_delivered_inThisTimeSlice=0;
    //
    //pulsonic.ml_x1tick = 5.0/231; //5ml/231 ticks 0.021645ml/tick
    pulsonic.ml_x1tick = Qmhl_measured/nTicks_measured; //con Ronald
    //
    pulsonic.dist_total_time = 60;//=60min
    pulsonic.dist_access_time = 2;//c/2min
    pulsonic.timeslice = pulsonic.dist_total_time/pulsonic.dist_access_time;//30
    //
    pulsonic.nozzle[0].Q_mlh = 12;//ml
    nticksReq_xTotalTime = pulsonic.nozzle[0].Q_mlh / pulsonic.ml_x1tick;
    pulsonic.nozzle[0].nticks_xtimeslice = nticksReq_xTotalTime / pulsonic.timeslice;

    printf("RESUMEN DE DATOS DE ENTRADA:\n\n");
    //
    printf("Cantidad medidas experimentalmente ( #ticks-->#ml ):  \n\n");
    printf("Cantidad ml medido, Qmhl_measured = %.2f ml \n", Qmhl_measured);
    printf("Cantidad ticks medido, nTicks_measured = %.2f ticks \n", nTicks_measured);
    printf("=> Cantidad de ml/tick, pulsonic.ml_x1tick = %f ml/tick \n\n", pulsonic.ml_x1tick);
    //
    printf("pulsonic.dist_total_time = %.2f min\n", pulsonic.dist_total_time);
    printf("pulsonic.dist_access_time = %.2f min\n", pulsonic.dist_access_time);
    printf("=> pulsonic.timeslice = %.2f partes\n\n", pulsonic.timeslice);
    //
    printf("pulsonic.nozzle[0].Q_mlh = %.2f ml/h\n", pulsonic.nozzle[0].Q_mlh);
    printf("=>Total ticks deseados en %.2f min = %f ticks \n", pulsonic.dist_total_time, nticksReq_xTotalTime);
    printf("=>pulsonic.nozzle[0].nticks_xtimeslice = %f ticks/timeslice \n\n", pulsonic.nozzle[0].nticks_xtimeslice);


    if (pulsonic.nozzle[0].nticks_xtimeslice <1)
        pulsonic.nozzle[0].kmax_ticks_xtimeslice = 1;
    else
        pulsonic.nozzle[0].kmax_ticks_xtimeslice = (uint16_t)pulsonic.nozzle[0].nticks_xtimeslice;//trunca

    printf("********** INICIO DE ENTREGA************\n\n");

    for (i=0; i< (int)pulsonic.timeslice; i++)
    {
            printf("# time slice = %i (min=%.2f)\n", i+1, (i+1)*pulsonic.dist_access_time);

            nticks_delivered_inThisTimeSlice = 0;

            pulsonic.nozzle[0].accError += pulsonic.nozzle[0].nticks_xtimeslice;

            if (pulsonic.nozzle[0].accError >= pulsonic.nozzle[0].kmax_ticks_xtimeslice)
            {
                e = pulsonic.nozzle[0].accError - pulsonic.nozzle[0].kmax_ticks_xtimeslice;
                if ( e>=1)
                {
                    e = e - 1;
                    nticks_delivered_inThisTimeSlice+= 1;
                }
                pulsonic.nozzle[0].accError = e;
            }

            if (i== ((int)pulsonic.timeslice)-1)
            {
                if (e>0)
                    nticks_delivered_inThisTimeSlice+= 1;
            }
            nticks_delivered_inThisTimeSlice +=(uint16_t)pulsonic.nozzle[0].kmax_ticks_xtimeslice;

            counterTicks_xTotalTime += nticks_delivered_inThisTimeSlice;

            //pulsonic.nozzle[0].accTicks += pulsonic.nozzle[0].nticks_xtimeslice;
            //printf("Acumulador de ticks AaccT = %f\n", pulsonic.nozzle[0].accTicks);

            printf("\n# TOTAL DE TICKS ENTREGADOS EN ESTE TIMESLICE = %i \n", nticks_delivered_inThisTimeSlice);
            printf("-----------------------------------------------------\n\n");

    }

    printf("RESUMEN DE DATOS DE SALIDA:\n\n", pulsonic.dist_total_time);
    //
    printf("Total ticks deseados en %.2f min = %f ticks \n", pulsonic.dist_total_time, nticksReq_xTotalTime);
    printf("Total ticks entregados en %.2f min = %i ticks \n", pulsonic.dist_total_time,counterTicks_xTotalTime);
    printf("ml/h Deseados: pulsonic.nozzle[0].Q_mlh = %f ml/h\n", pulsonic.nozzle[0].Q_mlh);
    printf("ml/h Entregados: pulsonic.nozzle[0].Q_mlh = %f ml/h\n", counterTicks_xTotalTime * pulsonic.ml_x1tick);
    printf("ml/h error: (Entregados-deseados) %f ml/h\n", (counterTicks_xTotalTime * pulsonic.ml_x1tick) - pulsonic.nozzle[0].Q_mlh);

    return 0;
}
