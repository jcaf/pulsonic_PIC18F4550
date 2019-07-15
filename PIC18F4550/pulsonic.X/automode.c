#include "main.h"
#include "pulsonic.h"
#include "mpap.h"
#include "pump.h"
#include "automode.h"
#include "visualizerMode.h"
#include "ikb/ikb.h"
static struct _autoMode
{
    int8_t numNozzle;   //current nozzle position
    int8_t sm0;
}autoMode;

static int8_t autoMode_kb(void);

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

int8_t autoMode_job(void)
{
    int8_t cod_ret = 0;
    
    if (smain.focus.kb == FOCUS_KB_AUTOMODE)
    {
        cod_ret = autoMode_kb();
    }
    
    ////////////////////////////////////////////////
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
            autoMode.sm0 = 0x2;
        }
    }
    return cod_ret;
}
/*
 * 1 = return and exit from current process
 */
static int8_t autoMode_kb(void)
{
    int8_t cod_ret = 0;
    
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_UP))
    {
        ikb_key_was_read(KB_LYOUT_KEY_UP);
        //
        visMode_init(0x00);
        cod_ret = 1;
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_DOWN))
    {
        ikb_key_was_read(KB_LYOUT_KEY_DOWN);
        //
        visMode_init(VISMODE_NUMMAX_VISTAS-1);
        cod_ret = 1;
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_PLUS))
    {
        ikb_key_was_read(KB_LYOUT_KEY_PLUS);
        //
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_MINUS))
    {
        ikb_key_was_read(KB_LYOUT_KEY_MINUS);
        //
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_ENTER_F))
    {
cod_ret = 2;        
        ikb_key_was_read(KB_LYOUT_KEY_ENTER_F);
        //
    }
    return cod_ret;
}
