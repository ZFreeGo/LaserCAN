#ifndef _DEVICEIO_H_
#define _DEVICEIO_H_
#include "tydef.h"


#define  LEDA LATBbits.LATB0
#define  LEDB LATBbits.LATB1
#define  LEDC LATBbits.LATB2

void InitDeviceIO(void);
void Delay_ms(uint16_t ms);


#endif