/* Includes ------------------------------------------------------------------*/
#include "user_comm.h"

// BT发送过来的消息长度
#define MSG_LENGTH    18

// 消息头, 消息尾, 厂商id
#define BT_MSG_HEAD  0xaa
#define BT_MSG_TAIL 0xbb
#define AFUIOT  0x5A

#define BT_UART_REFRESH_TICK   200
#define BT_RECEIVE_MAX_SIZE    1024

// 上报消息类型
#define STATUS_UPDATE        0x01   //     状态变更
#define RFID_ERROR_WARNING   0x02   //     异常报警
#define INVENTRY_UPDATE      0x03   //     盘库上报
#define MOVEING_INTERFACE    0x05   //    移车接口

void BT_Task(void const *argument);
static void BTCmdHanlde();
static void bt_Console(int argc, char *argv[]);
int16_t BT_Console(uint8_t *data, uint16_t len);
int16_t BT_SocketSendData(uint8_t *data, uint16_t len);
void BT_Intercept_Proc(void);

TaskHandle_t BTtaskHandle;

static void sendInventoryUpdate(uint8_t deviceStatus);
static void rfidErrorUpdate(uint8_t highByte, uint8_t lowByte, uint32_t labelId);
static void stateChangedUpdate(uint8_t targetStatus, uint8_t initialStatus, uint32_t labelId);
static void sendMovingEvent(uint8_t gSensorStatus, uint32_t labelId, uint8_t deviceStatus);

uint64_t timeStamp64;

//uint8_t seed[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b' }
//0x41-0x5A 0x61-0x7a 0x30-0x39
//26+26+10

/**
 * BT初始化
 */
void bt_Init() {
	//genUUID();
	osThreadDef(btTask, BT_Task, osPriorityNormal, 0, 1500)
	;
	BTtaskHandle = osThreadCreate(osThread(btTask), NULL);

//osMessageQDef(BT_SendQ, BT_SEND_Q_SIZE, void*);
//BT_SendQId = osMessageCreate(osMessageQ(BT_SendQ), NULL);

//osThreadDef(BT, BT_Task, BT_TASK_PRIO, 0, 512);
//osThreadCreate(osThread(BT), NULL);
//if(xTaskCreate(&InquireTask, "InquireTask", 1024, NULL, 3, &Inquiretask) != pdPASS)
//	if (xTaskCreate(BT_Task, "BT_Task", BT_TASK_STK_SIZE, NULL, osPriorityHigh,
//			&BTtaskHandle) != pdPASS)
//		DBG_LOG("Create BT_Task failure");
//	else
//		DBG_LOG("Create BT_Task ok");

//osThreadDef(M4G, M4G_Task, M4G_TASK_PRIO, 0, M4G_TASK_STK_SIZE);
//osThreadCreate(osThread(M4G), NULL);

#if BT_CMD_EN > 0
//BT_Pipe = CMD_Pipe_Register((CMD_SendFun)BT_SocketSendData);
//DBG_LOG("BT CMD pipe is %d.", BT_Pipe);
#endif

//CMD_ENT_DEF(bt, bt_Console);
//Cmd_AddEntrance(CMD_ENT(bt));

	DBG_LOG("BT Init.");
}

void BT_Task(void const *argument) {

	TWDT_DEF(BTTask, 60000);
	TWDT_ADD(BTTask);
	TWDT_CLEAR(BTTask);
	uint8_t testCounter = 0;
	uint32_t btcmdtick = 0;
//BTtaskHandle = xQueueCreate(50, sizeof(CmdTYPE_t));

	while (1) {
		osDelay(1);

		if (TS_VOERFLOW(btcmdtick, 1000)) {
			btcmdtick = HAL_GetTick();
			//BTCmdHanlde();
			testCounter++;
		}
		if (testCounter > 15) {
			DBG_LOG("bt task running");
			testCounter = 0;
		}
		TWDT_CLEAR(BTTask);
		BT_Intercept_Proc();
	}

}

static void BTCmdHanlde(void) {

}

int16_t BT_SocketSendData(uint8_t *data, uint16_t len) {
	return 0;
}

static void bt_Console(int argc, char *argv[]) {

}

uint16_t reportTick = 0;
uint16_t lastReportTick = 0;
// 检查是否可以发送
uint8_t ifSendingGreenLight() {
	uint16_t timePassed = 0;
	if (MQTT_IsConnected()) {
		if (HAL_GetTick() > lastReportTick) {
			timePassed = HAL_GetTick() - lastReportTick;
			if (timePassed > 5000) {
				DBG_LOG("Green Light");
				lastReportTick = HAL_GetTick();
				return 1;
			}
		}
	}
	return 0;
}
// 协议分析
// Enhanced ShockBurs 协议DPL模式
//const uint8_t PIPE0_RX_ADDRESS[RX_ADR_WIDTH]={0x78,0x78,0x78,0x78,0x78};
//u_int8_t msg[MSG_LENGTH] = { 0xaa, 0x5a, 0, 0, 0, 0, BATTER_LOW, 0, 0, 0, 0, 0, 0x01, 0, 0, 0x10, 0, 0xbb };
//测试用: Aa 5A 00 00 71 00 00 08 00 00 00 00 00 00 2c 50 bb
// 包头 厂商id 标签id4byte 电量 旧状态 新状态 错误码2byte 陀螺仪状态 上报类型 预留2byte rssi 校验和 包尾
// 4种上报类型 0x01: 状态变更 0x02:	异常报警 0x03:盘库上报 0x05:移车上报
uint8_t msg[MSG_LENGTH] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static uint8_t anylizeBTCmd(uint8_t *cmd, uint16_t cmdLength) {
	uint8_t oldState, newState, gSensorStatus;
	uint8_t rfErrorHigh, rfErrorLow;
	uint32_t labelId = 0;
	if (cmdLength >= MSG_LENGTH) {
		DBG_LOG("rssi is %d", *(cmd + 15));
		// 校验一下吧, 以防万一, 没毛病
		if (*cmd == BT_MSG_HEAD && (*(cmd + MSG_LENGTH - 1) == BT_MSG_TAIL) && (*(cmd + 1) == AFUIOT)) {
			beep();
			oldState = *(cmd + 7);
			newState = *(cmd + 8);
			labelId = ((uint32_t)(*(cmd + 2))) << 24 || ((uint32_t)(*(cmd + 3))) << 16 || ((uint32_t)(*(cmd + 4))) << 8 || ((uint32_t)(*(cmd + 5)));

			// 检查数据上报类型
			switch (*(cmd + 12)) {
			// 状态异常
			case STATUS_UPDATE:
				if (ifSendingGreenLight()) {
					stateChangedUpdate(oldState, newState, labelId);
				}

				break;
				// 标签预警
			case RFID_ERROR_WARNING:
				if (ifSendingGreenLight()) {
					rfErrorHigh = *(cmd + 9);
					rfErrorLow = *(cmd + 10);
					rfidErrorUpdate(rfErrorHigh, rfErrorLow, labelId);
				}

				break;

			case INVENTRY_UPDATE:
				if (ifSendingGreenLight()) {
					sendInventoryUpdate(newState);
				}
				break;

			case MOVEING_INTERFACE:
				if (ifSendingGreenLight()) {
					gSensorStatus = *(cmd + 11);
					sendMovingEvent(gSensorStatus, labelId, newState);
				}
				break;

			default:
				break;
			}

			//case
			//DBG_LOG("My package");

		}
	}

	return 0;

}

/**
 * BT 串口数据监听处理
 */
void BT_Intercept_Proc(void) {
	uint8_t *pbuf = NULL;
	uint16_t len = 0;
	len = UART_DataSize(BT_TEST_PORT);
	if (len == 0) {
		return;
	}
	if ((UART_QueryByte(BT_TEST_PORT, len - 1) == BT_MSG_TAIL && UART_QueryByte(BT_TEST_PORT, len - (MSG_LENGTH - 1)) == BT_MSG_HEAD
			&& UART_GetDataIdleTicks(BT_TEST_PORT) >= 20) || UART_GetDataIdleTicks(BT_TEST_PORT) >= BT_UART_REFRESH_TICK) {
		if (len >= (BT_RECEIVE_MAX_SIZE - 1)) {
			len = M4G_RECEIVE_MAX_SIZE - 1;
		}
		pbuf = MMEMORY_ALLOC(len + 1);
		if (pbuf != NULL) {
			len = UART_ReadData(BT_TEST_PORT, pbuf, len);
			*(pbuf + len) = 0;

			anylizeBTCmd(pbuf, len);
			MMEMORY_FREE(pbuf);
		}
	}
}

static void sendInventoryUpdate(uint8_t deviceStatus) {
	cJSON *data = NULL;
	data = NULL;
	data = cJSON_CreateObject();
	if (data != NULL) {

		cJSON_AddNumberToObject(data, "deviceStatus", deviceStatus);

		cJSON_AddStringToObject(data, "labelId", "1169159667509538816-MThjNTIxMTQt");

		cJSON_AddNumberToObject(data, "dateTime", 1569484973000);
	}
	publishData("U1103", data);

//cJSON_Delete(data);
}

static void rfidErrorUpdate(uint8_t highByte, uint8_t lowByte, uint32_t labelId) {
	cJSON *data = NULL;
	data = NULL;
	data = cJSON_CreateObject();
	uint16_t errorCode = (uint16_t) highByte << 8 || lowByte;

	if (data != NULL) {

		cJSON_AddStringToObject(data, "labelId", "1169159667509538816-MThjNTIxMTQt");

		cJSON_AddNumberToObject(data, "errorCode", errorCode);
	}
// 标签错误cmd是u1102
	publishData("U1102", data);

//cJSON_Delete(data);
}

static void stateChangedUpdate(uint8_t targetStatus, uint8_t initialStatus, uint32_t labelId) {
	cJSON *data = NULL;
	data = NULL;
	data = cJSON_CreateObject();
	if (data != NULL) {
		cJSON_AddNumberToObject(data, "targetStatus", targetStatus);

		cJSON_AddNumberToObject(data, "initialStatus", initialStatus);

		cJSON_AddStringToObject(data, "labelId", "1169159667509538816-MThjNTIxMTQt");

		if (timeStamp64 != 0) {
			cJSON_AddNumberToObject(data, "dateTime", timeStamp64);

		} else {
			cJSON_AddNumberToObject(data, "dateTime", 1569484973000);
		}
	}
	publishData("U1101", data);

//cJSON_Delete(data);
}

static void sendMovingEvent(uint8_t gSensorStatus, uint32_t labelId, uint8_t deviceStatus) {
	cJSON *data = NULL;
	data = NULL;
	data = cJSON_CreateObject();
	if (data != NULL) {
		cJSON_AddNumberToObject(data, "deviceStatus", deviceStatus);

		cJSON_AddNumberToObject(data, "gyroscopeStatus", gSensorStatus);

		cJSON_AddStringToObject(data, "labelId", "1169159667509538816-MThjNTIxMTQt");

		cJSON_AddNumberToObject(data, "dateTime", 1569484973000);
	}
	publishData("U1105", data);

}
