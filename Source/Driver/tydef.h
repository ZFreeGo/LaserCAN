#ifndef _TYDEF_H_
#define _TYDEF_H_

typedef unsigned char       boolean;

#include <stdint.h> 

//typedef short           int8_t;
//typedef short           int16_t;
//typedef long            int32_t;
//typedef unsigned char   uint8_t;
//typedef unsigned short  uint16_t;
//typedef unsigned long   uint32_t;
typedef float           float32_t;
typedef long double     float64_t;
//
//#define UINT8_MAX                   255
//#define UINT16_MAX                65535
//#define UINT32_MAX           4294967295u

#define FALSE 0
#define TRUE 0xFF


/**
 *数据指针结构
 */

typedef struct TagPointUint8
{
    uint8_t* pData; //数据指针
    uint8_t len; //指向数据长度
}PointUint8;





#endif
