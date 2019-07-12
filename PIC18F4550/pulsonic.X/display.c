#include "main.h"
#include "display.h"
#include "pulsonic.h"

static inline void disp7s_mux0_off(void)
{
    PinTo1(PORTWxENABLE_DISP_0, PINxENABLE_DISP_0);
}
static inline void disp7s_mux0_on(void)
{
    PinTo0(PORTWxENABLE_DISP_0, PINxENABLE_DISP_0);
}
//
static inline void disp7s_mux1_off(void)
{
    PinTo1(PORTWxENABLE_DISP_1, PINxENABLE_DISP_1);
}
static inline void disp7s_mux1_on(void)
{
    PinTo0(PORTWxENABLE_DISP_1, PINxENABLE_DISP_1);
}
//
static inline void disp7s_mux2_off(void)
{
    PinTo1(PORTWxENABLE_DISP_2, PINxENABLE_DISP_2);
}
static inline void disp7s_mux2_on(void)
{
    PinTo0(PORTWxENABLE_DISP_2, PINxENABLE_DISP_2);
}
//
static inline void disp7s_mux3_off(void)
{
    PinTo1(PORTWxENABLE_DISP_3, PINxENABLE_DISP_3);
}
static inline void disp7s_mux3_on(void)
{
    PinTo0(PORTWxENABLE_DISP_3, PINxENABLE_DISP_3);
}
//
static inline void disp7s_mux4_off(void)
{
    PinTo1(PORTWxENABLE_DISP_4, PINxENABLE_DISP_4);
}
static inline void disp7s_mux4_on(void)
{
    PinTo0(PORTWxENABLE_DISP_4, PINxENABLE_DISP_4);
}

struct _disp7s_mux
{
    PTRFX_retVOID Qonoff[2];//0=..off(), 1=..on()
};

struct _disp7s_mux disp7s_mux[DISP7S_TOTAL_NUMMAX] = 
{
    {disp7s_mux0_off, disp7s_mux0_on},
    {disp7s_mux1_off, disp7s_mux1_on},
    {disp7s_mux2_off, disp7s_mux2_on},
    {disp7s_mux3_off, disp7s_mux3_on},
    {disp7s_mux4_off, disp7s_mux4_on},
};

const uint8_t DISP7S_NUMS[10] = 
{
    0x3F,//    0b00111111,//0
    0x06,//    0b00000110,//1
    0x5B,//    0b01011011,//2
    0x8F,//    0b01001111,//3
    0x66,//    0b01100110,//4
    0x6D,//    0b01101101,//5
    0x7C,//    0b01111100,//6
    0x07,//    0b00000111,//7
    0x7F,//    0b01111111,//8
    0x67,//    0b01100111,//9
};
const uint8_t DISP7S_CHARS[2] = 
{
    0x00,   //0b00000000,//OFF
    0x40,   //0b01000000,//- raya
};

static void disp7s_mux_init(void)
{
    int8_t i;
    for (i=0; i< DISP7S_TOTAL_NUMMAX; i++)
    {
        disp7s_mux[i].Qonoff[0]();//off
    }
    ConfigOutputPin(CONFIGIOxENABLE_DISP_0, PINxENABLE_DISP_0);
    ConfigOutputPin(CONFIGIOxENABLE_DISP_1, PINxENABLE_DISP_1);
    ConfigOutputPin(CONFIGIOxENABLE_DISP_2, PINxENABLE_DISP_2);
    ConfigOutputPin(CONFIGIOxENABLE_DISP_3, PINxENABLE_DISP_3);
    ConfigOutputPin(CONFIGIOxENABLE_DISP_4, PINxENABLE_DISP_4);
}
//mostrar en los displays [][] - [][][]
static void disp7s_mux_Q(int8_t q)//q=transistor #      
{
    int8_t i;
    for (i=0; i< DISP7S_TOTAL_NUMMAX; i++)
    {
        if ( (1<<i) & (1<<q) )
            disp7s_mux[i].Qonoff[1]();//on
        else
            disp7s_mux[i].Qonoff[0]();//off
    }
}

/*MSB LSB
 * 0 1 ...
 */
void disp7s_init(void)
{
    int8_t i;
    for (i=0; i<DISP7S_MODE_NUMMAX; i++)
        {pulsonic.disp7s.mode[i] = DISP7S_CHARS[OFF];}

    for (i=0; i<DISP7S_QTY_NUMMAX; i++)
        {pulsonic.disp7s.qty[i] = DISP7S_CHARS[OFF];}
    //
    CONFIGIOxDISPLAY = 0x00;//TRISB outputs
    disp7s_mux_init();
}
/*
 q = 0 1  2 3 4
 *   [][]-[][][]
 */
void disp7s_job(void)
{
    static int8_t q;//q=transistor #
    
    //+-off current Q before to present new digit
    int8_t pq=0;
    pq= q-1;
    if (pq<0)
        {pq=DISP7S_TOTAL_NUMMAX-1;}
    disp7s_mux[pq].Qonoff[0]();//off
    //-+
    if (q < DISP7S_MODE_NUMMAX)
        {PORTWxDISPLAY = pulsonic.disp7s.mode[q];}
    else
        {PORTWxDISPLAY = pulsonic.disp7s.qty[q-DISP7S_MODE_NUMMAX];}
    
    disp7s_mux_Q(q);
    //
    if (++q == DISP7S_TOTAL_NUMMAX)
        {q=0x0;}
}

void disp7s_write_f(uint8_t *pDisp, int8_t NUM_OF_DISP, double num)
{
    double K10MULT;//don't use ^ -> multiples 10
    int16_t I;
	char buff[10];
    int8_t i;
    int8_t iMAX;
    int8_t iMIN;

    K10MULT = 1;
    i = NUM_OF_DISP;
    while (i--)
        {K10MULT *=10;}
    //     
    if (num >= K10MULT)
        {num= K10MULT-1;}
    
    for (i=0; i<NUM_OF_DISP; i++)
        {pDisp[i] = DISP7S_CHARS[OFF];}
    
    /*//solo se toma 1 decimal
    if (f<1.0)//0.1.. 0.9 -> 1..9
        CDU, D=0. ,U=decimal        
    if (f<10)//1.0.. 9.9 -> 10->99
        CDU, D=U. ,U=decimal
    if (f<100)//10.0.. 99.9 -> 100->999
        CDU, C=D D=U. ,U=decimal
    if (f<1000)//100.0.. 999.9 -> se trunca solo a los 3 primeros digitos, no se muestra el decimal
        CDU, C=C D=D. ,U=U
    */

    K10MULT = 1;
    for (i=0; i<=NUM_OF_DISP; i++)//0 1 2
    {
        if (num< K10MULT)
        {
            if (i<NUM_OF_DISP)
            {
                I = (int16_t)(num*10);    
                if (i==0)
                {
                    pDisp[NUM_OF_DISP-2]= DISP7S_NUMS[0];/*NUM_OF_DISP-2 = posc.del indice para el D.P*/
                }
                pDisp[NUM_OF_DISP-2] |= 0x80;   //+ D.P
                iMAX = i+1;
                iMIN = (NUM_OF_DISP-1)-i;
                break;
            }
            else
            {
                I = (int16_t)(num);    
                iMAX = i;
                iMIN = 0;
            }
        }
        K10MULT *= 10;
    }
    //
    myitoa(I, buff, 10);

    for (i=0; i<iMAX; i++)
        {pDisp[iMIN++] |= DISP7S_NUMS[ buff[i] - 0x30 ];}
}
void disp7s_qtyDisp_writeFloat(float f)
{
    disp7s_write_f(pulsonic.disp7s.qty, DISP7S_QTY_NUMMAX, f);
}
void disp7s_modeDisp_writeFloat(float f)
{
    disp7s_write_f(pulsonic.disp7s.mode, DISP7S_MODE_NUMMAX, f);
}

/*
void disp_show_quantity(double f)
{
	int16_t I;
	char buff[10];
    uint8_t num;
    int8_t i;
    int8_t last_pos;
    int8_t q;

    if (f>= 1000.0f)
        {f=999.0f;}
    
    pulsonic.display7s[QUANT_DIG_2] = DISP7S_CHARS[OFF];
    pulsonic.display7s[QUANT_DIG_1] = DISP7S_CHARS[OFF];
    pulsonic.display7s[QUANT_DIG_0] = DISP7S_CHARS[OFF];
    
    if (f < 100.0f)         //si menor a 100, por ejmp 99.9, se puede representar en el display
    {                       //99.9X * 10 -> 999.X, con esto obtengo el decimal tal y cual es,
        I = (int16_t)(f*10);    //la multip. trunca al ser un entero I //si es un 101.6 -> quedo truncado I y se muestra solo los 3 digitos enteros
        myitoa(I, buff, 10);
        //
        if (I>99)
            {last_pos = 2;}
        else if (I>9)
            {last_pos = 1;}
        else 
        {
            pulsonic.display7s[QUANT_DIG_1] = DISP7S_NUMS[0];
            last_pos = 0;
        }
        q = 0;
        for (i=last_pos; i>=0; i--)
        {
            num = buff[i] - 0x30;
            pulsonic.display7s[q++] = DISP7S_NUMS[num];
        }
        pulsonic.display7s[QUANT_DIG_1] |= 0x80;    //add . decimalpoint
    }                       
    else
    {
        I = (int16_t)f; //mostrar solo los 3 digitos enteros
        myitoa(I, buff, 10);
        //
        q = 0;
        last_pos = 2;
        for (i=last_pos; i>=0; i--)
        {
            num = buff[i] - 0x30;
            pulsonic.display7s[q++] = DISP7S_NUMS[num];
        }
    }
}
*/
/*
void disp_show_quantity(double f)//no presenta el decimal si es .0
{
	int16_t I;
	char buff[10];
    uint8_t num;
    int8_t i;
    int8_t last_pos;
    int8_t q;
    double decimal;

    if (f>= 1000.0f)
        f=999.0f;//truncar al max. valor 
    //    
	I = (int16_t)f;//parte entera
	decimal = f-I;
    if (decimal > 0.0f)//tiene un decimal diferente de 0
    {
        if (f < 100.0f)//si menor a 100, por ejmp 99.9, se puede representar en el display
        {
            I = (int)(f*10); //99.9 * 10 -> 999, con esto obtengo el decimal tal y cual es,
        }             //la multip. trunca al ser un entero I
                        //si es un 101.6 -> quedo truncado I y se muestra solo los 3 digitos enteros
    }
    myitoa(I, buff, 10);
    //
    pulsonic.display7s[QUANT_DIG_2] = DISP7S_CHARS[OFF];
    pulsonic.display7s[QUANT_DIG_1] = DISP7S_CHARS[OFF];
    pulsonic.display7s[QUANT_DIG_0] = DISP7S_CHARS[OFF];
	//
    if (decimal<0.01)//no existe parte decimal
	{
		if (I>99)
			last_pos = 2;
		else if (I>9)
            last_pos = 1;
		else 
            last_pos = 0;
        
        q = 0;
        for (i=last_pos; i>=0; i--)
        {
            num = buff[i] - 0x30;
            pulsonic.display7s[q++] = DISP7S_NUMS[num];
        }
	}
	else
	{
        if (f<100.0)//
        {
            //I = f*10; //1.8
            //myitoa(I, buff, 10);
            
            if (I>99)
                {last_pos = 2;}
            else if (I>9)
                {last_pos = 1;}
            else 
            {
                pulsonic.display7s[QUANT_DIG_1] = DISP7S_NUMS[0];
                last_pos = 0;
            }
            
            q = 0;
            for (i=last_pos; i>=0; i--)
            {
                num = buff[i] - 0x30;
                pulsonic.display7s[q++] = DISP7S_NUMS[num];
            }
            pulsonic.display7s[QUANT_DIG_1] |= 0x80;
        }
        else//mostrar solo los 3 digitos enteros
        {
            q = 0;
            last_pos = 2;
            for (i=last_pos; i>=0; i--)
            {
                num = buff[i] - 0x30;
                pulsonic.display7s[q++] = DISP7S_NUMS[num];
            }
        }
	}
}

 */