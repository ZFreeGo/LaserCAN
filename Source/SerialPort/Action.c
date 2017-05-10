/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:Action.c
*文件标识:
*创建日期： 2015年月4月23日
*摘要:

*2015/4/23:套用历史版本框架.
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#include "../Header.h"

void SendAckMesssage(uint8_t fun);
void SendErrorFrame(uint8_t receiveID,uint8_t errorID);

extern uint8_t volatile SendFrameData[SEND_FRAME_LEN];



//分合状态指令 单片机在看门狗复位的情况下不会改变该值
_PERSISTENT uint16_t ReceiveStateFlag;  

//预制指令超时时间
uint16_t OverTime = 0;

//预制指令超时时间计数器
uint16_t OverTimeCn = 0;

/**************************************************
 *函数名： SendAckMesssage()
 *功能：  回传校验码
 *形参：  uint16_t fun 功能代码地址
 *返回值：void
****************************************************/
inline void SendAckMesssage(uint8_t fun)
{
    uint16_t len = 0;
    ClrWdt();
    GenRTUFrame(LOCAL_ADDRESS, ACK, &fun, 1, (uint8_t*)SendFrameData, (uint8_t *)&len);
    ClrWdt();
    SendFrame((uint8_t*)SendFrameData, len);
    ClrWdt();
}
/**************************************************
 *函数名： ExecuteFunctioncode()
 *功能：  执行功能代码
 *形参：  接收帧指针 frameRtu* pRtu
 *返回值：void
****************************************************/
void ExecuteFunctioncode(frameRtu* pRtu)
{
    ClrWdt();
    //该数据帧未处理过
    if (pRtu->completeFlag == TRUE)
    {
       // LEDE ^= 1;
        if ( pRtu->funcode != YONGCI_WAIT_HE_ACTION)
        {
            SendAckMesssage( pRtu->funcode);
        }
         ClrWdt();
        switch(pRtu->funcode)
        {
            case RESET_MCU:
            {
                Reset(); //软件复位
                break;
           
            default :
            {
                break;
            }
            }
        }
    pRtu->completeFlag = FALSE;
    }
}

