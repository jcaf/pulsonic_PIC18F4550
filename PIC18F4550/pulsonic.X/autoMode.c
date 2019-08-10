#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "autoMode.h"
#include "visMode.h"
#include "ikb/ikb.h"
#include "flushAllMode.h"

/*
 * Experimental
Con Ronald calculamos que 300 ticks = 10 ml
*/
//    double Qmhl_measured = 10;//ml
//    double nTicks_measured = 300;//#ticks
double Qmhl_measured = 5.0;//ml
double nTicks_measured = 231;//#ticks
    
void autoMode_setup(void)
{
    int i;
    double nticksReq_xTotalTime;

    //pulsonic.ml_x1tick = 5.0/231; //5ml/231 ticks 0.021645ml/tick
    pulsonic.ml_x1tick = Qmhl_measured/nTicks_measured; //con Ronald
    
    //
    pulsonic.dist_total_time = 60;//=60min
    pulsonic.dist_access_time = 2;//c/2min
    pulsonic.timeslice = pulsonic.dist_total_time/pulsonic.dist_access_time;//30
    //added for firmware:
    pulsonic.kTimeBetweenNozzleAvailable = (uint16_t) ( (pulsonic.dist_access_time* 1000)/ pulsonic.numNozzleAvailable);
    pulsonic.countTimeBetweenNozzleAvailable = 0x0000;
    //
    
    for (i=0; i<NOZZLE_NUMMAX; i++)
    {
        if (nozzle_isEnabled(i))
        {
                                                                                    //pulsonic.nozzle[i].Q_mlh = 12;//ml
            nticksReq_xTotalTime = pulsonic.nozzle[i].Q_mlh / pulsonic.ml_x1tick;
            pulsonic.nozzle[i].nticks_xtimeslice = nticksReq_xTotalTime / pulsonic.timeslice;

            if (pulsonic.nozzle[i].nticks_xtimeslice <1)
                {pulsonic.nozzle[i].kmax_ticks_xtimeslice = 1;}
            else
                {pulsonic.nozzle[i].kmax_ticks_xtimeslice = (uint16_t)pulsonic.nozzle[i].nticks_xtimeslice;}//trunca
        }
        
    }
    
    
}
static struct _autoMode
{
    int8_t numNozzle;   //current nozzle position
    int8_t sm0;
}autoMode;

void autoMode_disp7s_writeSumTotal(void)
{
    if (disp_owner == DISPOWNER_AUTOMODE)
    {
        disp7s_modeDisp_off();
        disp7s_qtyDisp_writeFloat( pulsonic_getTotalSum_mlh() );
    }
}
void autoMode_cmd(int8_t cmd)
{
    if (cmd == JOB_RESTART)
    {
        autoMode.numNozzle = 0x0;
        autoMode.sm0 = 0x1;
    }
    else if (cmd == JOB_STOP)
    {
        autoMode.sm0 = 0;
    }
    pump_stop();
    mpap.mode = MPAP_STALL_MODE;
}
/*
void autoMode_job(void)
{
    static int16_t num_timeslice;
    static int8_t countNozzleAvailable;
    //
    double e = 0;

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
                autoMode.sm0++;
            }
        }
        //Distribute oil
        else if (autoMode.sm0 == 3)
        {
            if (nozzle_isEnabled(autoMode.numNozzle))
            {
                mpap_movetoNozzle(autoMode.numNozzle);
                autoMode.sm0++;
            }
            //
            if (++autoMode.numNozzle == NOZZLE_NUMMAX)
                {autoMode.numNozzle = 0x00;}
        }
        else if (autoMode.sm0 == 4)
        {
            if (mpap_isIdle())
            {
                //+-
                pulsonic.nozzle[autoMode.numNozzle].nticks_delivered_inThisTimeSlice = 0;
                pulsonic.nozzle[autoMode.numNozzle].accError += pulsonic.nozzle[0].nticks_xtimeslice;
                if (pulsonic.nozzle[autoMode.numNozzle].accError >= pulsonic.nozzle[0].kmax_ticks_xtimeslice)
                {
                    e = pulsonic.nozzle[autoMode.numNozzle].accError - pulsonic.nozzle[0].kmax_ticks_xtimeslice;
                    if ( e>=1)
                    {
                        e = e - 1;
                        pulsonic.nozzle[autoMode.numNozzle].nticks_delivered_inThisTimeSlice+= 1;
                    }
                    pulsonic.nozzle[autoMode.numNozzle].accError = e;
                }
                if ( num_timeslice == ((int)pulsonic.timeslice)-1)
                {
                    if (e>0)
                        pulsonic.nozzle[autoMode.numNozzle].nticks_delivered_inThisTimeSlice+= 1;
                }
                pulsonic.nozzle[autoMode.numNozzle].nticks_delivered_inThisTimeSlice +=(uint16_t)pulsonic.nozzle[autoMode.numNozzle].kmax_ticks_xtimeslice;
                pulsonic.nozzle[autoMode.numNozzle].counterTicks_xTotalTime += pulsonic.nozzle[autoMode.numNozzle].nticks_delivered_inThisTimeSlice;
                //-+
                
                pump_setTick( pulsonic.nozzle[autoMode.numNozzle].nticks_delivered_inThisTimeSlice );
                autoMode.sm0++;
                
                //desde este momento se empieza a tomar el tiempo
                pulsonic.countTimeBetweenNozzleAvailable = 0x0000;
            }
        }
        else if (autoMode.sm0 == 5)
        {
            if (pump_isIdle())
            {
            }
            
            if (smain.f.f1ms)
            {
                if (++pulsonic.countTimeBetweenNozzleAvailable >= pulsonic.kTimeBetweenNozzleAvailable)
                {
                    pulsonic.countTimeBetweenNozzleAvailable = 0;
                    autoMode.sm0 = 3;
                    //
                    if (++countNozzleAvailable >= pulsonic.numNozzleAvailable )
                    {
                        countNozzleAvailable = 0;
                        
                        
                        if (++num_timeslice == ((int)pulsonic.timeslice))
                        {
                            num_timeslice = 0;
                        }
                     }
                }
            }
        }
        
    }//if (autoMode.sm0 >0)
}
*/
void autoMode_job(void)
{
    static uint16_t c_ms;
    static uint16_t c_min;

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
                autoMode.sm0++;
            }
        }
        /* Distribute oil */
        else if (autoMode.sm0 == 3)
        {
            if (nozzle_isEnabled(autoMode.numNozzle))
            {
                mpap_movetoNozzle(autoMode.numNozzle);
                autoMode.sm0++;
            }

            if (++autoMode.numNozzle == NOZZLE_NUMMAX)
                {autoMode.numNozzle = 0x00;}
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
                autoMode.sm0++;
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