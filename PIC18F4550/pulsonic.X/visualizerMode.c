#include "main.h"
#include "pulsonic.h"
#include "ikb/ikb.h"

static struct _visMode
{
    int8_t disp7s_req;//request
    int8_t c;
}visMode;

static void visMode_disp(int8_t c);

void visMode_init(void)
{
    visMode.c = 0x00;
    visMode.disp7s_req = 1;
}

int8_t visMode_job(void)
{
    int8_t cod_ret = 0;
    
    //1)process
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_UP))
    {
        if (++visMode.c>NOZZLE_NUMMAX)
        {
            visMode.c = 0x00;
            cod_ret = 1;
        }
        else
        {
           visMode.disp7s_req = 1;
        }
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_DOWN))
    {
        if (--visMode.c < 0)
        {
            visMode.c = NOZZLE_NUMMAX;
            cod_ret = 1;
        }
        else
        {
            visMode.disp7s_req = 1;
        }
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_PLUS))
    {
        
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_MINUS))
    {
        
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_ENTER_F))
    {
        
    }
    
    //2)display
    if ( visMode.disp7s_req == 1)
    {
        visMode_disp(visMode.c);
        visMode.disp7s_req = 0;
    }

    return cod_ret;
}
static void visMode_disp(int8_t c)
{
    double qty;
    
    //2) display
    if (disp_owner == DISPOWNER_VISUALIZER_MODE)       
    {
        disp7s_modeDisp_writeInt(c+1);
        
        if (c < NOZZLE_NUMMAX)                
        {
            qty = pulsonic.nozzle[c].Q_mlh;
            if (qty == 0)
            {
                disp7s_qtyDisp_writeFloat( qty );//disp OFF
            }
            else
            {
                disp7s_qtyDisp_writeFloat( qty );
            }
        }
        else
        {
            disp7s_qtyDisp_writeInt( pulsonic.oil.viscosity );
        }

    }
}

