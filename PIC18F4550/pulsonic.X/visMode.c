#include "main.h"
#include "visMode.h"
#include "pulsonic.h"
#include "ikb/ikb.h"

struct _visMode visMode = {-1, 0};

static void visMode_disp(int8_t numVista);

struct _ps ps_visMode;

void  visMode_job(void)
{
    //if (ps_visMode.unlock.ps)
    {

        if (visMode.disp7s_accessReq == 1)
        {
            if (disp_owner == DISPOWNER_VISMODE)       
            {
                visMode_disp(visMode.numVista);
            }
            visMode.disp7s_accessReq = 0;
        }
    }
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

