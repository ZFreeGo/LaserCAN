/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:CAN.c
*文件标识:
*创建日期： 2015年11月10日 
*摘要:
*2015/11/10:编写CAN配置程序
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#include "CAN.h"
#include "../Header.h"

//#define FCY 		30000000             		// 30 MHz
#define BITRATE 	100000			 			// 100K
#define NTQ 		16							// Number of Tq cycles which will make the 
												//CAN Bit Timing .
#define BRP_VAL		(((float)FCY * 4/(2*NTQ*BITRATE))-1)  //Formulae used for C1CFG1bits.BRP 

uint16_t g_CANStatus;


void ConfigCANOneMaskFilterRX1(EIDBits* pRm1, EIDBits* pRf2);
void ConfigCANOneMaskFilterRX0(EIDBits* pRm0, EIDBits* pRf0);
void GetReciveRX0EID(EIDBits* pEID);
void ConfigCANOneBraud(void);


CAN_msg       CAN_TxMsg;                      /* CAN message for sending */
CAN_msg       CAN_RxMsg;                      /* CAN message for receiving */                                

unsigned int  CAN_TxRdy;              /* CAN HW ready to transmit a message */
unsigned int  CAN_RxRdy;              /* CAN HW received a message */


/********************************************
*函数名： ConfigCANOneBraud()
*形参：void
*返回值：void
*功能：配置CAN1 波特率有关
**********************************************/
inline void ConfigCANOneBraud(void)
{
    C1CTRLbits.CANCKS = 1;			// Select the CAN Master Clock . It is equal to Fcy here. 
                                    // equal to Fcy.(Fcy=30Mhz)
    C1CFG1bits.SJW=00;				//1TQ 同步跳转宽度时间 Synchronized jump width time is 1 x TQ when SJW is equal to 00
    C1CFG1bits.BRP = BRP_VAL;		//波特率预分频比位 ((FCY/(2*NTQ*BITRATE))-1) 	

    //C1CFG2 = 0x03F5;               // SEG1PH=6Tq, SEG2PH=3Tq, PRSEG=5Tq    
                                    // Sample 3 times
                                    // Each bit time is 15Tq    
   
    C1CFG2bits.SEG1PH = 1; //相位缓冲段 （n+1） TQ  
    C1CFG2bits.SEG2PHTS = 1; //相位段 2 时间选择位 1-可自由编程， 0-SEG1PH 与信息处理时间 （ 3 TQ）中的较大值
    C1CFG2bits.SAM = 0;    //采样次数 1-采样1次， 0-采样3次    
    C1CFG2bits.PRSEG = 1; //广播时间端bit
    C1CFG2bits.SEG2PH = 2; //相位缓冲段 2 位 
}

/********************************************
*函数名： ConfigCANOneMaskFilterRX0()
*形参：void
*返回值：void
*功能：配置CAN1 RX0接收屏蔽与滤波寄存器寄存器
**********************************************/
inline void ConfigCANOneMaskFilterRX0(EIDBits* pRm0, EIDBits* pRf0)
{
  // Configure Receive registers, Filters and Masks配置接收滤波器与屏蔽滤波器
     //清空接收滤波器
     // 接收缓冲区 0 状态和控制寄存器  RM0-RF0,RF1
      C1RX0CON = 0x0000;
      C1RX0CONbits.FILHIT0 = 0; //选择接收滤波器RF0
       //RM0 屏蔽器配置 标准与扩展
      C1RXM0SIDbits.MIDE = 0; //过滤类型 扩展帧; //EXIDE = 1 其它29bit全为1， 0-标准帧
      C1RXM0SIDbits.SID     = pRm0->regBits.SID10_0; 
      //C1RXM0EIDH            = pRm0->regBits.EID17_6;
      //C1RXM0EIDLbits.EID5_0 = pRm0->regBits.EID5_0;       
      //接收滤波器 RF0 (RF1)
      C1RXF0SIDbits.EXIDE = 0;//1-使能扩展帧 0-标准帧
      C1RXF0SIDbits.SID     = pRf0->regBits.SID10_0;	//CAN1 Receive Acceptance Filter2 SID 	 bit 12-2 SID<10:0>： 标准标识符屏蔽位
      //C1RXF0EIDH            = pRf0->regBits.EID17_6;   //CAN1 Receive Acceptace  Filter2 Extended Identifier high byte  bit 11-0 EID<17:6>： 扩展标识符屏蔽位 17-6
     // C1RXF0EIDLbits.EID5_0 =  pRf0->regBits.EID5_0;	//CAN1 Receive Acceptance Filter2 Extended identifier low byte   bit 15-10 EID<5:0>： 扩展标识符位         

}
/********************************************
*函数名： ConfigCANOneMaskFilterRX1()
*形参：void
*返回值：void
*功能：配置CAN1 RX1接收屏蔽与滤波寄存器寄存器
**********************************************/
inline void ConfigCANOneMaskFilterRX1(EIDBits* pRm1, EIDBits* pRf2)
{
          //接收缓冲区1配置 RF1 RM1-RF2,RF3,RF4,RF5
      C1RX1CON = 0x0000;
      C1RX1CONbits.FILHIT = 0b010;//选择接收过滤器RXF2
       //RM1 屏蔽器配置 标准与扩展
      C1RXM1SIDbits.MIDE = 1; //过滤类型 扩展帧; //EXIDE = 1 其它29bit全为1
      C1RXM1SIDbits.SID     = pRm1->regBits.SID10_0; 
      C1RXM1EIDH            = pRm1->regBits.EID17_6;
      C1RXM1EIDLbits.EID5_0 = pRm1->regBits.EID5_0;       
      //接收滤波器 RF2 (RF2)
      C1RXF2SIDbits.EXIDE = 1;//使能扩展帧
      C1RXF2SIDbits.SID     = pRf2->regBits.SID10_0;	//CAN1 Receive Acceptance Filter2 SID 	 bit 12-2 SID<10:0>： 标准标识符屏蔽位
      C1RXF2EIDH            = pRf2->regBits.EID17_6;   //CAN1 Receive Acceptace  Filter2 Extended Identifier high byte  bit 11-0 EID<17:6>： 扩展标识符屏蔽位 17-6
      C1RXF2EIDLbits.EID5_0 =  pRf2->regBits.EID5_0;	//CAN1 Receive Acceptance Filter2 Extended identifier low byte   bit 15-10 EID<5:0>： 扩展标识符位
}
/********************************************
*函数名： InitCANOne()
*形参：EIDBits* pRm——屏蔽寄存器设置，EIDBits* pRf——滤波寄存器设置
*返回值：uint16_t 初始化成功返回0xAA 失败返回0
*功能：初始化系统中的 CAN1 模块
**********************************************/
 uint16_t InitCANOne(EIDBits* pRm, EIDBits* pRf)
 {
     uint16_t i = 0, state = 0;
    
    ///Interrupt Section of CAN Peripheral 中断配置
     C1INTF = 0;					//Reset all The CAN Interrupts 
     IFS1bits.C1IF = 0;  			//Reset the Interrupt Flag status register
     C1INTE = 0x00FF;               //Enable all CAN interrupt sources
     IEC1bits.C1IE = 1;				//Enable the CAN1 Interrupt 
      
    C1CTRLbits.REQOP = REQOP_CONFIG;//设置为配置模式 
    
    ConfigCANOneBraud();//配置CAN1波特率
    ConfigCANOneMaskFilterRX0(pRm, pRf);//配置RX0接收屏蔽滤波器
    ConfigCANOneMaskFilterRX1(pRm, pRf);//配置RX1接收屏蔽滤波器
    
    C1CTRLbits.REQOP = REQOP_LOOPBACK;
    //2ms 看门狗等待
    i = 0;
    state = 0xAA;
    while(C1CTRLbits.OPMODE != REQOP_LOOPBACK)//Wait for CAN1 mode change from Configuration Mode to Loopback mode 
    {
        __delay_us(10);
        ClrWdt();
        if (i++ > 1000)
        {
            state = 0;
            break;//超时错误
        }
    }
    
    g_CANStatus = 0;
    return state;
     
 }
  uint16_t InitStandardCAN(uint16_t id, uint16_t mask)
 {
     uint16_t i = 0, state = 0;
    
    ///Interrupt Section of CAN Peripheral 中断配置
     C1INTF = 0;					//Reset all The CAN Interrupts 
     IFS1bits.C1IF = 0;  			//Reset the Interrupt Flag status register
     C1INTE = 0x00FF;               //Enable all CAN interrupt sources
     IEC1bits.C1IE = 1;				//Enable the CAN1 Interrupt 
      
    C1CTRLbits.REQOP = REQOP_CONFIG;//设置为配置模式 
    
    //配置CAN比特率
    C1CTRLbits.CANCKS = 0;			// 0-4Fcy 1-Fcy  Select the CAN Master Clock . It is equal to Fcy here. 
                                    // equal to Fcy.(Fcy=30Mhz)
    C1CFG1bits.SJW=00;				//同步跳转宽度时间 Synchronized jump width time is 1 x TQ when SJW is equal to 00
    C1CFG1bits.BRP = BRP_VAL;		//波特率预分频比位 ((FCY/(2*NTQ*BITRATE))-1) 	

   // C1CFG2 = 0x03F5;               // SEG1PH=6Tq, SEG2PH=3Tq, PRSEG=5Tq 
                                    // Sample 3 times
                                    // Each bit time is 15Tq   
    C1CFG2bits.SEG1PH = 6; //相位缓冲段 （n+1） TQ  
    C1CFG2bits.SEG2PHTS = 1; //相位段 2 时间选择位 1-可自由编程， 0-SEG1PH 与信息处理时间 （ 3 TQ）中的较大值
    C1CFG2bits.SAM = 0;    //采样次数 1-采样1次， 0-采样3次    
    C1CFG2bits.PRSEG = 1; //广播时间端bit
    C1CFG2bits.SEG2PH = 5; //相位缓冲段 2 位 
    
    
    
     // Configure Receive registers, Filters and Masks配置接收滤波器与屏蔽滤波器
     //清空接收滤波器
     // 接收缓冲区 0 状态和控制寄存器  RM0-RF0,RF1
      C1RX0CON = 0x0000;
      C1RX0CONbits.FILHIT0 = 0; //选择接收滤波器RF0
       //RM0 屏蔽器配置 标准与扩展
      C1RXM0SIDbits.MIDE = 0; //过滤类型 扩展帧; //EXIDE = 1 其它29bit全为1， 0-标准帧
      C1RXM0SIDbits.SID     =  0;   //全部接收
      //接收滤波器 RF0 (RF1)
      C1RXF0SIDbits.EXIDE = 0;//1-使能扩展帧 0-标准帧
      C1RXF0SIDbits.SID     = 0;	//CAN1 Receive Acceptance Filter2 SID 	 bit 12-2 SID<10:0>： 标准标识符屏蔽位
     
    //错误中断允许位
      C1INTEbits.ERRIE = 1;//错误中断允许位
      C1INTFbits.ERRIF = 0;
      
      
    C1CTRLbits.REQOP = REQOP_WORK   ; //正常工作模式REQOP_WORK
    //2ms 看门狗等待
    i = 0;
    state = 0xAA;
    while(C1CTRLbits.OPMODE !=  REQOP_WORK  )//Wait for CAN1 mode change from Configuration Mode to work mode 
    {
        __delay_us(10);
        ClrWdt();
        if (i++ > 1000)
        {
            state = 0;
            break;//超时错误
        }
    }
    return state;
     
 }
 /********************************************
*函数名：ConfigEIDTX0()
*形参：EIDBits* pEID 发送扩展标识符
*返回值：void
*功能： 配置发送扩展标识符
**********************************************/
 inline void ConfigEIDTX0(EIDBits* pEID)
 {
     C1TX0SIDbits.TXIDE = 1;//扩展帧
     C1TX0SIDbits.SRR = 1;
     C1TX0SIDbits.SID10_6 = pEID->txBits.SID10_6;
     C1TX0SIDbits.SID5_0  = pEID->txBits.SID5_0;
     
     C1TX0EIDbits.EID17_14 = pEID->txBits.EID17_14;
     C1TX0EIDbits.EID13_6 = pEID->txBits.EID13_6;
     
     C1TX0DLCbits.EID5_0 =  pEID->txBits.EID5_0;
 }
 /********************************************
*函数名：ConfigDataTXB0()
*形参：uint8_t len 待处理数据长度 ，CANFrame* pframe 帧数据
*返回值：uint8_t ——发送数据长度 0错误 
*功能： 将待发送数据装入TXB0 发送寄存器
**********************************************/
 uint8_t ConfigDataTXB0(uint8_t len, CANFrame* pframe)
 {
     uint8_t i = 0, j = 0;
     if (len > 0 && len <= 8)
     {
         C1TX0DLCbits.DLC = len;
         C1TX0DLCbits.TXRTR = 0;//正常报文
        
         C1TX0CONbits.TXPRI = 3;
         for ( i = 0, j = 0; i < len; i += 2, j++)
         {
             switch (j)
             {
                 case 0:
                     C1TX0B1 = pframe->frameDataWord[j];
                     break;
                 case 1:
                     C1TX0B2 = pframe->frameDataWord[j];
                     break;
                 case 2:
                     C1TX0B3 = pframe->frameDataWord[j];
                     break;
                 case 3:
                     C1TX0B4 = pframe->frameDataWord[j];
                     break;
             }
         }
         len = C1TX0DLCbits.DLC;
     }
     else
     {
         len =  0;
     }
     return len;
 }
  /********************************************
*函数名：ConfigDataTXB1()
*形参：uint8_t len 待处理数据长度 ，CANFrame* pframe 帧数据
*返回值：uint8_t ——发送数据长度 0 错误
*功能： 将待发送数据装入TXB1 发送寄存器
**********************************************/
  uint8_t ConfigDataTXB1(uint8_t len, CANFrame* pframe)
 {
     uint8_t i = 0, j = 0;
     if (len > 0 && len <= 8)
     {
         C1TX1DLCbits.DLC = len ;
         C1TX1DLCbits.TXRTR = 0;//正常报文
         C1TX1CONbits.TXPRI = 2;
         for ( i = 0, j = 0; i < len; i += 2, j++)
         {
             switch (j)
             {
                 case 0:
                     C1TX1B1 = pframe->frameDataWord[j];
                     break;
                 case 1:
                     C1TX1B2 = pframe->frameDataWord[j];
                     break;
                 case 2:
                     C1TX1B3 = pframe->frameDataWord[j];
                     break;
                 case 3:
                     C1TX1B4 = pframe->frameDataWord[j];
                     break;
             }
         }
         len = C1TX1DLCbits.DLC;
     }
     else
     {
         len =  0;
     }
     return len;
 }
  
   /********************************************
*函数名：ConfigDataTXB2()
*形参：uint8_t len 待处理数据长度 ，CANFrame* pframe 帧数据
*返回值：uint8_t ——发送数据长度 0错误
*功能： 将待发送数据装入TXB2 发送寄存器
**********************************************/
uint8_t ConfigDataTXB2(uint8_t len, CANFrame* pframe)
 {
     uint8_t i = 0, j = 0;
     if (len > 0 && len <= 8)
     {
         C1TX2DLCbits.DLC = len;
         C1TX2DLCbits.TXRTR = 0;//正常报文
          C1TX0CONbits.TXPRI = 1;
         for ( i = 0, j = 0; i < len; i += 2, j++)
         {
             switch (j)
             {
                 case 0:
                     C1TX2B1 = pframe->frameDataWord[j];
                     break;
                 case 1:
                     C1TX2B2 = pframe->frameDataWord[j];
                     break;
                 case 2:
                     C1TX2B3 = pframe->frameDataWord[j];
                     break;
                 case 3:
                     C1TX2B4 = pframe->frameDataWord[j];
                     break;
             }
         }
         len = C1TX2DLCbits.DLC;
     }
     else
     {
         len =  0;
     }
     return len;
 }

/********************************************
*函数名：CANOneSendByTX0()
*形参：: uint16_t* pID  11bitID标识, uint8_t * pbuff 缓冲数据, uint8_t len 数据长度
*返回值：uint8_t —— 发送数据总长度 0--数据出错
*功能： 通过TX0发送带有CRC16的帧数据
**********************************************/
 uint8_t CANSendData(uint16_t id, uint8_t * pbuff, uint8_t len)
 {  
    if ((len <= 8) && (len > 0))
    {
        C1TX0SIDbits.TXIDE = 0;//标准帧
        C1TX0SIDbits.SRR = 0; //0-正常报文 1-报文将请求远程发送
        C1TX0SIDbits.SID10_6 = GET_SID10_6(id);
        C1TX0SIDbits.SID5_0  =  GET_SID5_0(id);    
        
         C1TX0DLCbits.DLC = len;
         C1TX0DLCbits.TXRTR = 0;//正常报文       
         C1TX0CONbits.TXPRI = 3;
         
         //可考虑使用地址，简化操作
         switch(len) 
         {
             case 1:
             {
                 C1TX0B1 =   pbuff[0];                 
                 break;
             }
             case 2:
             {
                 C1TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];                 
                 break;
             }
             case 3:
             {
                 C1TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];       
                 C1TX0B2 =   pbuff[2];                 
                 break;
             }
             case 4:
             {
                 C1TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                 C1TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                 break;
             } 
             case 5:
             {
                  C1TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                  C1TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                  C1TX0B3 =   pbuff[4]; 
                 break;
             }
             case 6:
             {
                 C1TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                 C1TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                 C1TX0B3 =   (((uint16_t)pbuff[5]) << 8)  | pbuff[4];
                 break;
             } 
             case 7:
             {
                 C1TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                 C1TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                 C1TX0B3 =   (((uint16_t)pbuff[5]) << 8)  | pbuff[4];
                 C1TX0B4 =   pbuff[6]; 
                 break;
             }
             case 8:
             {
                 C1TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                 C1TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                 C1TX0B3 =   (((uint16_t)pbuff[5]) << 8)  | pbuff[4];
                 C1TX0B4 =   (((uint16_t)pbuff[7]) << 8)  | pbuff[6];
                 break;
             }    
         }
              
          
         
        
        
        
        
        C1TX0CONbits.TXREQ = 1;         //请求发送
    }
    return 0;
 }
// 
// /***********************************************************
//*函数名：ReadRx0Frame()
//*形参：CANFrame* pframe--保存帧地址
//*返回值：uint8_t —— 接收数据长度 0--数据出错
//*功能： 读取帧数据 
//**************************************************************/
// uint8_t ReadRx0Frame(CANFrame* pframe)
// {
//     uint8_t len = 0, i = 0, j = 0;
//     
//     if ((C1RX0CONbits.RXFUL == 1) && (C1RX0CONbits.RXRTRRO == 0) ) //包含一个有效报文 且不是远程帧
//     {
//         len = C1RX0DLCbits.DLC;
//         for ( i = 0, j = 0; i < len; i += 2, j++)
//         {
//             switch (j)
//             {
//                 case 0:
//                     pframe->frameDataWord[j] = C1RX0B1 ;
//                     break;
//                 case 1:
//                     pframe->frameDataWord[j] = C1RX0B2 ;
//                     break;
//                 case 2:
//                     pframe->frameDataWord[j] = C1RX0B3 ;
//                     break;
//                 case 3:
//                     pframe->frameDataWord[j] = C1RX0B4 ;
//                     break;
//                 default:
//                     return 0;//错误跳出
//             }
//         }
//        
//     }
//      C1RX0CONbits.RXFUL = 0; 
//     return len;
// }
 uint8_t ReadRx0Frame(PointUint8* pPoint)
 {
     uint8_t len = 0, i = 0, j = 0;
     if (pPoint->len >= 8)
     {
         if ((C1RX0CONbits.RXFUL == 1) && (C1RX0CONbits.RXRTRRO == 0) ) //包含一个有效报文 且不是远程帧
         {
            len = C1RX0DLCbits.DLC;
            pPoint->pData[0] = C1RX0B1  ;
            pPoint->pData[1] = C1RX0B1 >> 8 ;
            pPoint->pData[2] = C1RX0B2  ;
            pPoint->pData[3] = C1RX0B2 >> 8 ;
            pPoint->pData[4] = C1RX0B3  ;
            pPoint->pData[5] = C1RX0B3 >> 8 ;
            pPoint->pData[6] = C1RX0B4  ;
            pPoint->pData[7] = C1RX0B4 >> 8 ;                     
         }
     }
     else
     {
         len = 0;
     }
      C1RX0CONbits.RXFUL = 0; 
     return len;
 }

   /***********************************************************
*函数名：GetReciveRX0EID()
*形参：uint8_t len --数据长度, CANFrame* pframe --待检测帧数据
*返回值：uint8_t —— 若为0代表失败， 大于3 代表成功，其值为数据长度
*功能： 对接收帧数据进行CRC校验
**************************************************************/
 inline void GetReciveRX0EID(EIDBits* pEID)
 {
     pEID->regBits.SID10_0 = C1RX0SIDbits.SID;
     pEID->regBits.EID17_6 = C1RX0EID;
     pEID->regBits.EID5_0 = C1RX0DLCbits.EID5_0;
     
     
 }

 //--------------------------------------------------------------------------------------------------------------------------
											//Interrupt Section for CAN1
//--------------------------------------------------------------------------------------------------------------------------
 CANFrame Rframe;
 uint8_t rlen = 0;
 EIDBits rEID;
 
void __attribute__((interrupt, no_auto_psv)) _C1Interrupt(void)
{
    
    IFS1bits.C1IF = 0;         //Clear interrupt flag
    
        
      if(C1INTFbits.TX0IF)
      {       
		 C1INTFbits.TX0IF = 0;  //If the Interrupt is due to Transmit0 of CAN1 Clear the Interrupt
      }  
      else if(C1INTFbits.TX1IF)
      {        
		C1INTFbits.TX1IF = 0;   //If the Interrupt is due to Transmit1 of CAN1 Clear the Interrupt 
      }  
      if(C1INTFbits.RX0IF)
      {   
		C1INTFbits.RX0IF = 0; 	//If the Interrupt is due to Receive0 of CAN1 Clear the Interrupt        

        CAN_RxMsg.id =  C1RX0SIDbits.SID;
        CAN_RxMsg.len = C1RX0DLCbits.DLC;
        
        PointUint8 point;
        point.len = 8;
        point.pData = CAN_RxMsg.data;
        ReadRx0Frame(&point);                       
        CAN_RxRdy = 1;                            /*  set receive flag */
        BufferEnqueue(&CAN_RxMsg);
        //DeviceNetReciveCenter(&id,Rframe.framDataByte, len);
      
      }
      else if(C1INTFbits.RX1IF)
      {           
          C1INTFbits.RX1IF = 0;  	//If the Interrupt is due to Receive1 of CAN1 Clear the Interrupt   
     }    
    
    if(C1INTFbits.ERRIF)
    {
        C1INTFbits.ERRIF = 0;
        g_CANStatus  = 0xA1;
      //  while(1);
    }
    
    
    if (C1INTFbits.IRXIF)
    {
        C1INTFbits.IRXIF = 0;
         g_CANStatus  = 0xA2;
    }
    C1INTF = 0;
}

