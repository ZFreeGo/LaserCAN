#include "usart.h"
#include <xc.h>
#include "../SerialPort/RtuFrame.h"
#include  "../Header.h"

extern boolean volatile ReciveErrorFlag;
void InitUART2(unsigned int baud);
void InitPortsUART2(void);

void InitUART1(unsigned int baud)
{
    float value = 0;
    // This is an EXAMPLE, so brutal typing goes into explaining all bit sets

    // The HPC16 board has a DB9 connector wired to UART2, so we will
    // be configuring this port only

    // configure U2MODE
    U1MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
    
    U1MODEbits.USIDL = 0;	// Bit13 Continue in Idle
    U1MODEbits.WAKE = 0;	// Bit7 No Wake up (since we don't sleep here)
    U1MODEbits.LPBACK = 0;	// Bit6 No Loop Back
    U1MODEbits.ABAUD = 0;	// Bit5 No Autobaud (would require sending '55')
    U1MODEbits.PDSEL = 0;	// Bits1,2 8bit, No Parity
    U1MODEbits.STSEL = 0;	// Bit0 One Stop Bit
    U1MODEbits.ALTIO = 1;//使用辅助位

    value = (float)FCY /(float)(16*baud) - 1; //波特率 = FCY/(16 * (BRG + 1))
    U1BRG = 17;	//25-9600 //12
    
    U1STAbits.UTXBRK = 0;	//Bit11 Disabled
    U1STAbits.UTXEN = 0;	//Bit10 TX pins controlled by periph
    U1STAbits.UTXBF = 0;	//Bit9 *Read Only Bit*
    U1STAbits.TRMT = 0;	//Bit8 *Read Only bit*
    U1STAbits.URXISEL = 0;	//Bits6,7 Int. on character recieved
    U1STAbits.ADDEN = 0;	//Bit5 Address Detect Disabled
    U1STAbits.RIDLE = 0;	//Bit4 *Read Only Bit*
    U1STAbits.PERR = 0;		//Bit3 *Read Only Bit*
    U1STAbits.FERR = 0;		//Bit2 *Read Only Bit*
    U1STAbits.OERR = 0;		//Bit1 *Read Only Bit*
    U1STAbits.URXDA = 0;	//Bit0 *Read Only Bit*

    IFS0bits.U1TXIF = 0;	// Clear the Transmit Interrupt Flag
    IFS0bits.U1RXIF = 0;	// Clear the Recieve Interrupt Flag
    IEC0bits.U1RXIE = 1;	// 

    U1MODEbits.UARTEN = 1;	// And turn the peripheral on
    U1STAbits.UTXEN = 1;
    
}
/********************************************
*函数名：InitPortsUART2()
*形参：void
*返回值：void
*功能：配置串口端口，尤其相对应的485使能发送端口
**********************************************/
void InitPortsUART1()
{ 
    RX_T = 1;
    TX_T = 0;
    RX_TX_MODE_T = 0;
    RX_TX_MODE = RX_MODE ; //默认为接收
}

void UsartInit(void)
{
    InitPortsUART1();
    InitUART1(9600);
    
    
    InitPortsUART2();
    InitUART2(1);
}
void UsartSend(unsigned char abyte)
{
    RX_TX_MODE = TX_MODE;   //--鉴于光耦响应时间，须有一定的延时
    U1TXREG = abyte;
    while(!U1STAbits.TRMT)
    {
        ClrWdt(); //2ms超时后,看门狗复位
    }
    
    RX_TX_MODE = RX_MODE;
}
void UsartRecive(unsigned char abyte)
{
}

void __attribute__ ((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{
    
    ClrWdt();
    IFS0bits.U1RXIF = 0;
    ReciveErrorFlag = FrameQueneIn(U1RXREG);
    
}
void __attribute__ ((interrupt, no_auto_psv)) _U1TXInterrupt(void)
{
	IFS0bits.U1TXIF = 0;
}



/**************************************************
 *函数名： InitUART2()
 *功能:初始化UART2
 *形参：  unsigned int baud 波特率
 *返回值：void
****************************************************/
void InitUART2(unsigned int baud)
{
    float value = 0;
    // This is an EXAMPLE, so brutal typing goes into explaining all bit sets

    // The HPC16 board has a DB9 connector wired to UART2, so we will
    // be configuring this port only

    // configure U2MODE
    U2MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
    Nop();
    U2MODEbits.USIDL = 0;	// Bit13 Continue in Idle
    U2MODEbits.WAKE = 0;	// Bit7 No Wake up (since we don't sleep here)
    U2MODEbits.LPBACK = 0;	// Bit6 No Loop Back
    U2MODEbits.ABAUD = 0;	// Bit5 No Autobaud (would require sending '55')
    U2MODEbits.PDSEL = 0;	// Bits1,2 8bit, No Parity
    U2MODEbits.STSEL = 0;	// Bit0 One Stop Bit


    value = (float)FCY /(float)(16*baud) - 1; //波特率 = FCY/(16 * (BRG + 1))
    U2BRG = 17;	//4M-25-9600 16M-17-56000 25-38000  8M--12-38000
    
    U2STAbits.UTXBRK = 0;	//Bit11 Disabled
    U2STAbits.UTXEN = 0;	//Bit10 TX pins controlled by periph
    U2STAbits.UTXBF = 0;	//Bit9 *Read Only Bit*
    U2STAbits.TRMT = 0;	//Bit8 *Read Only bit*
    U2STAbits.URXISEL = 0;	//Bits6,7 Int. on character recieved
    U2STAbits.ADDEN = 0;	//Bit5 Address Detect Disabled
    U2STAbits.RIDLE = 0;	//Bit4 *Read Only Bit*
    U2STAbits.PERR = 0;		//Bit3 *Read Only Bit*
    U2STAbits.FERR = 0;		//Bit2 *Read Only Bit*
    U2STAbits.OERR = 0;		//Bit1 *Read Only Bit*
    U2STAbits.URXDA = 0;	//Bit0 *Read Only Bit*

    IFS1bits.U2TXIF = 0;	// Clear the Transmit Interrupt Flag
    IFS1bits.U2RXIF = 0;	// Clear the Recieve Interrupt Flag
    IEC1bits.U2RXIE = 1;	// 

    U2MODEbits.UARTEN = 1;	// And turn the peripheral on
    U2STAbits.UTXEN = 1;
    
}


/********************************************
*函数名：InitPortsUART2()
*形参：void
*返回值：void
*功能：配置串口端口，尤其相对应的485使能发送端口
**********************************************/
void InitPortsUART2(void)
{ 
    RX2_T = 1;
    TX2_T = 0;

}

inline void Usart2Send(unsigned char abyte)
{
   // RX_TX_MODE = TX_MODE;   //--鉴于光耦响应时间，须有一定的延时
    U2TXREG = abyte;
    while(!U2STAbits.TRMT)
    {
        ClrWdt(); //2ms超时后,看门狗复位
    }
    
   // RX_TX_MODE = RX_MODE;
}
inline void Usart2SendData(PointUint8* pPoint)
{
    uint8_t i = 0;
    for ( i =0; i < pPoint->len; i++)
    {
         U2TXREG = pPoint->pData[i];
        while(!U2STAbits.TRMT)
        {
            ClrWdt(); //2ms超时后,看门狗复位
        }
    }
  
    
   // RX_TX_MODE = RX_MODE;
}
void __attribute__ ((interrupt, no_auto_psv)) _U2RXInterrupt(void)
{
    
    ClrWdt();
    IFS1bits.U2RXIF = 0;
   //  Usart2Send(U2RXREG);
   ReciveErrorFlag = FrameQueneIn(U2RXREG);
    
}
void __attribute__ ((interrupt, no_auto_psv)) _U2TXInterrupt(void)
{
	IFS1bits.U2TXIF = 0;
}