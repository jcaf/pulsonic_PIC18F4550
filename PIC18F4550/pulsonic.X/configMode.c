#include "main.h"
#include "configMode.h"
#include "pulsonic.h"
#include "ikb/ikb.h"

static struct _configMode
{
    int8_t disp7s_accessReq;//access request
    int8_t numRegistro;        //number of "vista"
}configMode;

static void configMode_disp(int8_t numRegistro);

void configMode_init(int8_t numRegistro)
{
    configMode.numRegistro = numRegistro;
    configMode.disp7s_accessReq = 1;//ingresa mostrando en numRegistro
}
/*
 * proceso
 * keyboard
 * displayer
 */
static int8_t configMode_kb(void);

int8_t configMode_job(void)
{
    int8_t cod_ret = 0;
    
    if (smain.focus.kb == FOCUS_KB_CONFIGMODE)
    {
        cod_ret = configMode_kb();
    }
    
    if ( configMode.disp7s_accessReq == 1)
    {
        if (disp_owner == DISPOWNER_CONFIGMODE)       
        {
            configMode_disp(configMode.numRegistro);
        }
        configMode.disp7s_accessReq = 0;
    }

    return cod_ret;
}
static int8_t configMode_kb(void)
{
    int8_t cod_ret = 0;
    
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_UP))
    {
        ikb_key_was_read(KB_LYOUT_KEY_UP);
        //
        if (++configMode.numRegistro >= CONFIGMODE_NUMMAX_REGISTROS)
        {
            configMode.numRegistro = 0x00;
        }
        configMode.disp7s_accessReq = 1;
        
    }

    if (ikb_key_is_ready2read(KB_LYOUT_KEY_DOWN))
    {
        ikb_key_was_read(KB_LYOUT_KEY_DOWN);
        //
        if (--configMode.numRegistro < 0)
        {
            configMode.numRegistro = CONFIGMODE_NUMMAX_REGISTROS-1;
        }
        configMode.disp7s_accessReq = 1;
    }
    
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_PLUS))
    {
         if (configMode.numRegistro < NOZZLE_NUMMAX)                
         {
             if (pulsonic.nozzle[configMode.numRegistro].Q_mlh == 0)
             {
                 pulsonic.nozzle[configMode.numRegistro].Q_mlh = NOZZLE_QMLH_MIN;
             }
             else
             {
                pulsonic.nozzle[configMode.numRegistro].Q_mlh += NOZZLE_QTY_DIFF;
             }
             //
             if ( pulsonic.nozzle[configMode.numRegistro].Q_mlh >= NOZZLE_QMLH_MAX)
             {
                 pulsonic.nozzle[configMode.numRegistro].Q_mlh = NOZZLE_QMLH_MAX;
             }
             
        }
        else
        {
             if (++pulsonic.oil.i>=OIL_VISCOSITY_NUMMAX)
             {
                 pulsonic.oil.i=0;
             }
             pulsonic.oil.viscosity = OIL_VISCOSITY[pulsonic.oil.i];
        }
        configMode.disp7s_accessReq = 1;
        
        ikb_key_was_read(KB_LYOUT_KEY_PLUS);
        //
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_MINUS))
    {
        if (configMode.numRegistro < NOZZLE_NUMMAX)                
         {
             pulsonic.nozzle[configMode.numRegistro].Q_mlh -= NOZZLE_QTY_DIFF;
             
             if (configMode.numRegistro == 0)//Nozzle "0" cannot be disabled
             {
                if ( pulsonic.nozzle[configMode.numRegistro].Q_mlh < NOZZLE_QMLH_MIN)
                {
                    pulsonic.nozzle[configMode.numRegistro].Q_mlh = NOZZLE_QMLH_MIN;
                }
             }
             else
             {
                if ( pulsonic.nozzle[configMode.numRegistro].Q_mlh < NOZZLE_QMLH_MIN)
                {
                    pulsonic.nozzle[configMode.numRegistro].Q_mlh = NOZZLE_OFF;
                }
             }
        }
        else
        {
             if (--pulsonic.oil.i < 0)
             {
                 pulsonic.oil.i= OIL_VISCOSITY_NUMMAX-1;
             }
             pulsonic.oil.viscosity = OIL_VISCOSITY[pulsonic.oil.i];
        }
        configMode.disp7s_accessReq = 1;
        
        ikb_key_was_read(KB_LYOUT_KEY_MINUS);
        //
    }
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_ENTER_F))
    {
cod_ret = 1;        
        ikb_key_was_read(KB_LYOUT_KEY_ENTER_F);
        //
    }
    return cod_ret;
}
static void configMode_disp(int8_t numRegistro)
{
    double qty;

    if (numRegistro < NOZZLE_NUMMAX)                
    {
        disp7s_modeDisp_writeInt(numRegistro+1);
        
        qty = pulsonic.nozzle[numRegistro].Q_mlh;
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
