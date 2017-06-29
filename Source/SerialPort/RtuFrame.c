#include "RtuFrame.h"
#include "../Header.h"
#include "CRC16.h"
#include <xc.h>
#include <string.h>

/******************************************
//针对此种情况出队与入队可能发生冲突。
//改在出队时，禁止串口接收中断。 2015/10/15
//增大缓冲帧容量由16改为64
//每次进入帧处理队列后刷新接收使能位
//增加  ReciveErrorFlag 接收错误标志 及接收满后的重新清空初始化
********************************************/

//#include <stdlib.h>
//入队错误标志位 TRUE未满 正常入队 FALSE 无法入队
boolean volatile ReciveErrorFlag = TRUE; 

#define FRAME_QUENE_LEN 256
uint8_t  volatile g_ReciveBufferLen = 0; //未处理接收数据长度
uint8_t  volatile  g_ReciveBuffer[FRAME_QUENE_LEN] = {0}; //临时存放串口接收数据
uint8_t  FifoHead = 0;
uint8_t  FifoEnd =0;
uint8_t  DealStep = 0;

//帧数据放置区域

uint8_t  volatile FrameData[FRAME_DATA_LEN] = {0};

uint8_t RtuFrame[16] = {0};
uint8_t completeFlag = 0;

uint8_t LocalAddress =  LOCAL_ADDRESS;

uint8_t ReciveIndex = 0;  //接收索引 指向待存帧位置

uint8_t  volatile SendFrameData[SEND_FRAME_LEN] = {0};

/**************************************************
 *函数名： ReciveFrameDataInit()
 *功能：  初始化此文件模块有关变量
 *形参： void
 *返回值：void
****************************************************/
void  ReciveFrameDataInit(void)
{
    g_ReciveBufferLen = 0;
    FifoHead = 0;
    FifoEnd = 0;
    
    ReciveErrorFlag = TRUE;
    LocalAddress =  LOCAL_ADDRESS;
    DealStep = 0;
    ReciveIndex = 0;
    completeFlag = 0;
}

/**************************************************
 *函数名： FrameQueneIn()
 *功能：  帧队列入队
 *形参：入队数据 uint8_t * pData
 *返回值：如果队列为满，则返回FALSE，否则为TURE
****************************************************/
boolean FrameQueneIn(uint8_t recivData)
{
    //队列未满
    if (g_ReciveBufferLen < FRAME_QUENE_LEN)
    {
        g_ReciveBufferLen++;
        g_ReciveBuffer[FifoEnd] = recivData; //入队
        FifoEnd =( FifoEnd + 1)% FRAME_QUENE_LEN;

        return TRUE;
    }
    return FALSE;
}
/**************************************************
 * 函数名：FrameQueneOut()
 * 功能：   帧队列出队
 * 形参：出队数据 uint8_t* pData
 * 返回值：如果为队列空则返回FALSE，否则为TURE
****************************************************/
boolean FrameQueneOut(uint8_t* pData)
{
    //队列是否为空
    #if (WORK_MODE == UART1_MODE)
        OFF_UART1_INT();
    #else
        OFF_UART2_INT();//防止接收读取冲突 应配对使用
    #endif
    if (g_ReciveBufferLen > 0)
    {
        g_ReciveBufferLen--;
        *pData = g_ReciveBuffer[FifoHead]; //首先出队
        FifoHead =( FifoHead + 1)% FRAME_QUENE_LEN;

        #if (WORK_MODE == UART1_MODE)
        ON_UART1_INT();
        #else
            ON_UART2_INT();//防止接收读取冲突 应配对使用
        #endif
        return TRUE;
    }
     #if (WORK_MODE == UART1_MODE)
        ON_UART1_INT();
    #else
        ON_UART2_INT();//防止接收读取冲突 应配对使用
    #endif
    return FALSE;
}



/*************************************************
 *函数名： ReciveBufferDataSimpleDealing()
 *功能：  对接收数据进行处理
 *形参：
 *返回值：
****************************************************/
uint8_t ReciveBufferDataSimpleDealing(frameRtu* pJudgeFrame, frameRtu* pReciveFrame)
{
    ClrWdt(); 
    RX_TX_MODE = RX_MODE; //刷新接收模式使能
    if (IFS0bits.T2IF == 1) //说明超时则丢弃
    {
        StopTimer2();
        DealStep = 0;
    }
    if (TRUE != ReciveErrorFlag)//如果接收错误
    {
        ReciveFrameDataInit();
        return 0;//重新初始化并返回
    }
    //如果存在未处理的数据
    ClrWdt(); 
    if (g_ReciveBufferLen > 0)//3 +  1 + 2 至少一个数据头三个，校验两字节
    {        
        switch(DealStep)
        {
            //判断地址
            case 0:
            {
                uint8_t data = 0;
                ClrWdt();
                pReciveFrame->completeFlag = FALSE;
                FrameQueneOut(&data);
                ClrWdt();
                if (pJudgeFrame->address == data) //地址符合则进行下一轮判断
                {
                    pReciveFrame->address =  data;
                    DealStep  = 1;
                    StartTimer2(); //超时接收检测定时器
                }
                break;
            }
            //获取功能代码
            case 1:
            {
                ClrWdt();
                uint8_t data = 0;
                FrameQueneOut(&data);
                pReciveFrame->funcode = data; //判断是否为转发包
                if (FUN_CODE == data)
                {
                   
                     DealStep = 2;
                }
                else
                {
                     StopTimer2();
                     DealStep = 0;
                }               
                break;
            }
            //获取数据字节长度
            case 2:
            {
                ClrWdt();
                uint8_t data = 0;
                FrameQueneOut(&data);
                if ( (data <= 10) && (data > 0))//2 + 8 =10
                {                
                    pReciveFrame->datalen = data;
                    DealStep = 3;
                    ReciveIndex = 3;
                }
                else
                { 
                    DealStep = 0;
                    ReciveIndex = 0;                    
                }
                break;
            }
            //进行校验
            case 3:
            {
                ClrWdt();
                //应加计时防止此处长时间接收不到
                //接收缓冲数据应大于等于数据长度
                //转存数据
                do
                {
                    FrameQueneOut((uint8_t*)&FrameData[ReciveIndex++]);// ReciveIndex++; //接收数据则索引加一

                    if (ReciveIndex > FRAME_DATA_LEN)
                    {
                        ReciveIndex = 3;
                        DealStep = 0;
                        break;
                        //接收帧长度过长丢弃
                    }
                    ClrWdt();
                    uint8_t len = pReciveFrame->datalen;
                    if (ReciveIndex >= len + 5)
                    {
                        FrameData[0] = pReciveFrame->address;
                        FrameData[1] = pReciveFrame->funcode;
                        FrameData[2] = len;
                        ClrWdt(); //两个ms不知是否够运算
                        PointUint8 Point;
                        Point.pData = (uint8_t* )&FrameData;
                        Point.len = len + 3;
                        uint16_t csc =  CumulativeSumCalculate(&Point);
                        ClrWdt();
                        uint8_t  cscL = FrameData[len + 3];
                        uint8_t  cscH = FrameData[len + 4];
                        ClrWdt();
                        //若校验吻合,则执行下一步动作
                        if (csc == ((uint16_t)cscH<<8  | cscL))
                        {
                            pReciveFrame->pData = (uint8_t*)FrameData;
                            pReciveFrame->completeFlag = TRUE;
                        }
                        ClrWdt();
                        StopTimer2();  //超时检测结束  100ns以内处理一帧数据若超时则丢弃                
                        DealStep = 0;
                        break;
                    }
                }
                while(g_ReciveBufferLen > 0);//存在数据则继续进行处理
                break;
            }
            default:
            {
                DealStep = 0;
                ReciveIndex = 3;
                break;
            }
        }
        
    }
    ClrWdt();
    return g_ReciveBufferLen; //返回剩余长度
}


/*************************************************
 *函数名： ReciveBufferDataDealing()
 *功能：  对接收数据进行处理
 *形参：
 *返回值：
****************************************************/
uint8_t ReciveBufferDataDealing(frameRtu* pJudgeFrame, frameRtu* pReciveFrame)
{
    ClrWdt(); 
    RX_TX_MODE = RX_MODE; //刷新接收模式使能
    if (IFS0bits.T2IF == 1) //说明超时则丢弃
    {
        StopTimer2();
        DealStep = 0;
    }
    if (TRUE != ReciveErrorFlag)//如果接收错误
    {
        ReciveFrameDataInit();
        return 0;//重新初始化并返回
    }
    //如果存在未处理的数据
    ClrWdt(); 
    if (g_ReciveBufferLen > 0)
    {
        switch(DealStep)
        {
            //判断地址
            case 0:
            {
                uint8_t data = 0;
                ClrWdt();
                pReciveFrame->completeFlag = FALSE;
                FrameQueneOut(&data);
                ClrWdt();
                if (pJudgeFrame->address == data) //地址符合则进行下一轮判断
                {
                    pReciveFrame->address =  data;
                    DealStep  = 1;
                    StartTimer2(); //超时接收检测定时器
                }
                break;
            }
            //获取功能代码
            case 1:
            {
                ClrWdt();
                uint8_t data = 0;
                FrameQueneOut(&data);
                pReciveFrame->funcode = data;
                DealStep = 2;
                break;
            }
            //获取数据字节长度
            case 2:
            {
                ClrWdt();
                uint8_t data = 0;
                FrameQueneOut(&data);
                pReciveFrame->datalen = data;
                DealStep = 3;
                ReciveIndex = 3;
                break;
            }
            //进行校验
            case 3:
            {
                ClrWdt();
                //应加计时防止此处长时间接收不到
                //接收缓冲数据应大于等于数据长度
                //转存数据
                FrameQueneOut((uint8_t*)&FrameData[ReciveIndex++]);// ReciveIndex++; //接收数据则索引加一

                if (ReciveIndex > FRAME_DATA_LEN)
                {
                    ReciveIndex = 3;
                    DealStep = 0;
                    break;
                    //接收帧长度过长丢弃
                }
                ClrWdt();
                uint8_t len = pReciveFrame->datalen;
                if (ReciveIndex >= len + 5)
                {
                    FrameData[0] = pReciveFrame->address;
                    FrameData[1] = pReciveFrame->funcode;
                    FrameData[2] = len;
                    ClrWdt(); //两个ms不知是否够运算
                    uint16_t crc =  CRC16((uint8_t* )FrameData, len + 3);
                    ClrWdt();
                    uint8_t  crcL = FrameData[len + 3];
                    uint8_t  crcH = FrameData[len + 4];
                    ClrWdt();
                    //若校验吻合,则执行下一步动作
                    if (crc == ((uint16_t)crcH<<8  | crcL))
                    {
                        pReciveFrame->pData = (uint8_t*)FrameData;
                        pReciveFrame->completeFlag = TRUE;
                    }
                    ClrWdt();
                    StopTimer2();  //超时检测结束  100ns以内处理一帧数据若超时则丢弃                
                    DealStep = 0;
                }
                break;
            }
            default:
            {
                DealStep = 0;
                ReciveIndex = 3;
                break;
            }
        }
    }
    ClrWdt();
    return g_ReciveBufferLen; //返回剩余长度
}

/**************************************************
 *函数名：  GenRTUFrame()
 *功能：  生成通信帧
 *形参： 目的地址 uint8_t addr
 *       功能代号 uint8_t funcode
 *       数据数组  uint8_t sendData[]
 *       数据长度  uint8_t datalen
 *       生成的帧指针 uint8_t* pRtuFrame
 *       帧总长度 uint8_t *plen
 *返回值：void
****************************************************/
void  GenRTUFrame(uint8_t addr, uint8_t funcode,
             uint8_t sendData[], uint8_t datalen, uint8_t* pRtuFrame, uint8_t *plen)
{
    uint8_t len = 1 + 1 +  + 1 + datalen + 2;
    *plen = len;

    pRtuFrame[0] = addr;
    pRtuFrame[1] = funcode;
    pRtuFrame[2] = datalen;
    ClrWdt();
    int i = 0;
    for ( i = 0; i < datalen;  i++)
    {
        ClrWdt();
        pRtuFrame[i + 3] = sendData[i];
    }
    ClrWdt();
    uint16_t crc =  CRC16(pRtuFrame, len - 2);
    pRtuFrame[len - 2] = (uint8_t)(crc & 0xFF);
    pRtuFrame[len - 1] = (uint8_t)(crc & 0xFF00 >> 8);
    ClrWdt();
    completeFlag = 0;
}

/**************************************************
 *函数名：  GenRTUFrame()
 *功能：  生成通信帧
 *形参： 目的地址 uint8_t addr
 *       功能代号 uint8_t funcode
 *       数据数组  uint8_t sendData[]
 *       数据长度  uint8_t datalen
 *       生成的帧指针 uint8_t* pRtuFrame
 *       帧总长度 uint8_t *plen
 *返回值：void
****************************************************/
void  GenRTUFrameCumulativeSum(uint8_t addr, uint8_t funcode,
             uint8_t sendData[], uint8_t datalen, uint8_t* pRtuFrame, uint8_t *plen)
{
    uint8_t len = 1 + 1 +  + 1 + datalen + 2;
    *plen = len;

    pRtuFrame[0] = addr;
    pRtuFrame[1] = funcode;
    pRtuFrame[2] = datalen;
    ClrWdt();
    int i = 0;
    for ( i = 0; i < datalen;  i++)
    {
        ClrWdt();
        pRtuFrame[i + 3] = sendData[i];
    }
    ClrWdt();
    
    PointUint8 Point;
    Point.pData = pRtuFrame;
    Point.len = len - 2;    
    uint16_t crc =  CumulativeSumCalculate(&Point);
    pRtuFrame[len - 2] = (uint8_t)(crc & 0xFF);
    pRtuFrame[len - 1] = (uint8_t)(crc  >> 8);
    ClrWdt();
    completeFlag = 0;
}
void SendFrame(uint8_t* pFrame, uint8_t len)
{
    uint8_t i = 0;
    RX_TX_MODE = TX_MODE;
    ClrWdt();
    __delay_us(500);//200
    ClrWdt();
    //__delay_ms(1);
    for ( i = 0 ; i < len; i++)
    {
        ClrWdt();
        UsartSend(pFrame[i]);
    }
    RX_TX_MODE = RX_MODE;
}


