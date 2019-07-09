#include "main.h"
#include "system.h"
#include "pulsonic.h"
#include "types.h"
struct _pulsonic pulsonic;

void pulsonic_init(void)
{
    pulsonic.dist_total_time = 60;  //min
    /*save and restore from EEPROM*/
}
uint16_t get_total_sum_mlh(void)
{
    int i;
    uint16_t acc = 0;
    for (i=0; i<NOZZLE_NUMMAX; i++)
    {
        acc += pulsonic.nozzle[i].Q_mlh;
    }
    return acc;
}

//DISLAYS DE CATODO COMUN - Todo el puerto de 8bits
#define PORTWxDISPLAY 	LATB
#define PORTRxDISPLAY 	PORTB
#define CONFIGIOxDISPLAY 	TRISB
//#define PINxDISPLAY 		0

//TRANSISTORES PARA EL BARRIDO/MULTIPLEXING 
#define PORTWxENABLE_DISP_0 	LATA
#define PORTRxENABLE_DISP_0 	PORTA
#define CONFIGIOxENABLE_DISP_0 	TRISA
#define PINxENABLE_DISP_0 		4

#define PORTWxENABLE_DISP_1 	LATA
#define PORTRxENABLE_DISP_1 	PORTA
#define CONFIGIOxENABLE_DISP_1 	TRISA
#define PINxENABLE_DISP_1 		3

#define PORTWxENABLE_DISP_2 	LATA
#define PORTRxENABLE_DISP_2 	PORTA
#define CONFIGIOxENABLE_DISP_2 	TRISA
#define PINxENABLE_DISP_2 		2

#define PORTWxENABLE_DISP_3 	LATA
#define PORTRxENABLE_DISP_3 	PORTA
#define CONFIGIOxENABLE_DISP_3 	TRISA
#define PINxENABLE_DISP_3 		1

#define PORTWxENABLE_DISP_4 	LATA
#define PORTRxENABLE_DISP_4 	PORTA
#define CONFIGIOxENABLE_DISP_4 	TRISA
#define PINxENABLE_DISP_4 		0

static inline void multiplexedDisp0_off(void)
{
    PinTo1(PORTWxENABLE_DISP_0, PINxENABLE_DISP_0);
}
static inline void multiplexedDisp0_on(void)
{
    PinTo0(PORTWxENABLE_DISP_0, PINxENABLE_DISP_0);
}
//
static inline void multiplexedDisp1_off(void)
{
    PinTo1(PORTWxENABLE_DISP_1, PINxENABLE_DISP_1);
}
static inline void multiplexedDisp1_on(void)
{
    PinTo0(PORTWxENABLE_DISP_1, PINxENABLE_DISP_1);
}
//
static inline void multiplexedDisp2_off(void)
{
    PinTo1(PORTWxENABLE_DISP_2, PINxENABLE_DISP_2);
}
static inline void multiplexedDisp2_on(void)
{
    PinTo0(PORTWxENABLE_DISP_2, PINxENABLE_DISP_2);
}
//
static inline void multiplexedDisp3_off(void)
{
    PinTo1(PORTWxENABLE_DISP_3, PINxENABLE_DISP_3);
}
static inline void multiplexedDisp3_on(void)
{
    PinTo0(PORTWxENABLE_DISP_3, PINxENABLE_DISP_3);
}
//
static inline void multiplexedDisp4_off(void)
{
    PinTo1(PORTWxENABLE_DISP_4, PINxENABLE_DISP_4);
}
static inline void multiplexedDisp4_on(void)
{
    PinTo0(PORTWxENABLE_DISP_4, PINxENABLE_DISP_4);
}
//fijo
struct _multiplexedDisp multiplexedDisp[DISP_TOTAL_NUMMAX] = 
{
    {multiplexedDisp0_off, multiplexedDisp0_on},
    {multiplexedDisp1_off, multiplexedDisp1_on},
    {multiplexedDisp2_off, multiplexedDisp2_on},
    {multiplexedDisp3_off, multiplexedDisp3_on},
    {multiplexedDisp4_off, multiplexedDisp4_on},
};
extern struct _multiplexedDisp multiplexedDisp[DISP_TOTAL_NUMMAX];

//
const uint8_t DISP7S_NUMS[10] = 
{
    0b00111111,//0
    0b00000110,//1
    0b01011011,//2
    0b01001111,//3
    0b01100110,//4
    0b01101101,//5
    0b01111100,//6
    0b00000111,//7
    0b01111111,//8
    0b01100111,//9
};
const uint8_t DISP7S_CHARS[2] = 
{
    0b00000000,//OFF
    0b01000000,//- raya
};
enum _DISP7S_CHARS {
    OFF,
    RAYA
};

void multiplexedDisp_init(void)
{
    int8_t i;
    for (i=0; i< DISP_TOTAL_NUMMAX; i++)
    {
        multiplexedDisp[i].Qonoff[0]();//off
    }
    //TRISA = 0;
    ConfigOutputPin(CONFIGIOxENABLE_DISP_0, PINxENABLE_DISP_0);
    ConfigOutputPin(CONFIGIOxENABLE_DISP_1, PINxENABLE_DISP_1);
    ConfigOutputPin(CONFIGIOxENABLE_DISP_2, PINxENABLE_DISP_2);
    ConfigOutputPin(CONFIGIOxENABLE_DISP_3, PINxENABLE_DISP_3);
    ConfigOutputPin(CONFIGIOxENABLE_DISP_4, PINxENABLE_DISP_4);
}
//mostrar en los displays [][] - [][][]
void multiplexedDisp_job(int8_t q)        
{
    int8_t i;
    for (i=0; i< DISP_TOTAL_NUMMAX; i++)
    {
        if ((0x01<<i) & (0x1<<q))
            multiplexedDisp[i].Qonoff[1]();//on
        else
            multiplexedDisp[i].Qonoff[0]();//off
    }
}
/*
 * MODE:2 DIGITOS         QUANTITY ml/h: 3 DIGITOS
[MODE_DIG_1][MODE_DIG_0]  [QUANT_DIG_2][QUANT_DIG_1][QUANT_DIG_0]
 */

void display7s_init(void)
{
    pulsonic.display7s[MODE_DIG_1] = DISP7S_NUMS[2];
    pulsonic.display7s[MODE_DIG_0] = DISP7S_NUMS[0];
    //
    pulsonic.display7s[QUANT_DIG_2] = DISP7S_CHARS[RAYA];
    pulsonic.display7s[QUANT_DIG_1] = DISP7S_CHARS[RAYA];
    pulsonic.display7s[QUANT_DIG_0] = DISP7S_CHARS[RAYA];
    //
    CONFIGIOxDISPLAY = 0x00;//TRISB outputs
    multiplexedDisp_init();
}

void display7s_job(void)
{
    static int8_t q;
    //
    PORTWxDISPLAY = pulsonic.display7s[q];
    multiplexedDisp_job(q);
    //
    if (++q == DISP_TOTAL_NUMMAX)
        {q=0;}
}
//////////////////////////////////////////////
//////////////////////////////////////////////
void disp_show_quantity(double f)
{
	int16_t I;
	char buff[10];
    uint8_t num;
    int8_t i;
    int8_t last_pos;
    int8_t q;
    double real;

    if (f>= 1000.0f)
        f=999.0f;//truncar al max. valor 
    //    
	I = (int16_t)f;//parte entera
	real = f-I;
    if (real > 0.0f)//tiene un decimal diferente de 0
    {
        if (f<100.0f)//si menor a 100, por ejmp 99.9, se puede representar en el display
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
    if (real<0.01)//no existe parte real
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