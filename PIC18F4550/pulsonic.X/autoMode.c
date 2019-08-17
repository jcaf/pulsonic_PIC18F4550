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

void autoMode_setup(void)
{
    int i;
    double nticksReq_xTotalTime;

    //pulsonic.ml_x1tick = 5.0/268; 
    pulsonic.ml_x1tick = Qmhl_measured / nTicks_measured; 

    //
    pulsonic.dist_total_time = 60; //=60min
    pulsonic.dist_access_time = 1;//2; //c/2min
    pulsonic.timeslice = pulsonic.dist_total_time / pulsonic.dist_access_time; //30
    //added for firmware:
    pulsonic.kTimeBetweenNozzleAvailable = (uint32_t) ((pulsonic.dist_access_time * 60 * 1000) / pulsonic.numNozzleAvailable); //en ms
    //

    for (i = 0; i < NOZZLE_NUMMAX; i++)
    {
        if (nozzle_isEnabled(i))
        {
            //pulsonic.nozzle[i].Q_mlh = 12;//ml
            nticksReq_xTotalTime = pulsonic.nozzle[i].Q_mlh / pulsonic.ml_x1tick;
            pulsonic.nozzle[i].nticks_xtimeslice = nticksReq_xTotalTime / pulsonic.timeslice;

         
            if (pulsonic.nozzle[i].nticks_xtimeslice >= 1)
                {pulsonic.nozzle[i].kmax_ticks_xtimeslice = (uint16_t) pulsonic.nozzle[i].nticks_xtimeslice;}
            
            //added:
            pulsonic.nozzle[i].accError = 0x00;
            
        }
    }
    
    pulsonic.num_timeslice = 0x00;
    pulsonic.countNozzleAvailable = 0x00;
}

static struct _autoMode
{
    int8_t sm0;
} autoMode;

void autoMode_disp7s_writeSumTotal(void)
{
    if (disp_owner == DISPOWNER_AUTOMODE)
    {
        disp7s_modeDisp_off();
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
                    autoMode.sm0+=2;//direct
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
                pulsonic.nozzle[pulsonic.numNozzle].accError += pulsonic.nozzle[pulsonic.numNozzle].nticks_xtimeslice;
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
                if (pulsonic.num_timeslice == ((int) pulsonic.timeslice) - 1)
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
                pulsonic.numNozzle = 0x00;
                //
                //mpap_setupToTurn(1 * MPAP_NUMSTEP_1NOZZLE);
                //mpap_setMode(MPAP_CROSSING_HOMESENSOR_MODE);
                mpap_doMovement(1 * MPAP_NUMSTEP_1NOZZLE, MPAP_CROSSING_HOMESENSOR_MODE);
                
                //
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
                    tacc = (numVueltas * 200) + (  tacc * (PUMP_TICK_TIME_ON+PUMP_TICK_TIME_OFF)); //ms
                    ktime_residuary = pulsonic.kTimeBetweenNozzleAvailable - tacc;
                    pulsonic.countTimeBetweenNozzleAvailable = 0x0000;
                    //
                    if (++pulsonic.countNozzleAvailable >= pulsonic.numNozzleAvailable)
                    {
                        pulsonic.countNozzleAvailable = 0;

                        if (++pulsonic.num_timeslice == ((int) pulsonic.timeslice))
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


void autoMode_jobX(void)
{
    static uint16_t c_ms;
    static uint16_t c_min;
    if (autoMode.sm0 >0)
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
                if (  pulsonic.numNozzle == NOZZLE_NUMMAX+1)
                {
                    while (1);
                    
                    if (PinRead(PORTRxSTEPPER_SENSOR_HOME, PINxSTEPPER_SENSOR_HOME) == 0)
                    {
                        //pulsonic.numNozzle = 0x00;
                    }

                }
                autoMode.sm0 = 3 ;
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
                if (++pulsonic.countTimeBetweenNozzleAvailable >= 10000)
                {
                    pulsonic.countTimeBetweenNozzleAvailable = 0x0000;
                    autoMode.sm0 = 2;
                    //

                }
            }
        }

    }//if (autoMode.sm0 >0)
}
