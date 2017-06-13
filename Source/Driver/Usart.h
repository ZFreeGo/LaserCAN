/* 
 * File:   Usart.h
 * Author: LiDehai
 *
 * Created on 2014年12月3日, 上午7:22
 */

#ifndef USART_H
#define	USART_H

#include "tydef.h"

#ifdef	__cplusplus
extern "C" {
#endif
#define TX2   LATFbits.LATF5
#define TX2_T TRISFbits.TRISF5
#define RX2   PORTFbits.RF4
#define RX2_T TRISFbits.TRISF4

#define TX   LATCbits.LATC14
#define TX_T TRISCbits.TRISC14
#define RX   PORTCbits.RC13
#define RX_T TRISCbits.TRISC13

#define RX_TX_MODE  LATFbits.LATF6  //0-接收模式， 1-发送模式
#define RX_TX_MODE_T TRISFbits.TRISF6
#define RX_MODE 0
#define TX_MODE 1
    
#define ON_UART1_INT()  {IEC0bits.U1RXIE = 1;	} //应该配对使用
#define OFF_UART1_INT()  {IEC0bits.U1RXIE = 0;	}    
    
#define ON_UART2_INT()  {IEC1bits.U2RXIE = 1;	} //应该配对使用
#define OFF_UART2_INT()  {IEC1bits.U2RXIE = 0;	}    

void UsartInit();

void UsartSend(unsigned char abyte);
void UsartSendData(PointUint8* pPoint);
void Usart1Send(unsigned char abyte);
void Usart1SendData(PointUint8* pPoint);
void Usart2Send(unsigned char abyte);
void Usart2SendData(PointUint8* pPoint);
#ifdef	__cplusplus
}
#endif

#endif	/* USART_H */

