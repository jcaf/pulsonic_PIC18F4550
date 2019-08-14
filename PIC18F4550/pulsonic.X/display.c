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
    0x4F,//    0b01001111,//3
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
 q = 4 3  2 1 0
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
    if (q < DISP7S_QTY_NUMMAX)
        {PORTWxDISPLAY = pulsonic.disp7s.qty[q];}
    else
        {PORTWxDISPLAY = pulsonic.disp7s.mode[q-DISP7S_QTY_NUMMAX];}
    
    disp7s_mux_Q(q);
    //
    if (++q == DISP7S_TOTAL_NUMMAX)
        {q=0x0;}
}

void disp7s_off(uint8_t *pDisp, int8_t NUM_OF_DISP)
{
    int8_t i;
    for (i=0; i<NUM_OF_DISP; i++)
        {pDisp[i] = DISP7S_CHARS[OFF];}   
}
void disp7s_modeDisp_off(void)
{
    disp7s_off(pulsonic.disp7s.mode, DISP7S_MODE_NUMMAX);
}
void disp7s_qtyDsp_off(void)
{
    disp7s_off(pulsonic.disp7s.qty, DISP7S_QTY_NUMMAX);
}
void disp7s_modeDisp_writeText_oil(void)
{
   pulsonic.disp7s.mode[1] =0x5C;
   pulsonic.disp7s.mode[0] =0x16;
}
void disp7s_qtyDisp_writeText_OFF(void)
{
   pulsonic.disp7s.qty[2] =0x3F;
   pulsonic.disp7s.qty[1] =0x71;
   pulsonic.disp7s.qty[0] =0x71;
}
void disp7s_qtyDisp_writeText_FFF(void)
{
   pulsonic.disp7s.qty[2] =0x71;
   pulsonic.disp7s.qty[1] =0x71;
   pulsonic.disp7s.qty[0] =0x71;
}
void disp7s_qtyDisp_writeText_FLU(void)
{
   pulsonic.disp7s.qty[2] =0x71;
   pulsonic.disp7s.qty[1] =0x38;
   pulsonic.disp7s.qty[0] =0x3E;
}

void disp7s_qtyDisp_writeText_20_3RAYAS(void)
{
   disp7s_modeDisp_writeInt(20);
   
   pulsonic.disp7s.qty[2] =DISP7S_CHARS[RAYA];
   pulsonic.disp7s.qty[1] =DISP7S_CHARS[RAYA];
   pulsonic.disp7s.qty[0] =DISP7S_CHARS[RAYA];
}
void disp7s_qtyDisp_writeText_NO_OIL(void)
{
    pulsonic.disp7s.mode[1] = 0x54;  //0b01010100;//n
    pulsonic.disp7s.mode[0] = 0x5C;  //0b01011100;//o
    //
    pulsonic.disp7s.qty[2] = DISP7S_NUMS[0];//O
    pulsonic.disp7s.qty[1] = 0x30;//I
    pulsonic.disp7s.qty[0] = 0x38;//L
}
void disp7s_qtyDisp_writeText_NO_HOME_SENSOR(void)
{
    disp7s_modeDisp_writeInt(21);
    //
    pulsonic.disp7s.qty[2] = 0x79;//E
    pulsonic.disp7s.qty[1] = 0x50;//r
    pulsonic.disp7s.qty[0] = 0x50;//r
}

/*
Take 1 decimal
  
if (f<1.0)//0.1.. 0.9 -> 1..9               Case=0.d
   CDU, D=0. ,U=decimal   ,break;     
else if (f<10)//1.0.. 9.9 -> 10->99         Case=I.d
   CDU, D=U. ,U=decimal ,break;     
else if (f<100)//10.0.. 99.9 -> 100->999    Case=II.d
   CDU, C=D D=U. ,U=decimal ,break;     
 else if (f<1000)//100.0.. 999.9 ->          Case=III.d -> III (truncate, no D.P)
   CDU, C=C D=D. ,U=U ,break;     
*/
void disp7s_write_f(uint8_t *pDisp, int8_t NUM_OF_DISP, double num)
{
    double K10MULT;//don't use ^ -> multiples 10
    int16_t I;
	char buff[10];
    int8_t i;
    int8_t pi;//iMAX;
    /*Truncate to max. number representable in NUM_OF_DISP displays*/
    K10MULT = 1;
    i = NUM_OF_DISP;
    while (i--)
        {K10MULT *=10;}
    if (num >= K10MULT)
        {num= K10MULT-1;}
        
    disp7s_off(pDisp, NUM_OF_DISP);
//    for (i=0; i<NUM_OF_DISP; i++)
//        {pDisp[i] = DISP7S_CHARS[OFF];}
    
    K10MULT = 1;
    for (i=0; i<=NUM_OF_DISP; i++)
    {
        if (num< K10MULT)
        {
            if (i<NUM_OF_DISP)//0.d, I.d, II.d 
            {
                I = (int16_t)(num*10);    
                //MSB<--LSB
                //[]  [1.] [0] where 1 is the index for D.P = |0x80; 
                if (i==0)
                    {pDisp[1]= DISP7S_NUMS[0];}
                
                pDisp[1] |= 0x80;   //+ D.P
                pi = i;
                break;
            }
            else    //III.d -> truncate to III and use full digits
            {
                I = (int16_t)(num);    
                pi = i-1;//= NUM_OF_DISP-1
            }
        }
        K10MULT *= 10;
    }
    //
    myitoa(I, buff, 10);

    i=0;
    while (buff[i] != '\0')
    {
        pDisp[pi--] |= DISP7S_NUMS[ buff[i] - 0x30 ];
        i++;
    }
}

void disp7s_qtyDisp_writeFloat(float num)
{
    disp7s_write_f(pulsonic.disp7s.qty, DISP7S_QTY_NUMMAX, num);
}
void disp7s_modeDisp_writeFloat(float num)
{
    disp7s_write_f(pulsonic.disp7s.mode, DISP7S_MODE_NUMMAX, num);
}

void disp7s_write_i(uint8_t *pDisp, int8_t NUM_OF_DISP, int16_t num)
{
    int16_t K10MULT;//don't use ^ -> multiples 10
	char buff[10];
    int8_t i;
    int8_t pi;
    //
    K10MULT = 1;
    i = NUM_OF_DISP;
    while (i--)
        {K10MULT *=10;}
    //     
    if (num >= K10MULT)
        {num= K10MULT-1;}
    
    myitoa(num, buff, 10);
    
    
    for (i=0; i<NUM_OF_DISP; i++)
        {pDisp[i] = DISP7S_CHARS[OFF];}
    //
    i=0;
    pi = strlen(buff)-1;
    while (buff[i] != '\0')
    {
        pDisp[pi--] |= DISP7S_NUMS[ buff[i] - 0x30 ];
        i++;
    }
    //
}
void disp7s_qtyDisp_writeInt(int16_t num)
{
    disp7s_write_i(pulsonic.disp7s.qty, DISP7S_QTY_NUMMAX, num);
}
void disp7s_modeDisp_writeInt(int16_t num)
{
    disp7s_write_i(pulsonic.disp7s.mode, DISP7S_MODE_NUMMAX, num);
}
        