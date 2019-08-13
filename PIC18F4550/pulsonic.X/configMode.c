#include "main.h"
#include "configMode.h"
#include "pulsonic.h"
#include "ikb/ikb.h"
#include "myeeprom.h"
#include "PIC/eeprom/eeprom.h"
#include "flushAtNozzle.h"
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

static int8_t configMode_kb(void);

int8_t configMode_job(void)
{
    int8_t cod_ret = 0;
    
    cod_ret = configMode_kb();
    
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
    static int8_t flushAtNozzle_active;
    struct _key_prop prop = {0};
    
    if (!flushAtNozzle_active)
    {
        if (ikb_key_is_ready2read(KB_LYOUT_KEY_UP))
        {
            //ikb_key_was_read(KB_LYOUT_KEY_UP);
            //
            if (++configMode.numRegistro >= CONFIGMODE_NUMMAX_REGISTROS)
            {
                configMode.numRegistro = 0x00;
            }
            configMode.disp7s_accessReq = 1;

        }
        else if (ikb_key_is_ready2read(KB_LYOUT_KEY_DOWN))
        {
            //ikb_key_was_read(KB_LYOUT_KEY_DOWN);
            //
            if (--configMode.numRegistro < 0)
            {
                configMode.numRegistro = CONFIGMODE_NUMMAX_REGISTROS-1;
            }
            configMode.disp7s_accessReq = 1;
        }
    }
        ikb_key_was_read(KB_LYOUT_KEY_UP);
        ikb_key_was_read(KB_LYOUT_KEY_DOWN);
    

    if ((ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_PLUS)==KB_BEFORE_THR) &&
        ikb_key_is_ready2read(KB_LYOUT_KEY_PLUS))
    {
        if (!flushAtNozzle_active)
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
        }

        ikb_key_was_read(KB_LYOUT_KEY_PLUS);
        //
    }

    if ((ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_MINUS)==KB_BEFORE_THR) &&
        ikb_key_is_ready2read(KB_LYOUT_KEY_MINUS))
    {
        if (!flushAtNozzle_active)
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
        }

        ikb_key_was_read(KB_LYOUT_KEY_MINUS);
        //
    }
    
    if (ikb_key_is_ready2read(KB_LYOUT_KEY_FLUSHENTER))
    {
        ikb_key_was_read(KB_LYOUT_KEY_FLUSHENTER);
        
        if (configMode.numRegistro < NOZZLE_NUMMAX)
        {
            flushAtNozzle_active = !flushAtNozzle_active;

            if (flushAtNozzle_active)
            {
                flushAtNozzle_setNozzle(configMode.numRegistro);
                flushAtNozzle_cmd(JOB_RESTART);
            }
            else
            {
                configMode.disp7s_accessReq = 1;//re-imprime
                flushAtNozzle_cmd(JOB_STOP);
            }
        }
    }
    
    //+--------
    //
    if ((ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_PLUS)==KB_AFTER_THR) &&
        ikb_key_is_ready2read(KB_LYOUT_KEY_PLUS) &&
        (ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_KEY_MINUS)==KB_AFTER_THR) &&
        ikb_key_is_ready2read(KB_LYOUT_KEY_MINUS))
    {
        ikb_key_was_read(KB_LYOUT_KEY_PLUS);
        ikb_key_was_read(KB_LYOUT_KEY_MINUS);
                //
        
        /* Update EEPROM*/
        uint8_t reg;    
        for (reg=0; reg<NOZZLE_NUMMAX; reg++)
        {
            eepromWrite_double(&(((double*)EEPROM_BLOCK_ADDR)[reg]), pulsonic.nozzle[reg].Q_mlh);
        }
        //next address is for index-of-OIL_VISCOSITY[]
        eepromWrite(EEPROM_BLOCK_ADDR + (NOZZLE_NUMMAX*sizeof(double)), pulsonic.oil.i);
        //
        //added
        pulsonic.numNozzleAvailable = pulsonic_getNumNozzleAvailable();//reinicia una nueva cuenta
        //autoMode_setup();//->autoMode_cmd(JOB_RESTART)) alli se hace la llamada
        //
        flushAtNozzle_active = 0;
        flushAtNozzle_cmd(JOB_STOP);
        
        /*change layout for FLush/Enter key*/
        prop = propEmpty;
        prop.uFlag.f.whilePressing = 1;
        ikb_setKeyProp(KB_LYOUT_KEY_FLUSHENTER, prop);
        
        cod_ret = 1;//exit 
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
