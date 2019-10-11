/**
 * *********************************************************************
 *             Copyright (c) 2019 AFU.com. All Rights Reserved.
 * @file Process.c
 * @version V1.1
 * @date 2018.9.18
 * @brief 主业务处理
 *
 * *********************************************************************
 * @note
 *
 * *********************************************************************
 * @author 罗勇改写.
 * 因为STM32RC有多达64K flash, 考虑多写点儿全局数组, 让代码更具可读性
 */

/* Includes ------------------------------------------------------------------*/
#include "user_comm.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

// U1101 - 状态变更即时上报     
#define  STATUS_CHANGE   1

#define TIMESTAMP_LENGTH 13

#define UUID_LENGTH     32

#define BATTER_LOW           0xf1   //

#define DEVICE_ID_LENGTH   16

/* Private variables ---------------------------------------------------------*/
static char subscribeTopic[36], publishTopic[36], playid[48];
// static uint32_t Report_ts = 0;
//static uint8_t UpDataArray[UPDATA_LENGTH];
static QueueHandle_t CmdHandle = NULL;
// static CmdTYPE_t CmdType;
// static SetCmd_t  SetCmd;
uint8_t titi = 0;
//long unixTimeLong = 0;

/* Private function prototypes -----------------------------------------------*/
static void ArrivePath(uint8_t *dat, uint16_t len);
// static proret_t ArriveDataProc(char* cmd, cJSON* desired);
static BOOL CMD_Confirm_Rsp(uint32_t ordermsgid, proret_t ret);
static void process_Console(int argc, char *argv[]);
// static void xReceiveInquirePara(Controller_t *xController);
static uint16_t WaitModbusReply_Single(const uint8_t *wcmd, uint32_t wtick);

static BOOL UpChickenStatus(uint32_t umessageid);
static void UpAlarmParameter(uint8_t uanum, uint32_t umessageid);
static void DataAnalysis(uint32_t messageid, cJSON *dis);
static void DeviceQueryAnalysis(uint32_t messageid, cJSON *dis);

static uint8_t anylizeBTCmd(uint8_t *cmd, uint16_t cmdLength);

uint32_t labelId;
//BOOL publishData(char *cmd, cJSON *data);
BOOL publishData(char *cmd, cJSON *data, uint32_t deviceIdNmb);

//static void rfidErrorUpdate(uint8_t highByte, uint8_t lowByte, uint32_t labelId);
//static void stateChangedUpdate(uint8_t targetStatus, uint8_t initialStatus,
//static void sendInventoryUpdate(uint8_t deviceStatus);
//static void sendMovingEvent(uint8_t gSensorStatus, uint32_t labelId,

uint8_t deviceStatus;
//static void CmdHanldeLogic(void);
void InquireTask(void *argument);
//发布心跳包
BOOL publishHeartBeat(void);
void genUUID(void);
static uint16_t mqttOFFLineCounter;
TaskHandle_t Inquiretask;
//不应该出现在这儿, but...
//void beep(void);
/* Exported functions --------------------------------------------------------*/
uint8_t uuid[UUID_LENGTH + 1];
extern uint64_t timeStamp64;

/**
 * 处理任务初始化
 */
void Process_Init(void) {

	// 要订阅的主题是: D/MHC000000000005
	strcpy(subscribeTopic, "D/");
	strcat(subscribeTopic, WorkParam.mqtt.MQTT_ClientID);
	// 用于发布的主题是: U/report
	strcpy(publishTopic, "U/report");

	CMD_ENT_DEF(process, process_Console);
	Cmd_AddEntrance(CMD_ENT(process));
	// 这个服务器要求QoS2
	//Subscribe_MQTT(subscribeTopic, QOS2, ArrivePath);
	Subscribe_MQTT(subscribeTopic, QOS0, ArrivePath);
	// xTaskCreate(InquireTask, "InquireTask", 256, NULL, osPriorityNormal, &Inquiretask);

	if (xTaskCreate(&InquireTask, "InquireTask", 512, NULL, 3, &Inquiretask) != pdPASS)
		DBG_LOG("Create InquireAnalysis failure");
	else
		DBG_LOG("Create InquireAnalysis ok");

	DBG_LOG("Process Start.");
}

static cJSON *cjsonData = NULL;

void genUUID(void) {
	uint8_t i = 0;
	int flag;
	//uint8_t seed = srand();
	for (i = 0; i < UUID_LENGTH; i++) {
		flag = rand() % 3;
		switch (flag) {
		case 0:
			uuid[i] = 'A' + rand() % 26;
			break;
		case 1:
			uuid[i] = 'a' + rand() % 26;
			break;
		case 2:
			uuid[i] = '0' + rand() % 10;
			break;
		default:
			uuid[i] = 'x';
			break;
		}
	}
	uuid[UUID_LENGTH] = '\0';
}

void checkMqttStatus(void) {
	if (MQTT_IsConnected() == FALSE) {
		mqttOFFLineCounter++;
	} else {
		mqttOFFLineCounter = 0;
	}

	if (mqttOFFLineCounter > 3) {
		M4G_ModulePowerOn();
		mqttOFFLineCounter = 0;
	}
}

void InquireTask(void *argument) {
	TWDT_DEF(PTask, 60000);
	TWDT_ADD(PTask);
	TWDT_CLEAR(PTask);
	static uint8_t heatBeatCounter = 0;
	static uint16_t upcrc = 0;
	static uint32_t qtick = 0, cmdtick = 0, heartBeatTick = 0;

	static uint8_t testSendCounter = 0;
	CmdHandle = xQueueCreate(50, sizeof(CmdTYPE_t));

	while (CmdHandle == NULL)
		DBG_WAR("xQueueCreate failure");
	//idnumhex = GetIDToHex();

	//等待mqqt连接成功
	while (!MQTT_IsConnected()) {
		osDelay(100);
	}

	DBG_LOG("InquireTask Start");

	while (1) {
		//osDelay(10000);

		// publishHeartBeat();
		if (TS_VOERFLOW(cmdtick, 1000)) {
			cmdtick = HAL_GetTick();
			//CmdHanldeLogic();
			heatBeatCounter++;
			testSendCounter++;
		}
		if (heatBeatCounter > 10) {
			//publishHeartBeat();
			DBG_LOG("living..");
			heatBeatCounter = 0;
		}
		if (testSendCounter > 35) {
			if (MQTT_IsConnected()) {
				publishHeartBeat();
                
                //stateChangedUpdate(1, 2, 1);
                
			}

			// 下面的测试用例没问题
			//stateChangedUpdate(1,2);

			//这里先注释掉, 看看效果.
			//checkMqttStatus();
			testSendCounter = 0;
		}
		//BT_Intercept_Proc();
		TWDT_CLEAR(PTask);
	}

}

void clearArray(uint8_t *arr, uint16_t length) {
	uint16_t i = 0;
	for (i = 0; i < length; i++) {
		*(arr + i) = 0;
	}
}
// M030057000000001
char deviceIDStr[DEVICE_ID_LENGTH + 1] = "M030057000000001";

void transDeviceIDToStr(uint32_t deviceIdNmb) {
	uint8_t i = 0;
	uint8_t counter = 8;
    if(deviceIdNmb>0){
	    for (i = 8; i < DEVICE_ID_LENGTH; i++) {
		    deviceIDStr[i] = (deviceIdNmb / pow(10, (counter - 1))) + 0x30;
		    counter--;
	    }
    }
}

BOOL publishData(char *cmd, cJSON *data, uint32_t deviceIdNmb) {

	BOOL ret = FALSE;
	cJSON *root = NULL;
	char *s = NULL;
	root = cJSON_CreateObject();
	if (root != NULL) {
		genUUID();
		transDeviceIDToStr(deviceIdNmb);
		cJSON_AddStringToObject(root, "cmd", cmd);
		cJSON_AddStringToObject(root, "msgId", uuid);
		cJSON_AddStringToObject(root, "deviceId", deviceIDStr);
		cJSON_AddStringToObject(root, "protocol", "DevCommon_1.0");
		if (timeStamp64 != 0) {
			cJSON_AddNumberToObject(data, "time", timeStamp64);

		} else {
			cJSON_AddNumberToObject(data, "time", 1569484973000);
		}
		//cJSON_AddNumberToObject(root, "time", 1569484973000);

		if (data != NULL) {
			cJSON_AddItemToObjectCS(root, "data", data);
		}

		s = cJSON_PrintUnformatted(root);
		if (s != NULL) {
			//DBG_INFO("Send q to mqtt: data:%s", s);
			DBG_INFO("Sending q to mqtt");
			//ret = Publish_MQTT(publishTopic, QOS2, (uint8_t*)s, strlen(s));
			ret = Publish_MQTT(publishTopic, QOS0, (uint8_t*) s, strlen(s));

			MMEMORY_FREE(s);
		}
		cJSON_Delete(root);

	} else {

		cJSON_Delete(data);

	}
// 尾巴改一下
// changeMsgTatail();
	return ret;
}

// 文艺青年写第二遍
/*
 cmd: U0002
 msgId: msgId规则
 deviceId: 设备id
 protocol: DevCommon_1.0
 time: 当前系统时间毫秒值
 data:

 */
BOOL publishHeartBeat(void) {
	DBG_LOG("HeartBeat.");
	BOOL ret = FALSE;
	cJSON *root = NULL;
	char *s = NULL;
	root = cJSON_CreateObject();
	if (root != NULL) {
		//"cmd": "U0001"
		genUUID();
		cJSON_AddStringToObject(root, "cmd", "U0002");
		cJSON_AddStringToObject(root, "msgId", uuid);
		cJSON_AddStringToObject(root, "deviceId", WorkParam.mqtt.MQTT_ClientID);
		cJSON_AddStringToObject(root, "protocol", "DevCommon_1.0");
		if (timeStamp64 != 0) {
			cJSON_AddNumberToObject(root, "time", timeStamp64);
		} else {
			cJSON_AddNumberToObject(root, "time", 1569484973000);
		}

		s = cJSON_PrintUnformatted(root);
		if (s != NULL) {
			ret = Publish_MQTT(publishTopic, QOS0, (uint8_t*) s, strlen(s));
			MMEMORY_FREE(s);
		}
		cJSON_Delete(root);
	}
	return TRUE;
}

// 先试着写一个publish包, 测试通过后后面再进行重新封包
BOOL publishReg(void) {
	BOOL ret = FALSE;
	cJSON *root = NULL;
	char *s = NULL;
	root = cJSON_CreateObject();
	if (root != NULL) {
		//"cmd": "U0001"
		cJSON_AddStringToObject(root, "cmd", "U0001");
		cJSON_AddStringToObject(root, "msgId", "9e847b5c7164429d907c387c7522b8f3");
		cJSON_AddStringToObject(root, "deviceId", WorkParam.mqtt.MQTT_ClientID);
		cJSON_AddStringToObject(root, "protocol", "DevCommon_1.0");
		cJSON_AddNumberToObject(root, "time", 1569484973000);
		cJSON *data = NULL;
		data = cJSON_CreateObject();
		if (data != NULL) {
			cJSON_AddStringToObject(data, "secretKey", "1001734021054373888-MDhiOTgyMDgtZDg0ZC00MDZhLThkMDgtZTY3Mzk4NWRlNGU4");
		}
		cJSON_AddItemToObjectCS(root, "data", data);
		s = cJSON_PrintUnformatted(root);
		if (s != NULL) {
			DBG_INFO("CMD_Updata ts:%u,data:%s", HAL_GetTick(), s);
			ret = Publish_MQTT(publishTopic, QOS0, (uint8_t*) s, strlen(s));
			MMEMORY_FREE(s);
		}
		cJSON_Delete(root);
	}
	return ret;
}

uint8_t timestampArr[TIMESTAMP_LENGTH];

static void getTimeStamp() {
	uint8_t i;
	uint8_t counter = TIMESTAMP_LENGTH - 1;
	timeStamp64 = 0;
	for (i = 0; i < TIMESTAMP_LENGTH; i++) {

		timeStamp64 = timeStamp64 + ((timestampArr[i] - 0x30) * (uint64_t) pow(10, counter));
		counter--;
	}

}

static void ArrivePath(uint8_t *dat, uint16_t len) {
	char tail;
	uint8_t i = 0;
	uint16_t timeStampIndex;
// DBG_LOG("New Msg");
	cJSON *root = NULL, *responseId = NULL, *timestamp = NULL, *responseCmd =
	NULL;
	DBG_INFO("ArrivePath data:%s", dat);
//tail = *(dat + len - 1);
//DBG_INFO("last char is %c", tail);
	root = cJSON_Parse((const char*) dat);
	if (root != NULL) {
		responseCmd = cJSON_GetObjectItem(root, "responseCmd");
		if (responseCmd != NULL && responseCmd->type == cJSON_String) {
			//DBG_LOG("responseCmd:%s", responseCmd->valuestring);

			//cJSON_Delete(responseCmd);
		}

		timestamp = cJSON_GetObjectItem(root, "time");
		if (timestamp != NULL && timestamp->type == cJSON_Number) {
			//ts = timestamp->valueint;
			timeStampIndex = len - 14;
			for (i = 0; i < TIMESTAMP_LENGTH; i++) {
				timestampArr[i] = *(dat + timeStampIndex);
				timeStampIndex++;
			}
			DBG_LOG("timestamp is :%s", timestampArr);
			getTimeStamp();
			DBG_LOG("after trans, timestamp is %lld", timeStamp64);
		}

		cJSON_Delete(root);
	}
}

#if 0
cJSON* receiveRoot = NULL;
cJSON* responseCmd = NULL;
static void ArrivePath(uint8_t* dat, uint16_t len)
{
    receiveRoot = NULL,
    receiveRoot = cJSON_Parse((const char*)dat);
    if (receiveRoot != NULL) 
    {
        responseCmd = NULL;
        responseCmd = cJSON_GetObjectItem(receiveRoot, "responseCmd");
        if (responseCmd != NULL && responseCmd->type == cJSON_String){
            DBG_LOG("responseCmd:%s", responseCmd->valuestring);
            responseCmd = NULL;
        }
           
    }
        receiveRoot = NULL;
    //DBG_LOG("New Msg");
    
}
#endif

static void ArrivePath_old(uint8_t *dat, uint16_t len) {
	cJSON *root = NULL, *msgid = NULL, *timestamp = NULL;
	root = cJSON_Parse((const char*) dat);
	DBG_LOG("New Msg");
//DBG_INFO("ArrivePath ts:%u, data:%s", HAL_GetTick(), dat);
#if 0    
    if (root != NULL) 
    {
        msgid = cJSON_GetObjectItem(root, "msgId");
        DBG_LOG("msgid:%s", msgid->valuestring); 
        timestamp = cJSON_GetObjectItem(root, "time");
        DBG_LOG("timestamp:%s", timestamp->string); 
        if(timestamp != NULL && timestamp->type == cJSON_Number){
            unixTimeLong = (long)timestamp->valuedouble;
            DBG_LOG("timestamp:%f", unixTimeLong); 
        }      

        cJSON_Delete(root);          
    
    }  
#endif     
}

#if 0
/**
 * 接收处理
 *
 * @param dat    接收到的数据指针
 * @param len    数据长度
 */
static void ArrivePath(uint8_t* dat, uint16_t len) {
    uint8_t save = 0;
    uint32_t tsdiff = 0, ts = 0;
//    char temp[48] = { 0, 0 };
//    proret_t ret = ret_ok;
    cJSON* root = NULL, *msgid = NULL, *timestamp = NULL, *cmd = NULL, *desired = NULL, *deviceid = NULL, *child = NULL;;
    *(dat + len) = 0;
    DBG_INFO("ArrivePath ts:%u,data:%s", HAL_GetTick(), dat);
    root = cJSON_Parse((const char*)dat);
    if (root != NULL) {
        msgid = cJSON_GetObjectItem(root, "messageid");
        deviceid = cJSON_GetObjectItem(root, "deviceid");
        DBG_LOG("msgid:%d", cmd->valueint);
        DBG_LOG("deviceid:%s", cmd->valuestring);
        if (msgid != NULL && msgid->type == cJSON_Number
                && deviceid != NULL &&
                (strcmp(deviceid->valuestring, WorkParam.mqtt.MQTT_ClientID) == 0
                 || deviceid->valuestring[0] == '0')) {
            timestamp = cJSON_GetObjectItem(root, "timestamp");
            desired = cJSON_GetObjectItem(root, "desired");
            cmd = cJSON_GetObjectItem(root, "cmd");
            DBG_LOG("cmm:%s", cmd->valuestring);
            /*RTC校时*/
            if (timestamp != NULL && timestamp->type == cJSON_Number) {
                ts = timestamp->valueint;
                tsdiff = RTC_ReadTick();
                tsdiff = abs(ts - tsdiff);
            }
            if (STR_EQUAL(cmd->valuestring, "CMD-01")) {
                child = cJSON_GetObjectItem(desired, "devicereset");
                if (child != NULL && child->type == cJSON_True) {
                    NVIC_SystemReset();
                }
                child = cJSON_GetObjectItem(desired, "devicefactoryreset");
                if (child != NULL && child->type == cJSON_True) {
                    WorkParam.mqtt.MQTT_Port = MQTT_PORT_DEF;
                    WorkParam.mqtt.MQTT_PingInvt = MQTT_PING_INVT_DEF;
                    strcpy(WorkParam.mqtt.MQTT_Server, MQTT_SERVER_DEF);
                    strcpy(WorkParam.mqtt.MQTT_UserName, MQTT_USER_DEF);
                    strcpy(WorkParam.mqtt.MQTT_UserPWD, MQTT_PWD_DEF);
                    WorkParam_Save();
                }
                child = cJSON_GetObjectItem(desired, "deviceparamget");
                if (child != NULL && child->type == cJSON_True) {
                    Status_Updata();
                }
                child = cJSON_GetObjectItem(desired, "scanstation");
                if (child != NULL && child->type == cJSON_True) {
                    MCPU_ENTER_CRITICAL();
                    // ScanStationStart = TRUE;
                    MCPU_EXIT_CRITICAL();
                }
            }
            if (tsdiff < 30 || STR_EQUAL(cmd->valuestring, "CMD-02")) {
                child = cJSON_GetObjectItem(desired, "timestamp");
                if (child != NULL && child->type == cJSON_Number) {
                    timeRTC_t time;
                    RTC_TickToTime(child->valueint, &time);
                    RTC_SetTime(&time);
                }
                child = cJSON_GetObjectItem(desired, "ip");
                if (child != NULL && child->type == cJSON_String
                        && !STR_EQUAL(WorkParam.mqtt.MQTT_Server, child->valuestring)) {
                    strcpy(WorkParam.mqtt.MQTT_Server, child->valuestring);
                    save++;
                }
                child = cJSON_GetObjectItem(desired, "username");
                if (child != NULL && child->type == cJSON_String
                        && !STR_EQUAL(WorkParam.mqtt.MQTT_UserName, child->valuestring)) {
                    strcpy(WorkParam.mqtt.MQTT_UserName, child->valuestring);
                    save++;
                }
                child = cJSON_GetObjectItem(desired, "userpwd");
                if (child != NULL && child->type == cJSON_String
                        && !STR_EQUAL(WorkParam.mqtt.MQTT_UserPWD, child->valuestring)) {
                    strcpy(WorkParam.mqtt.MQTT_UserPWD, child->valuestring);
                    save++;
                }
                child = cJSON_GetObjectItem(desired, "port");
                if (child != NULL && child->type == cJSON_Number
                        && WorkParam.mqtt.MQTT_Port != child->valueint) {
                    WorkParam.mqtt.MQTT_Port = child->valueint;
                    save++;
                }
                child = cJSON_GetObjectItem(desired, "heartbeat");
                if (child != NULL && child->type == cJSON_Number
                        && WorkParam.mqtt.MQTT_PingInvt != child->valueint) {
                    WorkParam.mqtt.MQTT_PingInvt = child->valueint;
                    save++;
                }
                if (save > 0) {
                    WorkParam_Save();
                }
                CMD_Confirm_Rsp(msgid->valueint, ret_ok);
            } else {
                DBG_WAR("tsdiff error:%d", tsdiff);
            }
            if (STR_EQUAL(cmd->valuestring, "CMD-03")) {
                ChickenManagementAnalysis(msgid->valueint, desired);
            }
            if (STR_EQUAL(cmd->valuestring, "CMD-05")) {
                DeviceQueryAnalysis(msgid->valueint, desired);
            }
            if (STR_EQUAL(cmd->valuestring, "CMD-04")) {
                DataAnalysis(msgid->valueint, desired);
            }
        }
        cJSON_Delete(root);
    }
//    if (temp[0] != 0) {
//        // CMD_Confirm_Rsp(temp, ret);
//    }
}

#endif

void Status_Updata(void) {
	DBG_LOG("Ready to publish heartbeat.");
	publishHeartBeat();
}
/**
 * 通讯板参数与状态上传
 */
void Status_Updata_OLD(void) {
	cJSON *desired = NULL;
	desired = cJSON_CreateObject();
	if (desired != NULL) {
		cJSON_AddNumberToObject(desired, "timestamp", RTC_ReadTick());
		cJSON_AddStringToObject(desired, "ip", WorkParam.mqtt.MQTT_Server);
		cJSON_AddNumberToObject(desired, "port", WorkParam.mqtt.MQTT_Port);
		cJSON_AddNumberToObject(desired, "heartbeat", WorkParam.mqtt.MQTT_PingInvt);
		cJSON_AddStringToObject(desired, "project", PROJECT);
		cJSON_AddStringToObject(desired, "firmware", VERSION);
		cJSON_AddStringToObject(desired, "hardware", VERSION_HARDWARE);
		cJSON_AddStringToObject(desired, "status", "ok");
		CMD_Updata("CMD-102", desired);
	}
}
#if 0
/**
 * @brief  设置命令解析
 * @note
 * @param  messageid: 消息ID
 * @param  *dis:
 * @retval None
 */
void DataAnalysis(uint32_t messageid, cJSON *dis) {
    uint8_t chnum = 0;
    uint16_t cmdcacheconut = 0;
    uint8_t cmdsrt[5];
    CmdTYPE_t  *dcmdtype = NULL;
    // uint32_t cmdcache[20];
    cJSON *other = NULL;
    // memset(cmdcache, 0, 20 * sizeof(uint32_t));
    if(dis != NULL) {
        other = cJSON_GetObjectItem(dis, "number");
        if((other != NULL) && (other->type == cJSON_Number)) {
            chnum = (uint8_t)(other->valueint);
        }
        for(cmdcacheconut = 0; cmdcacheconut < SETCMD_REG_QUANTITY; cmdcacheconut++) {
            //other = cJSON_GetObjectItem(dis, uitoa(SetCmdReg[cmdcacheconut], (char*)cmdsrt));
            if((other != NULL) && (other->type == cJSON_Number)) {
                dcmdtype = MMEMORY_ALLOC(sizeof(CmdTYPE_t));
                if(dcmdtype != NULL) {
                    dcmdtype->mid = messageid;
                    dcmdtype->chid = chnum;
                    //dcmdtype->addr = SetCmdReg[cmdcacheconut];
                    dcmdtype->value = other->valueint;
                    xQueueSend(CmdHandle, dcmdtype, portMAX_DELAY);
                    MMEMORY_FREE(dcmdtype);
                }
                
            }
        }
    }
}



/**
 * @brief  查询命令解析
 * @note
 * @param  messageid: 消息ID
 * @param  *dis:
 * @retval None
 */
void DeviceQueryAnalysis(uint32_t messageid, cJSON *dis) {
    CmdTYPE_t  *qcmdtype = NULL;
    cJSON *other = NULL;
    if(dis != NULL) {
        other = cJSON_GetObjectItem(dis, "chicken");
        if((other != NULL) && (other->type == cJSON_Number)) {
            qcmdtype = MMEMORY_ALLOC(sizeof(CmdTYPE_t));
            if(qcmdtype != NULL) {
                qcmdtype->mid = messageid;
                qcmdtype->chid = other->valueint;
                qcmdtype->addr = 0xffff;
                qcmdtype->value = 0;
                xQueueSend(CmdHandle, qcmdtype, portMAX_DELAY);
                MMEMORY_FREE(qcmdtype);
            }
        }
        other = cJSON_GetObjectItem(dis, "alarm");
        if((other != NULL) && (other->type == cJSON_Number)) {
            qcmdtype = MMEMORY_ALLOC(sizeof(CmdTYPE_t));
            if(qcmdtype != NULL) {
                qcmdtype->mid = messageid;
                qcmdtype->chid = other->valueint;
                qcmdtype->addr = 0xfffe;
                qcmdtype->value = 0;
                xQueueSend(CmdHandle, qcmdtype, portMAX_DELAY);
                MMEMORY_FREE(qcmdtype);
            }
        }
    }
}

/**
 * 上传数据
 *
 * @param ordermsgid 上行的消息ID
 * @param ret        执行的结果
 * @return 向服务器上传命令执行结果
 */
static BOOL CMD_Confirm_Rsp(uint32_t ordermsgid, proret_t ret) {
    BOOL r = FALSE;
    cJSON* bodydesired;
    bodydesired = cJSON_CreateObject();
    cJSON_AddNumberToObject(bodydesired, "messageid", ordermsgid);
    if(ret == ret_ok)
        cJSON_AddStringToObject(bodydesired, "ret", "OK");
    else if(ret == ret_fault)
        cJSON_AddStringToObject(bodydesired, "ret", "FAULT");
    r = CMD_Updata("CMD-99", bodydesired);
    return r;
}
#endif

static void process_Console(int argc, char *argv[]) {

}

#if 0

/**
 * 锟斤拷锟斤拷锟斤拷锟斤拷
 * @param argc 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
 * @param argv 锟斤拷锟斤拷锟叫憋拷
 */
static void process_Console(int argc, char* argv[]) {
    uint8_t tttt[6] = {0x01, 0x01, 0x00, 0x32, 0x00, 0x01};
    static uint8_t yyyy[100];
    uint16_t ChickenNumber = 0;
    uint16_t crcv = 0;
    cJSON* desired = NULL;
    memset((char*)yyyy, 0xff, 100);
    argv++;
    argc--;
    if (ARGV_EQUAL("deviceid")) {
        if (argv[1] != NULL) {
            strcpy(WorkParam.mqtt.MQTT_ClientID, argv[1]);
            WorkParam_Save();
            strcpy(subscribeTopic, "/");
            strcat(subscribeTopic, PROJECT);
            strcat(subscribeTopic, "/");
            strcat(subscribeTopic, WorkParam.mqtt.MQTT_ClientID);
            strcpy(publishTopic, "/");
            strcat(publishTopic, PROJECT);
            strcat(publishTopic, "/");
            strcat(publishTopic, WorkParam.mqtt.MQTT_ClientID);
        }
        DBG_LOG("Device ID:%s", WorkParam.mqtt.MQTT_ClientID);
    } else if (ARGV_EQUAL("receive")) {
        ArrivePath((uint8_t*)argv[1], strlen(argv[1]));
    } else if (ARGV_EQUAL("send")) {
        desired = cJSON_Parse((const char*)argv[2]);
        if (desired != NULL) {
            CMD_Updata(argv[1], desired);
        }
    } else if (ARGV_EQUAL("crc")) {
        crcv = CRC_16(0xffff, tttt, 6);
        DBG_LOG("crc:%x", crcv);
    } else if(ARGV_EQUAL("GetChicken")) {
        ChickenNumber = GetChickenNumber();
        DBG_LOG("ChickenNumber:%x", ChickenNumber);
    } else if(ARGV_EQUAL("SetChicken")) {
        DBG_LOG("SetChickenNumber:%x", SetChickenNumber(3));
    } else if(ARGV_EQUAL("GetRegisterData")) {
        GetRegisterData(2001, 48, yyyy);
        DBG_LOG("GetRegisterData");
    } else if(ARGV_EQUAL("UpChickenStatus")) {
        titi |= 0x01;
        DBG_LOG("UpChickenStatus");
    } else if(ARGV_EQUAL("UpAlarmParameter")) {
        titi |= 0x02;
        DBG_LOG("UpAlarmParameter");
    } else if(ARGV_EQUAL("UpAlarmParameterall")) {
        titi |= 0x04;
        DBG_LOG("UpAlarmParameter all");
    }
}
#endif

void beep(void) {
	uint16_t i;
//for(i=0; i<500; i++){
// 蜂鸣器
	HAL_GPIO_WritePin(GPIOC, BEEP_EN_Pin, GPIO_PIN_SET);
	osDelay(500);
	HAL_GPIO_WritePin(GPIOC, BEEP_EN_Pin, GPIO_PIN_RESET);
//osDelay(1);
//}

}
