#include "sw1.h"
#include "p24FJ64GA002.h"

void initSW1()
{
    TRISBbits.TRISB5=INPUT;
    IFS1bits.CNIF=0;    //CN flag is put down.
    IEC1bits.CNIE=1;    //global CN enable is turned on.
    CNEN2bits.CN27IE=1; //CN enable for switch turned on.
}
