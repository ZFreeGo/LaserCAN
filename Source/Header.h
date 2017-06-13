#ifndef _Header_H_
#define _Header_H_

#define FCY 16000000
#include <libpic30.h>

#include "Driver/tydef.h"
#include "Driver/can.h"
#include "Driver/DevicdIO.h"
#include "Driver/Usart.h"
#include "Driver/Timer.h"
#include "Driver/EEPROMOperate.h"


#include "SerialPort/Action.h"
#include "SerialPort/RtuFrame.h"

#include "CANTransmission/buffer.h"

#define LOCAL_ADDRESS 0xA1
#define MAIN_ADDRESS 0x1A

#define FUN_CODE  0x55
#define UP_CODE 0xAA

#define  UART1_MODE 0xAA
#define  UART2_MODE 0x55
#define  U485_MODE 0xA5

#define WORK_MODE  UART1_MODE

#define Reset() {__asm__ volatile ("RESET");}
#endif
