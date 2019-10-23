/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file Process.h
 * @author 宋阳
 * @version V1.0
 * @date 2016.8.31
 * @brief 业务逻辑处理函数头文件.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _PROCESS_H
#define _PROCESS_H


/* Includes ------------------------------------------------------------------*/
#include "prjlib.h"
#include "cjson.h"
//#include "RelayComProtocal.h"
     
/* Exported define -----------------------------------------------------------*/
#define SET_CMD_CACHE_QUANTITY          50
#define SET_CMD_CACHE_ARRAY_LENGTH      (SET_CMD_CACHE_QUANTITY*3)
#define REG_TYPE                        uint16_t
#define PHEASANTRY_MAX                  16
#define MODBUS_SLAVE_ADDR               0x01//从机地址码
#define MODBUS_READ_COIL                0x01//读线圈寄存器功能码（可多个操作）
#define MODBUS_WRITE_COIL               0x05//写线圈寄存器功能码（写单个线圈）
#define MODBUS_READ_REG                 0x03//读保持寄存器功能码（可多个操作）
#define MODBUS_WRITE_REG                0x06//写保持寄存器功能码（写单个保持寄存器）

#define SETCMD_REG_QUANTITY             351
#define HOMEPAGE_REG_QUANTITY           8
//#define OTHERDATA_REG_QUANTITY          568
#define OTHERDATA_REG_QUANTITY          650
#define ALARMDATA_REG_QUANTITY          38
#define UPDATA_REG_AMOUNT               (HOMEPAGE_REG_QUANTITY + OTHERDATA_REG_QUANTITY)
#define UPDATA_LENGTH                   (UPDATA_REG_AMOUNT * 2)
#define UPDATA_ARRAYSIZE                (UPDATA_LENGTH + 14)//(开始标志2byte，结束标志2byte，序列号4byte，时间戳4byte，校验码2byte)
#define UPDATA_FIRSTPLACE               10

#define  MAX_PACKAGE_SIZE     FIRSTPACKETSIZE
//#define FIRSTPACKETSIZE                 814//第一个数据包长度
#define FIRSTPACKETSIZE                 404//第一个数据包长度, 根据新的协议, 分为4个数据包, 第一个包数据长度为420-16 404个

//#define THESECONDPACKETSIZE             (UPDATA_LENGTH-FIRSTPACKETSIZE)//第二个数据包长度
#define THESECONDPACKETSIZE             384//第二个数据包长度, 根据新的协议, 400-16

#define THIRD_PACKAGE_SIZE              386//第三个数据包长度, 根据新的协议, 402-16

#define FORTH_PACKAGE_SIZE              274//第四个数据包长度, 根据新的协议, 290-16

//总共字节数1448
//之前是814 + 338 1152 字节

#define UART_IDLETICKOVER               4

#define TS_VOERFLOW(xTick,over)         ((HAL_GetTick() - xTick)>=over?1:0)
#define U16BIT_BYTESWAP(x)              ((x>>8)|(x<<8))
#define TWOBYTE_16BIT(H,L)              ((uint16_t)(H<<8)|(uint16_t)(L))
#define TWO16BIT_32BIT(H,L)              ((uint32_t)(H<<16)|(uint32_t)(L))
/* Exported types ------------------------------------------------------------*/
typedef enum {
    ret_ok,
    ret_fault,
    ret_timeout,
} proret_t;

typedef struct {
    uint32_t mid;
    uint16_t chid;
    uint16_t addr;
    uint16_t value;
} CmdTYPE_t;


/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/
void Process_Init(void);

BOOL CMD_Updata(char* cmd, cJSON* desired);

void Status_Updata(void);

BOOL publishReg(void);

void BT_Intercept_Proc(void);

//BOOL publishData(char *cmd, cJSON *data,uint32_t deviceIDNumber);
BOOL publishData(char *cmd, cJSON *data);

void beep(void);

#endif
