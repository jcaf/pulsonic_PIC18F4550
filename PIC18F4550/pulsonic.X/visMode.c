#include "main.h"
#include "visMode.h"
#include "pulsonic.h"
#include "ikb/ikb.h"

static struct _visMode
{
    int8_t disp7s_accessReq;//access request
    int8_t numVista;        //number of "vista"
}visMode;

static void visMode_disp(int8_t numVista);

void visMode_init(int8_t numVista)
{
    visMode.numVista = numVista;
    visMode.disp7s_accessReq = 1;//ingresa mostrando en numVista
}
/*
 * proceso
 * keyboard
 * displayer
 */
static int8_t visMode_kb(void);

int8_t visMode_job(void)
{
    int8_t cod_ret = 0;
    
    if (smain.focus.kb == FOCUS_KB_VISMODE)
    {
        cod_ret = visMode_kb();
    }
    
    if ( visMode.disp7s_accessReq == 1)
    {
        if (disp_owner == DISPOWNER_VISMODE)       
        {
            visMode_disp(visMode.numVista);
        }
        visMode.disp7s_accessReq = 0;
    }

    return cod_ret;
}
static int8_t visMode_kb(void)
{
    int8_t cod_ret = 0;
    
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_UP))
    {
        ikb_key_was_read(KB_LYOUT_KEY_UP);
        //
        if (++visMode.numVista >= VISMODE_NUMMAX_VISTAS)
        {
            visMode.numVista = 0x00;
            cod_ret = 1;
        }
        else
        {
           visMode.disp7s_accessReq = 1;
        }
    }

    if (ikb_key_is_ready2read(KB_LYOUT_KEY_DOWN))
    {
        ikb_key_was_read(KB_LYOUT_KEY_DOWN);
        //
        if (--visMode.numVista < 0)
        {
            visMode.numVista = VISMODE_NUMMAX_VISTAS-1;
            cod_ret = 1;
        }
        else
        {
            visMode.disp7s_accessReq = 1;
        }
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
        ikb_key_was_read(KB_LYOUT_KEY_ENTER_F);
        //
    }
    return cod_ret;
}
static void visMode_disp(int8_t numVista)
{
    double qty;

    if (numVista < NOZZLE_NUMMAX)                
    {
        disp7s_modeDisp_writeInt(numVista+1);
        
        qty = pulsonic.nozzle[numVista].Q_mlh;
        if (qty == 0)
        {
            disp7s_qtyDisp_writeText_OFF();
        }
        else
        {
            disp7s_qtyDisp_writeFloat( qty );
        }
    }
    else
    {
        disp7s_modeDisp_writeText_oil();
        disp7s_qtyDisp_writeInt( pulsonic.oil.viscosity );
    }

}

