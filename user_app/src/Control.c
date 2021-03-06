
/**
 * *********************************************************************
 *             Copyright (c) 2017 AFU  All Rights Reserved.
 * @file control.c
 * @version V1.0
 * @date 2017.7.12
 * @brief 设备控制函数文件
 *
 * *********************************************************************
 * @note
 * 2017.7.12
 *
 * *********************************************************************
 * @author 宋阳
 */



/* Includes ------------------------------------------------------------------*/
#include "user_comm.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#define DOOR_IN_OPEN()		  do {IO_H(DOOR_OPEN1); osDelay(20); IO_L(DOOR_OPEN1);} while(0)
#define DOOR_IN_IS_CLOSE()  (IO_READ(DOOR_S1) == 0)
#define DOOR_OUT_HOLD()		  IO_H(DOOR_OPEN2)
#define DOOR_OUT_RELEASE()	IO_L(DOOR_OPEN2)
#define DOOR_OUT_IS_TOUCH() (IO_READ(DOOR_S2) == 0)
#define DOOR_IS_REQOUT()    (IO_READ(DOOR_PS) == 0)
#define DOOR_PLAN2_IN()     (IO_H(RELAY1))
#define DOOR_PLAN2_OUT()     (IO_L(RELAY1))
/* Private variables ---------------------------------------------------------*/
uint32_t TS_Door = 0;
BOOL ScanStationStart =  FALSE, ClearChannelScan = FALSE, ScanChannelStart = FALSE, ReaportIR = FALSE, RFID_Reset = FALSE;
BOOL DOOR_PLAN = FALSE;//FALSE为电磁锁方案，TRUE为人体感应方案
/* Private function prototypes -----------------------------------------------*/
static void control_Console(int argc, char* argv[]);

/* Exported functions --------------------------------------------------------*/

/**
 * 夹娃娃机接口控制板初始化
 */
void Control_Init(void) {
    CMD_ENT_DEF(control, control_Console);
    Cmd_AddEntrance(CMD_ENT(control));
    DBG_LOG("Control Init.");
    if(IO_READ(MOTOR1_SC)) {
        DOOR_PLAN = TRUE;
        // DBG_LOG();
    } else {
        DOOR_PLAN = FALSE;
    }
}

/**
 * 控制门打开
 *
 * @param inout  1为进店开门,2为出开门
 */
void Control_DoorOpen(uint8_t inout) {
    if (inout == DOOR_OPEN_IN) {
        GPRS_TTS("您好！欢迎光临");
        if(DOOR_PLAN == FALSE) {
            DOOR_IN_OPEN();
            DOOR_OUT_HOLD();
        } else if(DOOR_PLAN == TRUE) {
            DOOR_PLAN2_IN();
        }
    } else if (inout == DOOR_OPEN_OUT)  {
        GPRS_TTS("小魔期待您的再次光临");
        DOOR_OUT_RELEASE();
    }
}

/**
 * 设备控制轮询函数
 */
void Control_Polling(void) {
    uint8_t door = 0;
    static uint8_t doorstatus = 0, reqout = 0;
    /*上报门锁状态*/
    door = (uint8_t)DOOR_IN_IS_CLOSE();
    if (door != doorstatus) {
        osDelay(100);
        door = (uint8_t)DOOR_IN_IS_CLOSE();
        if (door != doorstatus) {
            doorstatus = door;
            if (MQTT_IsConnected()) {
                // if(door)
                //     GPRS_TTS("小魔期待您的再次光临");
                if(DOOR_PLAN == FALSE)
                    Report_DoorStatus((door) ? "close" : "open");
                else if(DOOR_PLAN == TRUE) {
                    Report_DoorStatus((door) ? "open" : "close");
                    TS_INIT(TS_Door);
                }
            }
        }
    }
    if((DOOR_PLAN == TRUE) && (door == 0)) {
        osDelay(100);
        door = (uint8_t)DOOR_IN_IS_CLOSE();
        if(door == 0) {
            if(TS_IS_OVER(TS_Door, 1000) && (DOOR_OUT_IS_TOUCH() == 0)) {
                osDelay(100);
                if(DOOR_OUT_IS_TOUCH() == 0) {
                    GPRS_TTS("小魔期待您的再次光临");
                    DOOR_PLAN2_OUT();
                }
            }
            if(DOOR_OUT_IS_TOUCH() == 1) {
                osDelay(100);
                if(DOOR_OUT_IS_TOUCH() == 1) {
                    DOOR_PLAN2_IN();
                    TS_INIT(TS_Door);
                }
            }
        }
    }
    /*请求开门*/
    door = (uint8_t)DOOR_IS_REQOUT();
    if (door != reqout) {
        osDelay(100);
        door = (uint8_t)DOOR_IS_REQOUT();
        if (door != reqout) {
            reqout = door;
            if (MQTT_IsConnected() && reqout) {
                Report_DoorStatus("reqout");
            }
        }
    }
    /*上报RFID*/
    uint8_t buf[9];
    uint16_t voltage = 0;
    uint32_t rfid = 0;
    char rfidbuf[12];
    if (!RF_WTR() && RF_Receive(buf, 9, 100)) {
        if (buf[0] == 0xAF && AddCheck(buf, 8) == buf[8]) {
            CMD_HEX_Print(buf, 9);
            rfid = buf[4] << 24;
            rfid |= buf[5] << 16;
            rfid |= buf[6] << 8;
            rfid |= buf[7];
            voltage = buf[2] << 8;
            voltage |= buf[3];
            uitoa(rfid, rfidbuf);
            DBG_LOG("RF_Receive cmd:%#x, rfid:%s, add:%#x", buf[1], rfidbuf, buf[8]);
            if (MQTT_IsConnected()) {
                if (buf[1] == 0x03) {
                    Request_Price(rfidbuf);
                } else {
                    Report_Rope_RFID(rfidbuf, buf[1], voltage);
                }
            }
        } else {
            DBG_LOG("RF_Receive Check Failed:%#x.", buf[8]);
        }
    }
}

/* Private function prototypes -----------------------------------------------*/
/**
 * 接口控制板调试命令
 *
 * @param argc   参数个数
 * @param argv   参数列表
 */
static void control_Console(int argc, char* argv[]) {
    argv++;
    argc--;
    if (ARGV_EQUAL("polling")) {
        Control_Polling();
    } else if (ARGV_EQUAL("opendoor")) {
        Control_DoorOpen(uatoi(argv[1]));
        DBG_LOG("test door open.");
    }
}


