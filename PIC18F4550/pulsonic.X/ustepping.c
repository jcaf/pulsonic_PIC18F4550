#include "main.h"
#include "ustepping.h"

void setdc(uint16_t dc)
{
    CCP2CON = (uint8_t)  ( ((dc&0x03)<<4)| (CCP2CON & 0xCF));
    CCPR2L = (uint8_t)(dc>>2);
}

/*DC_TOP - (x) is for complement to inverted Enable chip */
const uint16_t ustep_lockup[MICROSTEP_N+1]=
{   DC_TOP -(DC_MIN+(MICROSTEP*0)), //MIN
    DC_TOP -(DC_MIN+(MICROSTEP*1)), 
    DC_TOP -(DC_MIN+(MICROSTEP*2)), 
    DC_TOP -(DC_MIN+(MICROSTEP*3)), 
    DC_TOP -(DC_MIN+(MICROSTEP*4)), //MAX
    //
    //DC_TOP -(DC_MIN+(MICROSTEP*5)), //MAX
    //DC_TOP -(DC_MIN+(MICROSTEP*6)), //MAX
    //DC_TOP -(DC_MIN+(MICROSTEP*7)), //MAX
    //DC_TOP -(DC_MIN+(MICROSTEP*8)), //MAX
};
