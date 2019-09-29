#ifndef _BT_H
#define _BT_H

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "prjlib.h"
#include "system.h"
#include "console.h"
#include "common.h"

#define BT_TASK_STK_SIZE          512
#define BT_TASK_PRIO              osPriorityNormal
#define BT_SEND_Q_SIZE            8

#define BT_UART_BDR               115200
//#define BT_UART_REFRESH_TICK      200

#define BT_SEND_MAX_SIZE          1500
//#define BT_RECEIVE_MAX_SIZE       2048

#define BT_CMD_EN                  1

void bt_Init();

#endif

