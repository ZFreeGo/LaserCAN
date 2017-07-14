/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:Main.c
*文件标识:
*创建日期： 2017年5月10日 
*摘要:
 * 数据结构：目的地址（1byte 0xA1）--功能码(1byte 0x55)--数据长度(1byte) + (数据包) + 16bit累加和（ 2bytes）
 * 数据包： ID（low byte） + ID （Hight Byte） + 数据(1-8) 
 * 实现CAN与串口转发
 * 
*2017/5/10: 新建光纤CAN转发程序
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/

// DSPIC30F4011 Configuration Bit Settings

// 'C' source line config statements

#include <xc.h>
#include "Header.h"
#include <string.h>
// FOSC
#pragma config FPR = XT_PLL16            // Primary Oscillator Mode (XT w/PLL 4x)
#pragma config FOS = PRI                // Oscillator Source (Internal Fast RC)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_1         // WDT Prescaler B (1:1)
#pragma config FWPSA = WDTPSA_8         // WDT Prescaler A (1:8)
#pragma config WDT = WDT_ON             // Watchdog Timer (Enabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV42          // Brown Out Voltage (4.2V)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config LPOL = PWMxL_ACT_HI      // Low-side PWM Output Polarity (Active High)
#pragma config HPOL = PWMxH_ACT_HI      // High-side PWM Output Polarity (Active High)
#pragma config PWMPIN = RST_IOPIN       // PWM Output Pin Reset (Control with PORT/TRIS regs)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)


frameRtu sendFrame, recvFrame;


CAN_msg ReciveMsg;
uint8_t SendData[12];
uint8_t SendPacket[20];
 PointUint8 Point;
 /**
  *最后CAN接收接收计数，进行时间计数若长时间未收到信息认为发生错误。
  */
 uint32_t CanOverTimeCount;
 uint32_t UartOverTimeCount;
 
void SendCANState(void);



int main()
{
    uint8_t result = 0;
      ClrWdt();
    __delay_ms(10);
      ClrWdt();
      __delay_ms(10);
      ClrWdt();
      __delay_ms(10);
      ClrWdt();
      __delay_ms(10);
      ClrWdt();
      __delay_ms(10);
      ClrWdt();
      __delay_ms(10);
      ClrWdt();
      __delay_ms(10);
      ClrWdt();
    uint16_t cn = 0;
    uint32_t sendCn = 0;
    
    InitDeviceIO(); //IO初始化 首先禁止中断  
    ClrWdt();

    SetTimer4(1);   //1ms的固定延时时间
    ClrWdt(); 
    UsartInit(); //串口初始化 9600bps 785cycs 上

    ClrWdt(); 

    RX_TX_MODE = TX_MODE; //串口发送    
    __delay_us(20);
    
    UsartSend(0xAA);
     ClrWdt();
    UsartSend(0x55);
     ClrWdt();
    UsartSend(0xAA);

    RX_TX_MODE = RX_MODE;
    ClrWdt(); //204cys 

    SetTimer2(100);//用于超时检测
    
    ClrWdt(); //452cycs
    ReciveFrameDataInit();              //接收帧初始化
    sendFrame.address =  LOCAL_ADDRESS; //本机接收地址处理
    ClrWdt(); //21cys
    
    CanOverTimeCount = 0;
    UartOverTimeCount = 0;
    cn = 0;
    BufferInit();
    InitStandardCAN(0, 0);     

  

    Point.pData = SendPacket;
    while(1)
    {
        result = ReciveBufferDataSimpleDealing(&sendFrame, &recvFrame);//返回剩余长度
        ClrWdt();
        if (recvFrame.completeFlag == TRUE)
        {
            UartOverTimeCount = 0;
            LEDB = 1 - LEDB;
            //将数据帧发送出去
            uint16_t id = recvFrame.pData[3] |(((uint16_t)recvFrame.pData[4])<<8);   
            //是否为本机CAN MAC ID,若为本机则直接上传
            if (LOCAL_MAC_ID == id)
            {
              SendCANState();
            }           
            
             ClrWdt();
            result = CANSendData(id, recvFrame.pData + 5, recvFrame.datalen - 2);
            if(result)
            {
                 Reset();
            }
            else
            {
                recvFrame.completeFlag = FALSE;
            }
        }
         ClrWdt();
        result = BufferDequeue(&ReciveMsg);
         ClrWdt();
        if (result)
        {
            CanOverTimeCount = 0;
             ClrWdt();
            LEDC = 1 - LEDC;
            memcpy(SendData + 2, ReciveMsg.data, ReciveMsg.len);
            SendData[0] = ReciveMsg.id;
            SendData[1] = ReciveMsg.id >> 8;
            ClrWdt();
            GenRTUFrameCumulativeSum(MAIN_ADDRESS,  UP_CODE,   SendData, ReciveMsg.len + 2,  Point.pData  ,  &Point.len);          
             ClrWdt();            
            UsartSendData(&Point);
        }
         ClrWdt();
        if (cn++ > 10000)
        {
             LEDA = 1 - LEDA;
             cn = 0;
        }
         //每隔一段时间重新发送状态信息3-4s发送一次
        if ( sendCn++ > 600000)
        {
            SendCANState();
            sendCn = 0;
        }
         //CAN长时间未收到信息超时复位 约15s
         if ( CanOverTimeCount ++ > 3000000)//
         {
             Reset();
         }
         //UART长时间未收到信息超时复位 约15s
         if (UartOverTimeCount ++ > 3000000)//
         {
             Reset();
         }
         ClrWdt();
         if (C1INTFbits.TXBO)
         {
             InitStandardCAN(0, 0);     
         }
    }
    
  
}

 /**
  * 发送CAN状态
  */
 void SendCANState(void)
 {
    SendData[0] = LOCAL_MAC_ID;
    SendData[1] = 0;
    SendData[2] = 0x11 | 0x80;
    SendData[3] = 0xAA;
    SendData[4] = (uint8_t)(C1INTF); //错误标志   
    SendData[5] = (uint8_t)(C1INTF >> 8); //错误标注
    SendData[6] = (uint8_t)(C1EC); //接收错误计数               
    SendData[7] = (uint8_t)(C1EC >> 8); //发送错误计数               
    SendData[8] = 0;
    SendData[9] = 0;
    ClrWdt();
    GenRTUFrameCumulativeSum(MAIN_ADDRESS,  UP_CODE,   SendData, 10,  Point.pData  ,  &Point.len); 
    UsartSendData(&Point);
    
    //溢出错误，清空标志位
    if(C1INTFbits.RX0OVR )
    {
        C1INTFbits.RX0OVR = 0;
        C1RX0CONbits.RXFUL = 0; 
    }
    if(C1INTFbits.RX1OVR)
    {
        C1INTFbits.RX1OVR = 0;
        C1RX1CONbits.RXFUL = 0; 
    }    
    

   
 }