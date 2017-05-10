/*
 * File:   EEPROMOperate.c
 * Author: 张东旭
 *
 * Created on 2017年4月8日, 上午11:17
 */

#include "../Header.h"

/**
 * EEPROM地址
 */
#define EEPROM_STAR_ADDRESS 0x7FFC00    //EEPROM起始地址
#define EEPROM_END_ADDRESS  0x7FFC6F    //EEPROM结束地址
#define EEPROM_OFFSET_ADDRESS 0x04      //EEPROM偏移地址
#define EEPROM_ADDCHECK_ADDRESS 0x02    //EEPROM中累加和的偏移地址
#define ACCUMULATE_SUM_ADDRESS 0x7FFFFC   //累加和EEPROM地址

/**************************************************
 *函数名：WriteAWord()
 *形参： _prog_addressT addr, uint16_t* data
 *返回值：void
 *功能：  按地址写入一个word
****************************************************/
inline void WriteWord_EEPROM( _prog_addressT addr, uint16_t* data)
{
   //应禁止中断
    uint8_t cn = 0;
    ClrWdt();
    __builtin_disi(0x3FFF); 
    _erase_eedata(addr, _EE_WORD); /* erase the dat[] array */ 
    ClrWdt(); 
    cn = 0;        
    while(NVMCONbits.WR)
    {
        ClrWdt();
        __builtin_disi(0x3FFF); 
        __delay_us(100);
        if (cn++ >200) //超过20ms跳出
        {
            break;             
        }
    }
    __builtin_disi(0x3FFF); 
    _write_eedata_word(addr, data[0]); /* write a word to dat[0] */
    __builtin_disi(0x3FFF); 
    ClrWdt();
    cn = 0;
    while(NVMCONbits.WR)
    {
        ClrWdt(); 
        __builtin_disi(0x3FFF); 
        __delay_us(100);
        if (cn++ >200) //超过20ms跳出
        {
            break;            
        }
    }
    __builtin_disi(0);      
}

/**************************************************
 *函数名：WriteAWord()
 *形参： _prog_addressT addr, uint16_t* data
 *返回值：void
 *功能：  按地址读取一个word
****************************************************/
inline void ReadWord_EEPROM( _prog_addressT addr, uint16_t* data)
{
    ClrWdt();
    __builtin_disi(0x3FFF); 
    _memcpy_p2d16(data, addr, _EE_WORD );
    __builtin_disi(0); 
}
/**
 * 读取EEPROM
 * @param id  配置号
 * @param data 指向数据的指针
 */
void ReadEEPROM(uint8_t id,PointUint8* pPoint)
{    
    uint16_t readData = 0;
    uint8_t i = 0;
    _prog_addressT address;
    address = (_prog_addressT)(id * EEPROM_OFFSET_ADDRESS) + EEPROM_STAR_ADDRESS;
    for(i = 0;i < pPoint->len;i += 2)
    {
        ReadWord_EEPROM(address + i,&readData);
        pPoint->pData[i] = (readData & 0x00FF); //低位先读取
        if((pPoint->len % 2) == 0)  //对长度进行奇数校验
        {
            pPoint->pData[i + 1] = (readData >> 8);               
        }
    }
}
/**
 * 写入EEPROM
 * @param id  配置号
 * @param data 指向数据的指针
 */
void WriteEEPROM(uint8_t id,PointUint8* pPoint)
{
    uint16_t data[2] = {0,0};
    uint8_t i = 0;
    _prog_addressT address;
    address = (_prog_addressT)(id * EEPROM_OFFSET_ADDRESS) + EEPROM_STAR_ADDRESS;
    for(i = 0;i < pPoint->len;i += 2)
    {
        if((pPoint->len % 2) == 0)  //对长度进行奇数校验
        {
            data[i] = pPoint->pData[i + 1];               
        }
        data[i] = data[i] << 8 | pPoint->pData[i];
        WriteWord_EEPROM(address + i,&data[i]);
    }
}
