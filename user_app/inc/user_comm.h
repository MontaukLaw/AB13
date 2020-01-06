/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file uaer_comm.h
 * @author 宋阳
 * @version V1.0
 * @date 2016.4.1
 * @brief 用户公用头文件.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _USER_COMM_H
#define _USER_COMM_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

#include "prjlib.h"

#include "system.h"
#include "dfu.h"
#include "console.h"
#include "http.h"

#include "spi_flash.h"
#include "rtc_ext.h"
#include "uart.h"
#include "w5500_comm.h"
#include "wifi.h"
#include "bc26.h"
// #include "gprs.h"
// #include "ec20.h"
#include "bt.h"

#include "EC20.h"
#include "cjson.h"
#include "mqtt_conn.h"
#include "datasave.h"
#include "process.h"
#include "control.h"

#include "radio_a7139.h"

#include "RelayComProtocal.h"

/* Exported define -----------------------------------------------------------*/
/*调试串口号，为0时关闭打印输出*/
#define  DEBUG               5

//#define DEBUG_BACKUP        4

//#define UART_SP232_PORT     2

#define  BT_TEST_PORT        3

//#define BT_TEST_PORT        4

/*DEBUG 信息等级动态可设置使能*/
#define LOG_LEVEL_DYNAMIC   1

/*DEBUG 信息默认等级,须设置为常量数字宏才能展开*/
#define LOG_LEVEL_DEF       4

#define M4G_UART_PORT       2


/*定义DFU标记BKP寄存器*/
#define DFU_BKP             (BKP->DR10)

/*定义复位标识记数BKP*/
#define NRST_BKP            (BKP->DR9)
#define IWDG_BKP            (BKP->DR8)
#define SWRST_BKP           (BKP->DR7)
#define PORRST_BKP          (BKP->DR6)

/*使能flash读写保护*/
#define FLASH_WRP_EN        0

/*使能硬件看门狗*/
#define IWDG_HW_EN          0

#define PRODUCT_CHOSE       1

#define PROJECT             "AB-13"
#define VERSION             "AB-13_FM_V1.0"
#define VERSION_HARDWARE    "AB-13_HD_V1.0"

//这个案子根据文档, mqtt服务器信息如下:

#define MQTT_TLS_EN             0
#define PROD_ENV       0
     
#if PROD_ENV
//生产环境
#define MQTT_SERVER_DEF         "116.62.93.197"
#define MQTT_CLIENT_ID_DEF       "M047055000000010"
#define MQTT_PORT_DEF           1883
#define DEVICE_KEY               "1202842998612324353-Y2Y0MTM0ODgt"
#define MQTT_USER_DEF           "MHC"
#define MQTT_PWD_DEF            "f96a326b-7611-4533-8093-f01281884dbf"
#else     
//开发环境
#define MQTT_SERVER_DEF         "115.233.220.92"
#define MQTT_PORT_DEF           1883
#define MQTT_CLIENT_ID_DEF      "M047057000000002"
#define MQTT_USER_DEF           "admin"
#define MQTT_PWD_DEF            "public" 
#define DEVICE_KEY               "1196246406932361216-NzA0MjRhM2Mt"

#endif

//M047057000000001
//#define MQTT_CLIENT_ID_DEF    "MHC000000000005"
//#define MQTT_TLS_EN         0
//#define MQTT_SERVER_DEF     "47.107.124.226"
//#define MQTT_PORT_DEF       1883
//#define MQTT_USER_DEF       "guangxin"
//#define MQTT_PWD_DEF        "guangxin"

// #define MQTT_TLS_EN         0
// #define MQTT_SERVER_DEF     "47.107.107.103"
// #define MQTT_PORT_DEF       1883
// #define MQTT_USER_DEF       "caihua"
// #define MQTT_PWD_DEF        "123456"


#define UART1_RECEVIE_BUFFER_SIZE   0
#define UART2_RECEVIE_BUFFER_SIZE   2048
#define UART3_RECEVIE_BUFFER_SIZE   2048
#define UART4_RECEVIE_BUFFER_SIZE   0
#define UART5_RECEVIE_BUFFER_SIZE   128
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define DBG_LEVEL_ERR           1
#define DBG_LEVEL_WAR           2
#define DBG_LEVEL_LOG           3
#define DBG_LEVEL_INFO          4
#define DBG_LEVEL_DEBUG         5
#define DBG_LEVEL_TEMP          6

/* Exported macro ------------------------------------------------------------*/

/*启用DEBUG信息*/
#if DEBUG > 0

/*DEBUG 信息等级动态*/
#if LOG_LEVEL_DYNAMIC > 0
#define DBG_ERR(format, ...)    CMD_Printf_Level(DBG_LEVEL_ERR, "error> file:%s line:%d "format"\r\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define DBG_WAR(format, ...)    CMD_Printf_Level(DBG_LEVEL_WAR, "warring> "format"\r\n", ##__VA_ARGS__)
#define DBG_LOG(format, ...)    CMD_Printf_Level(DBG_LEVEL_LOG, "log> "format"\r\n", ##__VA_ARGS__)
#define DBG_INFO(format, ...)   CMD_Printf_Level(DBG_LEVEL_INFO, "inf> "format"\r\n", ##__VA_ARGS__)
#define DBG_DBG(format, ...)    CMD_Printf_Level(DBG_LEVEL_DEBUG, "dbg> "format"\r\n", ##__VA_ARGS__)
#define DBG_TEMP(format, ...)   CMD_Printf_Level(DBG_LEVEL_TEMP, "temp> "format"\r\n", ##__VA_ARGS__)

/*DEBUG 信息等级静态*/
#else
#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_ERR))
#define DBG_ERR(format, ...)    CMD_Printf("error> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_ERR(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_WAR))
#define DBG_WAR(format, ...)    CMD_Printf("warring> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_WAR(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_LOG))
#define DBG_LOG(format, ...)    CMD_Printf("log> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_LOG(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_INFO))
#define DBG_INFO(format, ...)   CMD_Printf("inf> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_INFO(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_DEBUG))
#define DBG_DBG(format, ...)    CMD_Printf("dbg> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_DBG(format, ...)
#endif

#if ((LOG_LEVEL_DEF > 0) && (LOG_LEVEL_DEF <= DBG_LEVEL_TEMP))
#define DBG_TEMP(format, ...)   CMD_Printf("temp> "format"\r\n", ##__VA_ARGS__)
#else
#define DBG_TEMP(format, ...)
#endif
#endif

#define DBG_HEX(dat, len)                       CMD_HEX_Print(dat, len)
#define DBG_SEND(dat, len)                      CMD_SendData(dat, len)
#define THROW(str)                              UART_SendData_Direct(DEBUG, (uint8_t *)str, strlen_t(str))
#define THROW_PRINTF(format, ...)               UART_Printf_Direct(DEBUG, "> "format"\r\n",  ##__VA_ARGS__);

#define DBG_PRINT(level, format, ...)           CMD_Printf_Level(level, format"\r\n", ##__VA_ARGS__)
#define DBG_PRINTBUF(level, format, buf, len)   do \
    {\
        CMD_Printf_Level(level, "%s\r\n", format);\
        CMD_HEX_Print_Level(level, buf, len);\
    } while (0)
/*禁用DEBUG信息*/
#else
#define DBG_ERR(format, ...)
#define DBG_WAR(format, ...)
#define DBG_LOG(format, ...)
#define DBG_INFO(format, ...)
#define DBG_DBG(format, ...)
#define DBG_TEMP(format, ...)
#define DBG_SEND(dat, len)
#define THROW(str)
#define DBG_HEX(dat, len)
#define DBG_PRINT(level, format, ...)
#define DBG_PRINTBUF(level, format, buf, len)

#endif
        
#define I_AM_IDIOT    0
/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/



#endif
