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

//新solution最大支持标签数量为480个, 一次性解决问题
#define MAX_LABLES_PER_BASE     480
//#define SECONDS_PER_MINUTE      60
#define MAX_LABLE_ARR_LENGTH    (MAX_LABLES_PER_BASE/8)


// 上报消息类型
#define STATUS_UPDATE        0x01   //     状态变更
#define RFID_ERROR_WARNING   0x02   //     异常报警
#define INVENTRY_UPDATE      0x03   //     盘库上报
#define MOVEING_INTERFACE    0x05   //    移车接口

//每个基站支持的标签数量
//#define SUPPORT_LABLE_PERLBASE   60

//每次发送队列的最大长度
#define U1103_QUEUE_MAX  10

//即每10秒发送10个标签的一个U1103包
#define U1103_TIME_INTERVAL   10

//每分钟6次
#define INVENTORY_UPDATE_PER_MINUTE  6 

void BT_Task(void const *argument);
static void BTCmdHanlde();
static void bt_Console(int argc, char *argv[]);
int16_t BT_Console(uint8_t *data, uint16_t len);
int16_t BT_SocketSendData(uint8_t *data, uint16_t len);
void BT_Intercept_Proc(void);
static void sendInventoryUpdate(void);
static void send1110Msg(uint8_t labelMarkByte);
//static void sendInventoryUpdate(uint8_t deviceStatus, uint32_t labelID);
static void rfidErrorUpdate(uint8_t highByte, uint8_t lowByte, uint32_t labelID);
static void stateChangedUpdate(uint8_t targetStatus, uint8_t initialStatus, uint32_t labelID);
static void sendMovingEvent(uint8_t gSensorStatus, uint32_t labelID, uint8_t deviceStatus);
void transLabelId(uint32_t deviceIDNumber);
uint16_t transStatusToNumber(uint8_t status);

TaskHandle_t BTtaskHandle;
uint16_t reportTick = 0;
uint16_t lastReportTick = 0;

//uint32_t labelIdArray[10];
//uint8_t deviceStatusArray[10];

uint16_t labelU1103Number;
uint8_t btIdx = 0;
uint64_t timeStamp64;
uint16_t queSize = 0;
char labelIDStandard[17];

//U1103队列索引
static uint16_t inventoryUpdateQueueIndex = 0;

static uint16_t inventoryWriteQueueIndex = 0;

typedef struct {
	uint16_t labelId;
	uint8_t deviceStatus;
} InventoryUpdateMsg;

uint32_t labelIdQue[MAX_LABLES_PER_BASE];

uint8_t labels1110Arr[MAX_LABLE_ARR_LENGTH];
uint8_t labelsDeviceStatusArr[MAX_LABLES_PER_BASE];
uint16_t label1110HandleIdx = 0;

//InventoryUpdateMsg inventoryUpdateArr[SUPPORT_LABLE_PERLBASE];

#if 0
void fillInventoryArrForTest() {
	uint8_t i = 0;
	//for (i = 0; i < SUPPORT_LABLE_PERLBASE; i++) {
	for (i = 0; i < 5; i++) {
		inventoryUpdateArr[i].labelId = 10 + i;
		inventoryUpdateArr[i].deviceStatus = 0x0E;
	}
}
#endif

//uint8_t seed[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b' }
//0x41-0x5A 0x61-0x7a 0x30-0x39
//26+26+10

/**
 * BT初始化
 */
void bt_Init() {
	//genUUID();
	//osThreadDef(btTask, BT_Task, osPriorityNormal, 0, 1500)
	osThreadDef(btTask, BT_Task, osPriorityNormal, 0, 1500)
	;
	BTtaskHandle = osThreadCreate(osThread(btTask), NULL);

	DBG_LOG("BT Init.");

	//fillInventoryArrForTest();
}

#if 0
BOOL ifQueueHasData(uint8_t queStartIndex) {
	uint8_t i = 0;
	for (i = queStartIndex; i < queStartIndex + U1103_QUEUE_MAX; i++) {
		if (inventoryUpdateArr[i].labelId > 0) {
			return TRUE;
		}
	}
	return FALSE;
}
#endif

static void sendInventoryUpdateMsg(uint16_t updateSendingIdx){
    uint16_t i = 0;
    uint16_t labelId = 0;
    BOOL ifNeedSend = FALSE;
 	cJSON *data = NULL;
    cJSON *infos = NULL;

    DBG_LOG("%d, ram: %d",updateSendingIdx, xPortGetFreeHeapSize());
    for(i=updateSendingIdx*8; i<((updateSendingIdx+1)*8); i++){
        if(labelIdQue[i] > 0){
            ifNeedSend = TRUE;
            break;
        }
    }
    if(ifNeedSend == TRUE){
        data = cJSON_CreateObject();
        if (data != NULL) {
            infos = NULL;
            infos = cJSON_CreateArray();
            if (infos != NULL) {
                for(i=updateSendingIdx*8; i<((updateSendingIdx+1)*8); i++){
                    if(labelIdQue[i] > 0){
                        //最少有一条可以发送
                        
                        labelId = labelIdQue[i];
                        DBG_LOG("%d ready to send, pos: %d", labelId, i);
                        
                        cJSON *info = NULL;
                        info = NULL;
                        info = cJSON_CreateObject(); 
                        if (info != NULL) {
                            //将id转成字符串
                            transLabelId(labelId);
                             //加入状态
                            cJSON_AddNumberToObject(info, "deviceStatus", transStatusToNumber(labelsDeviceStatusArr[i]));
                            //恢复状态值,避免脏数据
                            labelsDeviceStatusArr[i] = 0;
                            //加入labelID
                            cJSON_AddStringToObject(info, "labelId", labelIDStandard);
                            
                            //放入时间戳
                            if (timeStamp64 != 0) {
                                cJSON_AddNumberToObject(info, "dateTime", timeStamp64);
                            } else {
                                cJSON_AddNumberToObject(info, "dateTime", 1569484973000);
                            }
                            
                            //加入数组
                            cJSON_AddItemToArray(infos, info);            
                            
                        }//endof if (info != NULL)   
                        
                        labelIdQue[i] = 0;
                    }//endof if(labelIdQue[i]>0)
                         
                }//endof for(i=label1110HandleIdx*8; i<((label1110HandleIdx+1)*8); i++)
                
                cJSON_AddItemToObjectCS(data, "infos", infos);        
            }//endof if (infos != NULL)
            
            publishData("U1103", data);

        }//endof if (data != NULL)       
         
    }    
        
}

void BT_Task(void const *argument) {

	TWDT_DEF(BTTask, 60000);
	TWDT_ADD(BTTask);
	TWDT_CLEAR(BTTask);
	uint8_t testCounter = 0;
	uint32_t btcmdtick = 0;

	inventoryUpdateQueueIndex = 0;

	inventoryWriteQueueIndex = 0;
	//BTtaskHandle = xQueueCreate(50, sizeof(CmdTYPE_t));

	while (1) {
		osDelay(1);
        TWDT_CLEAR(BTTask);
		if (TS_VOERFLOW(btcmdtick, 1000)) {
			btcmdtick = HAL_GetTick();
			//BTCmdHanlde();
			testCounter++;
            if(MQTT_IsConnected()){
                sendInventoryUpdateMsg(label1110HandleIdx);
                //send1110Msg(labels1110Arr[label1110HandleIdx]);
            }
            label1110HandleIdx++;            
            if(label1110HandleIdx >= MAX_LABLE_ARR_LENGTH){
                label1110HandleIdx = 0;
            }            
                       
		}
#if 0
		if (testCounter > U1103_TIME_INTERVAL) {
			if (MQTT_IsConnected()) {
				DBG_LOG("Send  %d - %d", inventoryUpdateQueueIndex * U1103_QUEUE_MAX, inventoryUpdateQueueIndex * U1103_QUEUE_MAX+ U1103_QUEUE_MAX);

				//如果本段有数据
				//if (inventoryUpdateArr[inventoryUpdateQueueIndex * 10].labelId > 0) {
				if (ifQueueHasData(inventoryUpdateQueueIndex * U1103_QUEUE_MAX)) {
					DBG_LOG("Data In queue.");
					sendInventoryUpdate();
				}
				inventoryUpdateQueueIndex++;
				//如果发送过6次, 就从队列0开始发送
				if (inventoryUpdateQueueIndex >= INVENTORY_UPDATE_PER_MINUTE) {

					inventoryUpdateQueueIndex = 0;
				}
			}

			//DBG_LOG("bt task running");
			//DBG_LOG("Heap Memory free size:%u", xPortGetFreeHeapSize());
			testCounter = 0;
		}
#endif        
		
		if (MQTT_IsConnected()) {
			BT_Intercept_Proc();
		}
	}

}

static void BTCmdHanlde(void) {

}

int16_t BT_SocketSendData(uint8_t *data, uint16_t len) {
	return 0;
}

static void bt_Console(int argc, char *argv[]) {

}

static uint16_t addLabelIdInQue(uint32_t labelID){
    uint16_t i;
    for(i=0; i<MAX_LABLES_PER_BASE; i++){
        if(labelIdQue[i] == 0){
            labelIdQue[i] = labelID;
            return i;
        }     
    }
    
    return 0;
}

#if 0
static void addLabelIdInQue(uint32_t labelID){
    //首先找到属于数组中哪个byte
    uint16_t byteIdx = 0;
    uint8_t bitIdx = 0;
    DBG_LOG("Adding %d",labelID);
    byteIdx = (labelID / 8);
    //然后找到属于哪个bit
    bitIdx = (uint8_t)( labelID - byteIdx * 8); 
    
    labels1110Arr[byteIdx]= labels1110Arr[byteIdx] | ( 1 << (7 - bitIdx));
    DBG_LOG("%x: ",labels1110Arr[byteIdx]);
}
#endif

static BOOL ifLabelIdInQue(uint32_t labelID) {
    uint16_t i;
    for(i=0; i<MAX_LABLES_PER_BASE; i++){
        if(labelIdQue[i] == labelID){
            return TRUE;
        }
    }
    return FALSE;
}
#if 0
//0 1 2 3 4 5 6 7  8 9 10 11 12 13 14 15  16
//重写这个函数
static BOOL ifLabelIdInQue(uint32_t labelID) {
    //首先找到属于数组中哪个byte
    uint16_t byteIdx = 0;
    uint8_t bitIdx = 0;
    byteIdx = (labelID / 8);
    //然后找到属于哪个bit
    bitIdx = (uint8_t)( labelID - byteIdx * 8); 
    
    //然后判断这个bit是不是为1
    if(labels1110Arr[byteIdx] & ( 1 << (7 - bitIdx))){
        //already in queue
        DBG_LOG("%daiq", labelID);
	    return TRUE;       
    }
    return FALSE;
}


static BOOL ifLabelIdInQue(uint8_t labelID) {
	uint8_t i = 0;

	//必须全数组搜索
	for (i = 0; i < SUPPORT_LABLE_PERLBASE; i++) {
		if (inventoryUpdateArr[i].labelId == labelID) {
            
			DBG_LOG("%daiq", labelID);
			return TRUE;

		}
	}
	DBG_LOG("Adding %d to Queue", labelID);
	return FALSE;
}
#endif

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
	uint32_t labelID = 0;
	uint8_t packageNumber = 0;
	uint8_t i = 0;
	uint8_t *cmdStart;
	if (cmdLength >= MSG_LENGTH) {
		DBG_LOG("rssi is %d", *(cmd + 15));

		packageNumber = cmdLength / MSG_LENGTH;

		if (packageNumber > 0) {
			DBG_LOG("Got %d packages", packageNumber);
			for (i = 0; i < packageNumber; i++) {
				
				cmdStart = cmd + i * MSG_LENGTH;
				if (*cmdStart == BT_MSG_HEAD && (*(cmdStart + MSG_LENGTH - 1) == BT_MSG_TAIL) && (*(cmdStart + 1) == AFUIOT)) {
					oldState = *(cmdStart + 7);
					newState = *(cmdStart + 8);
					//这里应该是设备id
					//deviceIDNumber = ((uint32_t)(*(cmd + 2))) << 24 || ((uint32_t)(*(cmd + 3))) << 16 || ((uint32_t)(*(cmd + 4))) << 8
					//|| ((uint32_t)(*(cmd + 5)));

					//这里后面需要改
					labelID = *(cmdStart + 5) + ((*(cmdStart + 4) )<< 8) + ((*(cmdStart + 3)) << 16) + ((*(cmdStart + 2)) << 24);

					// 检查数据上报类型
					switch (*(cmdStart + 12)) {
					// 状态异常
					case STATUS_UPDATE:
						// 切换状态时beep
						beep();
						//if (ifSendingGreenLight()) {
						stateChangedUpdate(newState, oldState, labelID);
						//}

						break;
						// 标签预警
					case RFID_ERROR_WARNING:
						beep();
						//if (ifSendingGreenLight()) {
						rfErrorHigh = *(cmdStart + 9);
						rfErrorLow = *(cmdStart + 10);
						rfidErrorUpdate(rfErrorHigh, rfErrorLow, labelID);
						//}

						break;

					case INVENTRY_UPDATE:
                                              
                        if(!ifLabelIdInQue(labelID)){
                            
                            //如果labelID不在列表中, 就加进去.                            
                            uint16_t idx = addLabelIdInQue(labelID); 
                            
                            //增加state
                            labelsDeviceStatusArr[idx] = newState;
                            
                            //DBG_LOG("NS:%d",newState);
                            //DBG_LOG("OS:%d",oldState);
                        }

#if 0                       
						//首先判断队列中是否已经有记录了
						if (!ifLabelIdInQue(labelID)) {
							inventoryUpdateArr[inventoryWriteQueueIndex].labelId = labelID;
							inventoryUpdateArr[inventoryWriteQueueIndex].deviceStatus = newState;
							DBG_LOG("inventoryWriteQueueIndex:%d", inventoryWriteQueueIndex);

							//写入之后index++
							inventoryWriteQueueIndex++;
							if (inventoryWriteQueueIndex >= SUPPORT_LABLE_PERLBASE) {
								inventoryWriteQueueIndex = 0;
							}
						}
#endif
						break;

					case MOVEING_INTERFACE:
						beep();
						//if (ifSendingGreenLight()) {
						gSensorStatus = *(cmdStart + 11);
						sendMovingEvent(gSensorStatus, labelID, newState);
						//}
						break;

					default:
						break;
					}
				}
			}
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
		DBG_LOG("BT msg length:%d", len);

		if (len >= (BT_RECEIVE_MAX_SIZE - 1)) {

			len = BT_RECEIVE_MAX_SIZE - 1;
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

//下面测试用

//char labelIDStandard[17] = "M030057000000002";
//生产用下面的
//char labelIDStandard[17] = "M030055000000002";

void transLabelId(uint32_t deviceIDNumber) {
#if PROD_ENV   
    sprintf(labelIDStandard, "M030055%09d", deviceIDNumber); 
#else
    sprintf(labelIDStandard, "M030057%09d", deviceIDNumber); 
#endif    
}

void transLabelIdOld(uint32_t deviceIDNumber) {
//void transLabelId(uint8_t deviceIDNumber) {

	uint8_t time1, time10, time100;

    //先恢复一个初始状态
	labelIDStandard[15] = '0';
	labelIDStandard[14] = '0';
	labelIDStandard[13] = '0';

    //1位数
	if (deviceIDNumber < 10) {
		labelIDStandard[15] = (char) (deviceIDNumber + 0x30);
		//*(labelIDStandard + 16) = deviceIDNumber;
        
    //2位数
	} else if (deviceIDNumber > 9 && deviceIDNumber < 100) {
		time10 = deviceIDNumber / 10;
		time1 = deviceIDNumber - (time10 * 10);
		labelIDStandard[15] = (char) (time1 + 0x30);
		labelIDStandard[14] = (char) (time10 + 0x30);
		//*(labelIDStandard + 16) = time1;
		//*(labelIDStandard + 15) = time10;
   
        //3位数
	} else if (deviceIDNumber > 99) {
		time100 = deviceIDNumber / 100;
		time10 = (deviceIDNumber - time100 * 100) / 10;
		time1 = deviceIDNumber - (time10 * 10) - time100 * 100;

		labelIDStandard[15] = (char) (time1 + 0x30);
		labelIDStandard[14] = (char) (time10 + 0x30);
		labelIDStandard[13] = (char) (time100 + 0x30);
		//*(labelIDStandard + 16) = time1;
		//*(labelIDStandard + 15) = time10;
		//*(labelIDStandard + 14) = time100;
	}

}

uint16_t transStatusToNumber(uint8_t status) {
	return (1000 * ((status >> 3) & 1) + 100 * ((status >> 2) & 1) + 10 * ((status >> 1) & 1) + (status & 1));
}


//0 1 2 3 4 5 6 7  
//8 9 10 11 12 13 14 15  
//16 ...
//label1110HandleIdx
static void send1110Msg(uint8_t labelMarkByte){
    uint8_t i = 0;
    uint32_t lId = 0;
    BOOL ifNeedSend = FALSE;
  	cJSON *data = NULL;

	data = cJSON_CreateObject();
    DBG_LOG("%d, ram: %d",label1110HandleIdx, xPortGetFreeHeapSize());
	if (data != NULL) {

		cJSON *infos = NULL;
		infos = NULL;

		infos = cJSON_CreateArray();
		if (infos != NULL) {
            for(i=0; i<8; i++){
                //如果该bit不为空, 就发送
                if(labelMarkByte >> (7-i) & 1){
                    ifNeedSend = TRUE;
                    
                    //labelID等于byte在数组中的索引乘以8, 再加上在byte中的索引. 
                    lId = label1110HandleIdx * 8 + i; 
                    
                    DBG_LOG("%d ready to send", lId);
                    
                    cJSON *info = NULL;
					info = NULL;
					info = cJSON_CreateObject();

					if (info != NULL) {
                        //将id转成字符串
                        transLabelId(lId);
                        
                        //加入状态
                        cJSON_AddNumberToObject(info, "deviceStatus", transStatusToNumber(labelsDeviceStatusArr[lId]));
                        
                        //恢复状态值,避免脏数据
                        labelsDeviceStatusArr[lId] = 0;
                        
                        cJSON_AddStringToObject(info, "labelId", labelIDStandard);
                        //放入时间戳
                        if (timeStamp64 != 0) {
							cJSON_AddNumberToObject(info, "dateTime", timeStamp64);
						} else {
							cJSON_AddNumberToObject(info, "dateTime", 1569484973000);
						}

						//加入数组
						cJSON_AddItemToArray(infos, info);                       
                    }              
                }
            } 
            
            cJSON_AddItemToObjectCS(data, "infos", infos);
        }

    }
    
    if(ifNeedSend){
        publishData("U1103", data);  
    }else{
        cJSON_Delete(data);
    } 
   
    labels1110Arr[label1110HandleIdx] = 0;

}

//static void sendInventoryUpdate(uint8_t deviceStatus, uint32_t labelID)
static void sendInventoryUpdate(){
	cJSON *data = NULL;
//	data = NULL;

	data = cJSON_CreateObject();
	uint8_t i;
	uint8_t index;
	if (data != NULL) {

		cJSON *infos = NULL;
		infos = NULL;

		infos = cJSON_CreateArray();
		if (infos != NULL) {
			//if (labelU1103Number > 0) {
			index = inventoryUpdateQueueIndex * U1103_QUEUE_MAX;
            //依次增加要加入的array类型json
#if 0            
			for (i = index; i < index + U1103_QUEUE_MAX; i++) {
				if (inventoryUpdateArr[i].labelId > 0) {
					cJSON *info = NULL;
					info = NULL;
					info = cJSON_CreateObject();

					if (info != NULL) {

						transLabelId(inventoryUpdateArr[i].labelId);

						//清除记录
						inventoryUpdateArr[i].labelId = 0;

						cJSON_AddNumberToObject(info, "deviceStatus", transStatusToNumber(inventoryUpdateArr[i].deviceStatus));

						//cJSON_AddStringToObject(info, "labelId", "M030057000000002");
						//cJSON_AddStringToObject(info, "labelId", getLabelId(inventoryUpdateArr[i].labelId));
						cJSON_AddStringToObject(info, "labelId", labelIDStandard);

						if (timeStamp64 != 0) {
							cJSON_AddNumberToObject(info, "dateTime", timeStamp64);
						} else {
							cJSON_AddNumberToObject(info, "dateTime", 1569484973000);
						}

						//有记录才放进去json数组, 没有记录不放
						cJSON_AddItemToArray(infos, info);
					}

				}

			}
#endif
		}
		cJSON_AddItemToObjectCS(data, "infos", infos);

	}
	publishData("U1103", data);

//cJSON_Delete(data);
}

uint16_t getErrorCodeNumber(uint8_t highByte, uint8_t lowByte) {

//异常移动
	if (highByte == 0x10 && lowByte == 0x02) {
		return 1002;

		//电量不足
	} else if (highByte == 0x10 && lowByte == 0x01) {
		return 1001;

		//标签自身故障
	} else if (highByte == 0x10 && lowByte == 0x03) {
		return 1003;
	}

//自身故障
	return 2006;

}

//uint16_t erroCodeNumber =0;
static void rfidErrorUpdate(uint8_t highByte, uint8_t lowByte, uint32_t labelID) {
	cJSON *data = NULL;
	data = NULL;
	data = cJSON_CreateObject();
//uint16_t errorCode = (uint16_t) highByte << 8 || lowByte;
//erroCodeNumber=

	if (data != NULL) {
		transLabelId(labelID);
		cJSON_AddStringToObject(data, "labelId", labelIDStandard);
		cJSON_AddNumberToObject(data, "errorCode", getErrorCodeNumber(highByte, lowByte));
	}
// 标签错误cmd是u1102
	publishData("U1102", data);

//cJSON_Delete(data);
}

static void stateChangedUpdate(uint8_t targetStatus, uint8_t initialStatus, uint32_t labelID) {
	cJSON *data = NULL;
	data = NULL;
	data = cJSON_CreateObject();
	if (data != NULL) {
		transLabelId(labelID);

		cJSON_AddNumberToObject(data, "targetStatus", transStatusToNumber(targetStatus));

		cJSON_AddNumberToObject(data, "initialStatus", transStatusToNumber(initialStatus));

		cJSON_AddStringToObject(data, "labelId", labelIDStandard);

		if (timeStamp64 != 0) {
			cJSON_AddNumberToObject(data, "dateTime", timeStamp64);

		} else {
			cJSON_AddNumberToObject(data, "dateTime", 1569484973000);
		}
	}
	publishData("U1101", data);

//cJSON_Delete(data);
}

static void sendMovingEvent(uint8_t gSensorStatus, uint32_t labelID, uint8_t deviceStatus) {
	cJSON *data = NULL;
	data = NULL;
	data = cJSON_CreateObject();
	if (data != NULL) {

		cJSON *infos = NULL;
		infos = NULL;

		infos = cJSON_CreateArray();
		if (infos != NULL) {

			cJSON *info = NULL;
			info = NULL;
			info = cJSON_CreateObject();
            	
			if (info != NULL) {

				transLabelId(labelID);

				cJSON_AddNumberToObject(info, "deviceStatus", transStatusToNumber(deviceStatus));

				cJSON_AddNumberToObject(info, "gyroscopeStatus", gSensorStatus);

				cJSON_AddStringToObject(info, "labelId", labelIDStandard);

				if (timeStamp64 != 0) {
					cJSON_AddNumberToObject(info, "dateTime", timeStamp64);

				} else {
					cJSON_AddNumberToObject(info, "dateTime", 1569484973000);
				}
				//cJSON_AddNumberToObject(info, "deviceStatus", transStatusToNumber(deviceStatus));
				//cJSON_AddStringToObject(info, "labelId", getLabelId(labelID));
				//if (timeStamp64 != 0) {
				//	cJSON_AddNumberToObject(info, "dateTime", timeStamp64);
				//} else {
				//cJSON_AddNumberToObject(info, "dateTime", 1569484973000);
				//}
			}
			cJSON_AddItemToArray(infos, info);
		}
		cJSON_AddItemToObjectCS(data, "infos", infos);

		//cJSON_AddNumberToObject(data, "dateTime", 1569484973000);
	}
	publishData("U1105", data);

}

#if 0
if (*cmd == BT_MSG_HEAD && (*(cmd + MSG_LENGTH - 1) == BT_MSG_TAIL) && (*(cmd + 1) == AFUIOT)) {

	oldState = *(cmd + 7);
	newState = *(cmd + 8);
	//这里应该是设备id
	//deviceIDNumber = ((uint32_t)(*(cmd + 2))) << 24 || ((uint32_t)(*(cmd + 3))) << 16 || ((uint32_t)(*(cmd + 4))) << 8
	//|| ((uint32_t)(*(cmd + 5)));

	//这里后面需要改
	labelID = *(cmd + 5);

	// 检查数据上报类型
	switch (*(cmd + 12)) {
	// 状态异常
	case STATUS_UPDATE:
		// 切换状态时beep
		beep();
		//if (ifSendingGreenLight()) {
		stateChangedUpdate(newState, oldState, labelID);
		//}

		break;
		// 标签预警
	case RFID_ERROR_WARNING:
		beep();
		//if (ifSendingGreenLight()) {
		rfErrorHigh = *(cmd + 9);
		rfErrorLow = *(cmd + 10);
		rfidErrorUpdate(rfErrorHigh, rfErrorLow, labelID);
		//}

		break;

	case INVENTRY_UPDATE:

		//首先判断队列中是否已经有记录了
		if (!ifLabelIdInQue(labelID)) {
			inventoryUpdateArr[inventoryWriteQueueIndex].labelId = labelID;
			inventoryUpdateArr[inventoryWriteQueueIndex].deviceStatus = newState;
			DBG_LOG("inventoryWriteQueueIndex:%d", inventoryWriteQueueIndex);

			//写入之后index++
			inventoryWriteQueueIndex++;
			if (inventoryWriteQueueIndex >= SUPPORT_LABLE_PERLBASE) {
				inventoryWriteQueueIndex = 0;
			}
		}

		break;

	case MOVEING_INTERFACE:
		beep();
		//if (ifSendingGreenLight()) {
		gSensorStatus = *(cmd + 11);
		sendMovingEvent(gSensorStatus, labelID, newState);
		//}
		break;

	default:
		break;
	}

	//case
	//DBG_LOG("My package");


//检查是否含有了
uint8_t ifLabelRecorded(uint32_t labelId) {
	uint8_t i = 0;
	for (i = 0; i < 10; i++) {
		if (labelIdArray[i] == labelId) {
			return 1;
		}
	}

	return 0;
}


void clearLableArray() {
	uint8_t i = 0;
	for (i = 0; i < 10; i++) {
		labelIdArray[i] = 0;
	}
}


// 检查是否可以发送
uint8_t ifSendingGreenLight(uint8_t deviceStatus, uint32_t labelID) {
	uint16_t timePassed = 0;
	if (MQTT_IsConnected()) {
		if (HAL_GetTick() > lastReportTick) {
			timePassed = HAL_GetTick() - lastReportTick;
			if (timePassed > 60000) {
				DBG_LOG("Green Light");
				lastReportTick = HAL_GetTick();

				return 1;
				//如果时间不够, 就检查是否之前已经有记录
			} else {
				//如果记录存在, 直接return
				if (ifLabelRecorded(labelID)) {
					DBG_LOG("Already Got this 1103, total record: %d", labelU1103Number);
					return 0;
				} else {
					//此处应该用结构体
					labelIdArray[labelU1103Number] = labelID;
					deviceStatusArray[labelU1103Number] = deviceStatus;
					labelU1103Number++;
					DBG_LOG("Add this labelID");
					return 0;
				}

			}
		}
	}
	return 0;
}
#endif

