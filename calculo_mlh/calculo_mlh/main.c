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
    double nticks_xh;
    //
    //pulsonic.ml_x1tick = 5.0/231; //5ml/231 ticks 0.021645ml/tick
    pulsonic.ml_x1tick = Qmhl_measured/nTicks_measured; //con Ronald
    //
    pulsonic.dist_total_time = 60;//=60min
    pulsonic.dist_access_time = 2;//c/2min
    pulsonic.timeslice = pulsonic.dist_total_time/pulsonic.dist_access_time;//30
    //
    pulsonic.nozzle[0].Q_mlh = 0.3;//ml
    nticks_xh = pulsonic.nozzle[0].Q_mlh / pulsonic.ml_x1tick;
    pulsonic.nozzle[0].nticks_xtimeslice = nticks_xh / pulsonic.timeslice;

    printf("RESUMEN DE DATOS DE ENTRADA:\n\n", pulsonic.dist_total_time);
    //
    printf("Cantidad medidas experimentalmente ( #ticks-->#ml ):  \n\n");
    printf("Cantidad ml medido, Qmhl_measured = %.2f ml \n", Qmhl_measured);
    printf("Cantidad ticks medido, nTicks_measured = %.2f ticks \n", nTicks_measured);
    printf("=> Cantidad de ml/tick, pulsonic.ml_x1tick = %f ml/tick \n\n", pulsonic.ml_x1tick);
    //
    printf("pulsonic.dist_total_time = %.2f min\n", pulsonic.dist_total_time);
    printf("pulsonic.dist_access_time = %.2f min\n", pulsonic.dist_access_time);
    printf("pulsonic.timeslice = %.2f partes\n\n", pulsonic.timeslice);
    //
    printf("pulsonic.nozzle[0].Q_mlh = %.2f ml/h\n", pulsonic.nozzle[0].Q_mlh);
    printf("nticks_xh = %f ticks de bombeo/h \n", nticks_xh);
    printf("pulsonic.nozzle[0].nticks_xtimeslice = %f ticks bombeo/parte \n\n", pulsonic.nozzle[0].nticks_xtimeslice);


    if (pulsonic.nozzle[0].nticks_xtimeslice <1)
    {
        pulsonic.nozzle[0].kmax_ticks_xtimeslice = 1;
    }
    else
    {
        pulsonic.nozzle[0].kmax_ticks_xtimeslice = (uint16_t)pulsonic.nozzle[0].nticks_xtimeslice;//trunca
    }

    double accT = 0;
    double acc = 0;
    double e = 0;
    int counterTicks = 0;
    uint16_t nticks_entregados=0;

    printf("---------------INICIO DE ENTREGA------------------\n\n");
    for (i=0; i< (int)pulsonic.timeslice; i++)
    {

            printf("==INCIO========== # time slice = %i (min=%.2f) ===============\n\n", i+1, (i+1)*pulsonic.dist_access_time);
            nticks_entregados = 0;

            if (1)//(accT < nticks_xh)
            {
                acc = acc + pulsonic.nozzle[0].nticks_xtimeslice;// + e;
                printf("acc = %f\n", acc);

                accT = accT + pulsonic.nozzle[0].nticks_xtimeslice;// + e;

                if (acc >= pulsonic.nozzle[0].kmax_ticks_xtimeslice)//if (acc >= 1.0)
                {

                    for (c=0; c< (uint16_t)pulsonic.nozzle[0].kmax_ticks_xtimeslice; c++)
                    {
                        counterTicks++;
                    }
                    nticks_entregados = c;

                    printf("counterTicks = %i \n", counterTicks);
                    printf("acc = %f\n", acc);

                    e = acc - pulsonic.nozzle[0].kmax_ticks_xtimeslice;//e = acc - 1.0;
                    printf("error = %f\n", e);

                    if ( e>=1)
                    {
                        printf("error fue e>=1= %f\n", e);
                        e = e - 1;

                        counterTicks++;
                        printf("counterTicks (1 tick mas)= %i \n", counterTicks);
                        //
                        nticks_entregados++;
                    }
                    acc = e;
                    printf("el nuevo error -> acc = %f\n", acc);
                }
            }
            printf("\n # TOTAL DE TICKS ENTREGADOS EN ESTE TIMESLICE = %i \n", nticks_entregados);
            printf("Acumulador de ticks AaccT = %f\n", accT);
            printf("==FIN==============================================================\n\n");
    }
    nticks_entregados = 0;
    if (e>0)
    {
        counterTicks++;
        printf("counterTicks = %i \n", counterTicks);

        nticks_entregados++;
        printf("\n # TOTAL DE TICKS ENTREGADOS EN ESTE TIMESLICE (ACIONAL A LA ULTIMA ENTREGA)= %i \n", nticks_entregados);
        printf("==FIN==============================================================\n\n");
    }

    printf("RESUMEN DE DATOS DE SALIDA:\n\n", pulsonic.dist_total_time);
    //
    printf("Total ticks entregados en %.2f min = %i ticks \n", pulsonic.dist_total_time,counterTicks);
    printf("ml/h Deseados: pulsonic.nozzle[0].Q_mlh = %f ml/h\n", pulsonic.nozzle[0].Q_mlh);
    printf("ml/h Entregados: pulsonic.nozzle[0].Q_mlh = %f ml/h\n", counterTicks * pulsonic.ml_x1tick);
    printf("ml/h error: (Entregados-deseados) %f ml/h\n", (counterTicks * pulsonic.ml_x1tick) - pulsonic.nozzle[0].Q_mlh);



    return 0;
}
