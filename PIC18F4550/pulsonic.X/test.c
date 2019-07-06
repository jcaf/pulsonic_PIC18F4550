#include "main.h"
#include "test.h"
#include "kk.h"

static uint8_t keyRead_pin_key1(void)
{
//	PinTo0(PORTWxKBFIL_1, PINxKBFIL_1);
//	PinTo1(PORTWxKBFIL_2, PINxKBFIL_2);
//	PinTo1(PORTWxKBFIL_3, PINxKBFIL_3);
//	PinTo1(PORTWxKBFIL_4, PINxKBFIL_4);
//	readkey_setup_time();
	return ReadPin(PORTRxKBFIL_KEY1, 0);
}
const int KKB_NUM_KEYS = 4;

volatile struct _key key[KKB_NUM_KEYS];

void kb_initx(void)
{
    ConfigOutputPin(CONFIGIOxKBFIL_KEY1, 0);
    key[0].keyRead = keyRead_pin_key1;
    //
    ikey_scanx();
}

    