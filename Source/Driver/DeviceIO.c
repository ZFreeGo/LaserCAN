/** 
 * <p>application name： DeviceIO.c</p> 
 * <p>application describing： 对单片机的IO口进行初始化</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.07.07</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */
#include "../Header.h"
#include "DevicdIO.h"

/**
 * 
 * <p>Function name: [InitDeviceIO]</p>
 * <p>Discription: [初始化基本的IO端口]</p>
 */
void InitDeviceIO(void)
{
    ADPCFG = 0xFFFF; //模拟端口全部作为数字端口
    ClrWdt();
    
   
}
/**
 * 
 * <p>Function name: [Delay_ms]</p>
 * <p>Discription: [中间插入喂狗指令的可变长延时函数]</p>
 * @param ms 延时时间ms
 */
void Delay_ms(uint16_t ms)
{
    while(ms--)
    {
        ClrWdt();
        __delay_ms(1);
    }
}
