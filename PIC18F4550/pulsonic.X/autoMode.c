/*
 * Si
 * 
 *  double Qmhl_measured = 5.0;//ml
    double nTicks_measured = 268;//#ticks //esto fue comprobado con el medidor de jeringa
 * 
 *  pulsonic.distTotalTime_min = 60;//=60min
    pulsonic.dist_access_time = 1;//c/2min
 
 * pulsonic.nozzle[0].Q_mlh = 12;//ml
  
 *  #define PUMP_TICK_TIME_ON   50//in ms
    #define PUMP_TICK_TIME_OFF  75//in ms
 * -> Cada tick es = 50+75 = 125ms
 * -> Max time ticks 12 * 125ms = 1.5 s
 * -> 18 nozzles max * 1.5 = 27 s
 
 * ->18*200*2ms = 7.2s
 * 
 * TOTAL = 34.2s de consumo al maximo
 * 
 * 
 */
#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "autoMode.h"
#include "visMode.h"
#include "ikb/ikb.h"
#include "flushAllMode.h"

/*Medida oficial en jeringa*/
double Qmhl_measured = 5.0; //ml
double nTicks_measured = 268; //#ticks

//#define AUTOMODE_VER_0
#define AUTOMODE_VER_1

void autoMode_setup(void)
{
    int i;
    double nticksReq_xTotalTimeInMin;

    //pulsonic.ml_x1tick = 5.0/268; 
    pulsonic.ml_x1tick = Qmhl_measured / nTicks_measured;

    //expresed in minutes (min)
    pulsonic.distTotalTime_min = 60; //=60min
    pulsonic.distAccessTime_min = 1; //c x min
    pulsonic.timeslice_min = pulsonic.distTotalTime_min / pulsonic.distAccessTime_min;
    //
    pulsonic.distAccessTime_ms = pulsonic.distAccessTime_min*60*1000;
    
    //
    for (i = 0; i < NOZZLE_NUMMAX; i++)
    {
        if (nozzle_isEnabled(i))
        {
            //pulsonic.nozzle[i].Q_mlh = 12;//ml
            nticksReq_xTotalTimeInMin = pulsonic.nozzle[i].Q_mlh / pulsonic.ml_x1tick;
            pulsonic.nozzle[i].nticks_xtimeslice_min = nticksReq_xTotalTimeInMin / pulsonic.timeslice_min;

            if (pulsonic.nozzle[i].nticks_xtimeslice_min >= 1)
            {
                pulsonic.nozzle[i].kmax_ticks_xtimeslice = (uint16_t) pulsonic.nozzle[i].nticks_xtimeslice_min;
            }

            //added:
            pulsonic.nozzle[i].accError = 0x00;
        }
    }

    pulsonic.num_timeslice = 0x00;
    
#ifdef AUTOMODE_VER_0
    //added for firmware:
    //pulsonic.kTimeBetweenNozzleAvailable = (uint32_t) ((pulsonic.distAccessTime_min * 60 * 1000) / pulsonic.numNozzleAvailable); //en ms
    pulsonic.kTimeBetweenNozzleAvailable = (uint32_t) ((pulsonic.distAccessTime_ms) / pulsonic.numNozzleAvailable); //en ms
    pulsonic.countNozzleAvailable = 0x00;
#endif
}

static struct _autoMode
{
    int8_t sm0;
} autoMode;

void autoMode_disp7s_writeSumTotal(void)
{
    if (disp_owner == DISPOWNER_AUTOMODE)
    {
        //disp7s_modeDisp_off();
        disp7s_modeDisp_writeInt(20);
        disp7s_qtyDisp_writeFloat(pulsonic_getTotalSum_mlh());
    }
}

void autoMode_cmd(int8_t cmd)
{
    if (cmd == JOB_RESTART)
    {
        //pulsonic.numNozzle = 0x0;
        mpap_homming_job_reset();
        autoMode.sm0 = 0x1;
        //
        autoMode_setup();
    }
    else if (cmd == JOB_STOP)
    {
        autoMode.sm0 = 0x00;
    }
    pump_stop();
    mpap_stall();
}

void autoMode_job(void)
{
    static uint32_t tacc, ktime_residuary;
    static uint8_t numVueltas;

    //
    double e = 0;

    if (autoMode.sm0 > 0)
    {
        if (autoMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                if (mpap.numSteps_current == 0x000)//origin
                {
                    pulsonic.numNozzle = 0x0;
                    autoMode.sm0 += 2; //direct
                }
                else
                {
                    autoMode.sm0++;
                }
            }
        }
        else if (autoMode.sm0 == 2)
        {
            if (mpap_homming_job())//cod_ret = 1 o 2
            {
                pulsonic.numNozzle = 0x0;
                autoMode.sm0++;
            }
        } //Distribute oil
        else if (autoMode.sm0 == 3)
        {
            if (mpap_isIdle())
            {
                //+-
                pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice = 0;
                pulsonic.nozzle[pulsonic.numNozzle].accError += pulsonic.nozzle[pulsonic.numNozzle].nticks_xtimeslice_min;
                if (pulsonic.nozzle[pulsonic.numNozzle].accError >= pulsonic.nozzle[pulsonic.numNozzle].kmax_ticks_xtimeslice)
                {
                    e = pulsonic.nozzle[pulsonic.numNozzle].accError - pulsonic.nozzle[pulsonic.numNozzle].kmax_ticks_xtimeslice;
                    if (e >= 1)
                    {
                        e = e - 1;
                        pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice += 1;
                    }
                    pulsonic.nozzle[pulsonic.numNozzle].accError = e;
                }
                if (pulsonic.num_timeslice == ((int) pulsonic.timeslice_min) - 1)
                {
                    if (e > 0)
                        pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice += 1;
                }
                pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice += (uint16_t) pulsonic.nozzle[pulsonic.numNozzle].kmax_ticks_xtimeslice;
                pulsonic.nozzle[pulsonic.numNozzle].counterTicks_xTotalTime += pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice;
                //-+

                pump_setTick(pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice);
                autoMode.sm0++;

            }
        }
        else if (autoMode.sm0 == 4)
        {
            if (pump_isIdle())
            {
                tacc = pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice; //save
                numVueltas = 0x00;
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 5)
        {
            pulsonic.numNozzle++;

            if (pulsonic.numNozzle >= NOZZLE_NUMMAX)
            {
                pulsonic.numNozzle = 0x00; //aqui da por hecho que se completo la vuelta...
                //
                mpap_doMovement(1 * MPAP_NUMSTEP_1NOZZLE, MPAP_CROSSING_HOMESENSOR_MODE);
            }
            else
            {
                mpap_movetoNozzle(pulsonic.numNozzle);
            }
            numVueltas++;

            autoMode.sm0++;
        }
        else if (autoMode.sm0 == 6)
        {
            if (mpap_isIdle())
            {
                if (nozzle_isEnabled(pulsonic.numNozzle))
                {
                    tacc = (numVueltas * MPAP_NUMSTEP_1NOZZLE) + (tacc * (PUMP_TICK_TIME_ON + PUMP_TICK_TIME_OFF)); //ms
                    ktime_residuary = pulsonic.kTimeBetweenNozzleAvailable - tacc;
                    pulsonic.countTimeBetweenNozzleAvailable = 0x0000;
                    //
                    if (++pulsonic.countNozzleAvailable >= pulsonic.numNozzleAvailable)
                    {
                        pulsonic.countNozzleAvailable = 0;

                        if (++pulsonic.num_timeslice == ((int) pulsonic.timeslice_min))
                        {
                            pulsonic.num_timeslice = 0;
                        }
                    }
                    //
                    autoMode.sm0++;
                }
                else
                {
                    autoMode.sm0--; //un nuevo movimiento
                }
            }
        }
        else if (autoMode.sm0 == 7)
        {
            if (smain.f.f1ms)
            {
                if (++pulsonic.countTimeBetweenNozzleAvailable >= ktime_residuary)
                {
                    pulsonic.countTimeBetweenNozzleAvailable = 0x0000;
                    autoMode.sm0 = 3;
                    //

                }
            }
        }

    }//if (autoMode.sm0 >0)
}

/* This version is for latest firmware to Bryan Poma */
void autoMode1_job(void)
{
    static uint32_t timeEllapsed;
    //
    double e = 0;

    if (autoMode.sm0 > 0)
    {
        if (autoMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                pulsonic.numNozzle = 0x0;
                
                if (mpap.numSteps_current == 0x000)//origin
                    autoMode.sm0 = 3; //direct
                else
                    autoMode.sm0++; //mpap->homing
            }
        }
        else if (autoMode.sm0 == 2)
        {
            if (mpap_homming_job()) //cod_ret = 1 o 2
                autoMode.sm0++;
        }
        else
        {
            if (smain.f.f1ms == 1)
                {timeEllapsed++;}

            //Distribute oil
            if (autoMode.sm0 == 3)
            {
                while ( pulsonic.numNozzle < (NOZZLE_NUMMAX) )//termina con  pulsonic.numNozzle = NOZZLE_NUMMAX
                {
                    if (nozzle_isEnabled(pulsonic.numNozzle))
                        break;
                    else
                        pulsonic.numNozzle++;
                }
                
                if (pulsonic.numNozzle == NOZZLE_NUMMAX)
                {
                    if (nozzle_getPosition() == (NOZZLE_NUMMAX-1))
                    {
                        mpap_doMovement(1 * MPAP_NUMSTEP_1NOZZLE, MPAP_CROSSING_HOMESENSOR_MODE);
                        autoMode.sm0 = 7;
                    }
                    else
                    {
                        //mpap_movetoNozzle(pulsonic.numNozzle-1);
                        mpap_movetoNozzle(NOZZLE_NUMMAX-1);
                        autoMode.sm0 = 6;
                    }
                }
                else
                {
                    mpap_movetoNozzle(pulsonic.numNozzle);
                    //pulsonic.numNozzle++;
                    //
                    autoMode.sm0 = 4;
                }
            }
            else if (autoMode.sm0 == 4)
            {
                if (mpap_isIdle())
                {
                    //+-
                    pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice = 0;
                    pulsonic.nozzle[pulsonic.numNozzle].accError += pulsonic.nozzle[pulsonic.numNozzle].nticks_xtimeslice_min;
                    if (pulsonic.nozzle[pulsonic.numNozzle].accError >= pulsonic.nozzle[pulsonic.numNozzle].kmax_ticks_xtimeslice)
                    {
                        e = pulsonic.nozzle[pulsonic.numNozzle].accError - pulsonic.nozzle[pulsonic.numNozzle].kmax_ticks_xtimeslice;
                        if (e >= 1)
                        {
                            e = e - 1;
                            pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice += 1;
                        }
                        pulsonic.nozzle[pulsonic.numNozzle].accError = e;
                    }
                    if (pulsonic.num_timeslice == ((int) pulsonic.timeslice_min) - 1)
                    {
                        if (e > 0)
                            pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice += 1;
                    }
                    pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice += (uint16_t) pulsonic.nozzle[pulsonic.numNozzle].kmax_ticks_xtimeslice;
                    pulsonic.nozzle[pulsonic.numNozzle].counterTicks_xTotalTime += pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice;
                    //-+

                    pump_setTick(pulsonic.nozzle[pulsonic.numNozzle].nticks_delivered_inThisTimeSlice);
                    autoMode.sm0++;
                }
            }
            else if (autoMode.sm0 == 5)
            {
                if (pump_isIdle())
                {
                    pulsonic.numNozzle++;
                    autoMode.sm0 = 3; //back to test the next enabled nozzle
                }
            }
            
            /* this stage is when the nozzle return to home */
            else if (autoMode.sm0 == 6)
            {
               if (mpap_isIdle())
                {
                    mpap_doMovement(1 * MPAP_NUMSTEP_1NOZZLE, MPAP_CROSSING_HOMESENSOR_MODE);
                    autoMode.sm0++;
               }
            }
            else if (autoMode.sm0 == 7)
            {
                if (mpap_isIdle())
                {
                    if (timeEllapsed >= pulsonic.distAccessTime_ms)
                    {
                        timeEllapsed = 0x00;
                        pulsonic.numNozzle = 0x00; //aqui da por hecho que se completo la vuelta...
                        autoMode.sm0 = 3;
                        //
                        if (++pulsonic.num_timeslice == ((int) pulsonic.timeslice_min))
                        {
                            pulsonic.num_timeslice = 0;
                        }
                    }
                }
            }
        }
    }
}

//___________________________________________

void autoMode_jobX(void)
{
    static uint16_t c_ms;
    static uint16_t c_min;
    if (autoMode.sm0 > 0)
    {
        if (autoMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                mpap_setup_searchFirstPointHomeSensor();
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 2)
        {
            if (mpap_isIdle())
            {
                autoMode.sm0++;
            }
        }
            //Distribute oil 
        else if (autoMode.sm0 == 3)
        {
            if (1)
            {
                mpap_movetoNozzle(pulsonic.numNozzle);
                autoMode.sm0++;
            }

        }
        else if (autoMode.sm0 == 4)
        {
            if (mpap_isIdle())
            {
                pump_setTick(2);
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 5)
        {
            if (pump_isIdle())
            {
                pulsonic.numNozzle++;
                if (pulsonic.numNozzle == NOZZLE_NUMMAX + 1)
                {
                    while (1);

                    if (PinRead(PORTRxSTEPPER_SENSOR_HOME, PINxSTEPPER_SENSOR_HOME) == 0)
                    {
                        //pulsonic.numNozzle = 0x00;
                    }

                }
                autoMode.sm0 = 3;
                c_ms = 0x00;
                c_min = 0x00;
            }
        }

    }
}

/*
void autoMode_jobX(void)
{
    static uint16_t c_ms;
    static uint16_t c_min;
    static int8_t n;
    if (autoMode.sm0 >0)
    {
        if (autoMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                mpap_setupToHomming();
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 2)
        {
            if (mpap_isIdle())
            {
                n = 17;
                autoMode.sm0++;
            }
        }
        //Distribute oil 
        else if (autoMode.sm0 == 3)
        {
            mpap_movetoNozzle(n);
            autoMode.sm0++;
        }
        else if (autoMode.sm0 == 4)
        {
            if (mpap_isIdle())
            {
                pump_setTick(5);
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 5)
        {
            if (pump_isIdle())
            {
                n++;
                autoMode.sm0 = ;
                c_ms = 0x00;
                c_min = 0x00;
            }
        }
        else if (autoMode.sm0 == 6)
        {
            if (smain.f.f1ms)
            {
                if (++c_ms >= 0000)
                {
                    c_ms = 0x0000;
                    //if (++c_min >= 2)
                    {
                        c_min = 0x00;
                        autoMode.sm0 = 0x3;
                    }
                }
            }
        }
    }
}
 */


void autoMode_job_temperature(void)
{

    if (autoMode.sm0 > 0)
    {
        if (autoMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                pulsonic.numNozzle = 0x0;
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 2)
        {
            if (mpap_isIdle())
            {
                pump_setTick(6);
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 3)
        {
            if (pump_isIdle())
            {
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 4)
        {
            pulsonic.numNozzle++;

            if (pulsonic.numNozzle >= NOZZLE_NUMMAX)
            {
                pulsonic.numNozzle = 0x00;
            }
            mpap_movetoNozzle(pulsonic.numNozzle);

            autoMode.sm0++;
        }
        else if (autoMode.sm0 == 5)
        {
            if (mpap_isIdle())
            {
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 6)
        {
            if (smain.f.f1ms)
            {
                if (++pulsonic.countTimeBetweenNozzleAvailable >= pulsonic.distAccessTime_ms)
                {
                    pulsonic.countTimeBetweenNozzleAvailable = 0x0000;
                    autoMode.sm0 = 2;
                    //

                }
            }
        }

    }//if (autoMode.sm0 >0)
}
