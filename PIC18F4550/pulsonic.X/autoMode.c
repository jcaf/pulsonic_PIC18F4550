#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "autoMode.h"
#include "visMode.h"
#include "ikb/ikb.h"
#include "flushMode.h"

struct _ps ps_autoMode;

static struct _autoMode
{
    int8_t numNozzle;   //current nozzle position
    int8_t sm0;
}autoMode;

//static int8_t autoMode_kb(void);
static codapp_t autoMode_kb(void);

void autoMode_init(int8_t init)
{
    if (disp_owner == DISPOWNER_AUTOMODE)
    {
        disp7s_modeDisp_off();
        disp7s_qtyDisp_writeFloat( pulsonic_getTotalSum_mlh() );
    }
    if (init == AUTOMODE_INIT_RESTART)
    {
        autoMode.numNozzle = 0x0;
        autoMode.sm0 = 0x0;
    }
}

codapp_t autoMode_job(void)
{
    static uint16_t c_ms;
    static uint16_t c_min;
    
    //int8_t cod_ret = 0;
    codapp_t cod={0,0};
    
    if (smain.focus.kb == FOCUS_KB_AUTOMODE)
    //if (ps_autoMode.unlock.kb)
    {
        cod = autoMode_kb();
    }
    
    if (ps_autoMode.unlock.ps)
    {
        if (autoMode.sm0 == 0)
        {
            mpap_setupToHomming();
            autoMode.sm0++;
        }
        else if (autoMode.sm0 == 1)
        {
            if (mpap_isIdle())
            {
                autoMode.sm0++;
            }
        }
        /* Distribute oil */
        else if (autoMode.sm0 == 2)
        {
            if (nozzle_isEnabled(autoMode.numNozzle))
            {
                mpap_movetoNozzle(autoMode.numNozzle);
                autoMode.sm0++;
            }

            if (++autoMode.numNozzle == NOZZLE_NUMMAX)
                {autoMode.numNozzle = 0x00;}
        }
        else if (autoMode.sm0 == 3)
        {
            if (mpap_isIdle())
            {
                pump_setTick(1);
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 4)
        {
            if (pump_isIdle())
            {
                autoMode.sm0++;
            }
        }
        else if (autoMode.sm0 == 5)
        {
            if (smain.f.f1ms)
            {
                if (++c_ms >= 2000)
                {
                    c_ms = 0x0000;
                    //if (++c_min >= 2)
                    {
                        c_min = 0x00;
                        autoMode.sm0 = 0x2;
                    }
                }
            }

        }
    }
    
    return cod;
}
/*
 * 1 = return and exit from current process
 */
//static int8_t 
static codapp_t autoMode_kb(void)
{
    //int8_t cod_ret = 0;
    codapp_t cod={0,0};
    
    int8_t flushKb;
    static int8_t flushKb_last;
    
    flushKb = ikb_key_is_ready2read(KB_LYOUT_KEY_ENTER_F);
    if (flushKb_last != flushKb)
    {
        if (flushKb)
        {
            flushMode_cmd(FLUSH_CMD_RESTART);
        }
        else
        {
            flushMode_cmd(FLUSH_CMD_STOP);
            //
            autoMode_init(AUTOMODE_INIT_RESTART);
        }
        flushKb_last = flushKb;
    }
    
    ////////////////
    ////////////////
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_UP))
    {
        ikb_key_was_read(KB_LYOUT_KEY_UP);
        //
        //visMode_init(0x00);
        cod.param0 = '+';
        cod.ret = 1;
    }
    else if (ikb_key_is_ready2read(KB_LYOUT_KEY_DOWN))
    {
        ikb_key_was_read(KB_LYOUT_KEY_DOWN);
        //
        cod.param0 = '-';
        //visMode_init(VISMODE_NUMMAX_VISTAS-1);
        cod.ret = 1;
    }
    
    //
    if ((ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_PLUS)==KB_AFTER_THR) &&
        ikb_key_is_ready2read(KB_LYOUT_KEY_PLUS) &&
        (ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_MINUS)==KB_AFTER_THR) &&
        ikb_key_is_ready2read(KB_LYOUT_KEY_MINUS))
    {
        ikb_key_was_read(KB_LYOUT_KEY_PLUS);
        ikb_key_was_read(KB_LYOUT_KEY_MINUS);
        //
        cod.ret = 2;     
    }

    return cod;
}
