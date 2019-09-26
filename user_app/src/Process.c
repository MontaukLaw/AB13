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
#define CHICKEN_HOUSE_MAIN_PAGE_REG_LENGTH     20
/* Private define ------------------------------------------------------------*/


/* Private macros ------------------------------------------------------------*/
const uint8_t GetChickenNumberCmd[8] = {0x01, 0x03, 0x02, 0xC5, 0x00, 0x01, 0x95, 0x8F};

//报警参数寄存器表
const REG_TYPE AlarmPar[ALARMDATA_REG_QUANTITY] = {
    1153, 900, 1203, 424, 430, 431, 402, 403, 1231, 410, 411, 412, 413,  414, 415, 416, 417, 418,
    1204, 1205, 1206, 1207, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401
};

const REG_TYPE  ChickenAlarmPar[ALARMDATA_REG_QUANTITY] = {
    1153, 900, 381, 424, 430, 431, 402, 403, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418,
    382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401
};

//鸡舍线圈寄存器表
const REG_TYPE HouseAddr[16] = {50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65};

//鸡舍主页寄存器表
const REG_TYPE HouseParAddr[HOMEPAGE_REG_QUANTITY] = {1250, 1251, 1252, 1253, 1254, 1255, 1256, 1257};

// 鸡舍主页显示20个字节
const REG_TYPE ChickenHouseMainPageDisplay[CHICKEN_HOUSE_MAIN_PAGE_REG_LENGTH] = {700, 1062, 701, 508, 509, 510, 582, 513, 512, 511, 583, 584, 518, 1187, 1193, 305, 256, 255, 552, 551};

#if 0
// 下面就是一堆fuc***ing bull sh*t...
// 鸡舍参数寄存器表
const REG_TYPE ParRgeAddr[OTHERDATA_REG_QUANTITY] = {
    // 20个项目, 没变, 40byte输出
    700, 1062, 701, 508, 509, 510, 582, 513, 512, 511, 583, 584, 518, 1187, 1193, 305, 256, 255, 552, 551, //对应鸡舍主页显示20字节
    
    // 5个项目, 没变 10byte输出
    300, 306, 301, 304, 17, //设置系统参数10字节
    
    // 主从共14个项目 28byte输出
    307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, //主板、从板温度探头选择14字节
    
    //主板温度传感器 7项, 14byte输出
    501, 502, 503, 504, 505, 506, 507,
    
    //副板温度传感器 8项 16byte输出
    1161, 1162, 1163, 1164, 1165, 1166, 1167, 1168, //主板、副板温度传感器15字节
    
    // 风机11项, 22byte输出
    541, 542, 543, 544, 545, 546, 547, 548, 549, 550, 560, //风机11字节
    
    // 降温20项目, 40byte输出
    257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 275, 276, 888, 889, //降温参数20字节
    
    // 原来是21项目, 现在增加2项, 23项, 46byte输出 变频风机风量*10m3/h: 4X0060 鸡只总数: 4X0018, 分别插入到响应的位置
    1, 2, 695, 5, 6, 7, 8, 10, 11, 13, 14, 69, 70, 71, 72, 12, 3, 4, 60, 15, 16, 9, 18,//通风参数21字节
    
    //风机10项, 20byte输出, 没变化   
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28, //风机组10字节
    
    //原本30项, 先增加30个温差项4X0948~4X0977, 附在后面
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, //级别通风对应台数30字节
    948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 

    // 负压校准原本13现在增加到16项, 32byte输出, 4X1054, 4X1055, 4X1056
    585, 586, 587, 783, 784, 785, 786, 63, 64, 65, 588, 589, 590, 1054,1055,1056, //负压校准13byte
    
    // 以上是第一个数据包的内容.
        
    88, 85, 86, 89, 87, 93, 94, 95, 96, 97, 98, 99, 90, 91, //负压校准调节参数14byte
    75, 76, 77, 78, 79, 80, 81, 82, //负压校准极限位置8byte
    281, 282, 788, 789, 466, 467, 468, 469, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, //加热器22byte
    797, 374, 377, 376, //饮水196byte
    731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750,
    751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770,
    771, 772, 773, 774, 775, 776, 777, 778, ////////48byte
    802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821,
    822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 841,
    842, 843, 844, 845, 846, 847, 848, 849, ////////48byte
    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020,
    2021, 2022, 2023, 2024, 2025, 2026, 2027, 2028, 2029, 2030, 2031, 2032, 2033, 2034, 2035, 2036, 2037, 2038, 2039, 2040,
    2041, 2042, 2043, 2044, 2045, 2046, 2047, 2048, ////////48byte
    2054, 2055, 2056, 2057, 2058, 2059, 2060, 2061, 2062, 2063, 2064, 2065, 2066, 2067, 2068, 2069, 2070, 2071, 2072, 2073,
    2074, 2075, 2076, 2077, 2078, 2079, 2080, 2081, 2082, 2083, 2084, 2085, 2086, 2087, 2088, 2089, 2090, 2091, 2092, 2093,
    2094, 2095, 2096, 2097, 2098, 2099, 2100, 2101, ////////48byte
    2223, 2224, 2225, 2226, 2227, //照明定时开关5byte
    321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, //日龄1#12byte
    333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, //日龄2#12byte
    345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, //日龄3#12byte
    357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, //日龄4#12byte
    2151, 2152, 2154, 2155, 2156, 2157, 2158, 2159, 2160, 2161, 2162, 2163, //日龄5#12byte
    180, 181, 182, 183, 184, 185, 186, 187, 188, 62, 190, 73, 177, 297, 298, //故障使能15byte
    170, 171, 172, 173, 174, 175, 176, //故障参数7byte
    211, 212, 213, 214, 1188, 1189, 216, 217, 218, 219, 1194, 1195, //称重系统12byte
    39, 61, 29, 30, 31, 32, 33, 34, 35, 36, 160, 161, 162, 163, 164, 165, 166, 167, //光照强度18byte
    
    191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, //降温表20byte
    //
    251, 3, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250 //增重表32byte
};


const REG_TYPE SetCmdReg[SETCMD_REG_QUANTITY] = {
    305, 256, 551, 255, 552,
    300, 306, 301, 304, 17,
    307, 308, 309, 310, 311, 312, 313,
    314, 315, 316, 317, 318, 319, 320,
    257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 275, 276, 552, 888, 889,
    1, 2, 695, 5, 6, 7, 8, 10, 11, 13, 14, 69, 70, 71, 72, 12, 3, 4, 15, 16, 9,
    19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
    63, 64, 65,
    88, 89, 85, 87, 86, 93, 94, 95, 96, 97, 98, 99, 90, 91,
    75, 76, 77, 78, 79, 82,
    281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296,
    374, 377, 376,
    2223, 2224, 2225, 2226, 2227,
    321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332,
    333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344,
    345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356,
    357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368,
    2151, 2152, 2154, 2155, 2156, 2157, 2158, 2159, 2160, 2161, 2162, 2163,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 62, 190, 73, 177, 297, 298,
    170, 171, 172, 173, 174, 175, 176,
    211, 212, 213, 214, 216, 217, 218, 219, 695,
    39, 61, 29, 30, 31, 32, 33, 34, 35, 36, 160, 161, 162, 163, 164, 165, 166, 167,
    304, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
    251, 3, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250,
    381, 424, 430, 431, 402, 403, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401
};

#endif //没用的全局数组注释掉

/* Private variables ---------------------------------------------------------*/
static char subscribeTopic[36], publishTopic[36], playid[48];
// static uint32_t Report_ts = 0;
//static uint8_t UpDataArray[UPDATA_LENGTH];
static QueueHandle_t CmdHandle = NULL;
// static CmdTYPE_t CmdType;
// static SetCmd_t  SetCmd;
uint8_t titi = 0;

uint8_t gTxBuffer[FIRSTPACKETSIZE];

/* Private function prototypes -----------------------------------------------*/
static void ArrivePath(uint8_t* dat, uint16_t len);
// static proret_t ArriveDataProc(char* cmd, cJSON* desired);
static BOOL CMD_Confirm_Rsp(uint32_t ordermsgid, proret_t ret);
static void process_Console(int argc, char* argv[]);
// static void xReceiveInquirePara(Controller_t *xController);
static uint16_t WaitModbusReply_Single(const uint8_t *wcmd, uint32_t wtick);
static uint16_t GetChickenNumber();
uint16_t SetChickenNumber(uint16_t snum);
//static BOOL GetRegisterData(uint16_t gaddr, uint16_t glen, uint8_t *gbuf);

static BOOL UpChickenStatus(uint32_t umessageid);
static void UpAlarmParameter(uint8_t uanum, uint32_t umessageid);
static void DataAnalysis(uint32_t messageid, cJSON *dis);
static void DeviceQueryAnalysis(uint32_t messageid, cJSON *dis);
static void ChickenManagementAnalysis(uint32_t messageid, cJSON *dis);
static void CmdHanldeLogic(void);
void InquireTask(void* argument);
uint8_t ifChickenHouseOnline(uint8_t chickenHouse);
uint32_t GetIDToHex();
TaskHandle_t Inquiretask;
/* Exported functions --------------------------------------------------------*/

/**
 * 处理任务初始化
 */
void Process_Init(void) {
    strcpy(subscribeTopic, "/");
    strcat(subscribeTopic, PROJECT);
    strcat(subscribeTopic, "/");
    strcat(subscribeTopic, WorkParam.mqtt.MQTT_ClientID);
    strcpy(publishTopic, "/");
    strcat(publishTopic, PROJECT);
    CMD_ENT_DEF(process, process_Console);
    Cmd_AddEntrance(CMD_ENT(process));
    Subscribe_MQTT(subscribeTopic, QOS1, ArrivePath);
    // xTaskCreate(InquireTask, "InquireTask", 256, NULL, osPriorityNormal, &Inquiretask);
    if(xTaskCreate(&InquireTask, "InquireTask", 512, NULL, 3, &Inquiretask) != pdPASS)
        DBG_LOG("Create InquireAnalysis failure");
    else
        DBG_LOG("Create InquireAnalysis ok");
    DBG_LOG("Process Start.");
}

/**
 * 
 *
 * 
 */
void Updata_Gameoff(uint8_t result) {
    cJSON* desired = NULL;
    desired = cJSON_CreateObject();
    if (desired != NULL) {
        cJSON_AddStringToObject(desired, "messageid", playid);
        cJSON_AddNumberToObject(desired, "result", result);
        CMD_Updata("CMD-107", desired);
    }
}

#ifdef UNIT_TEST
void testSingalReg(uint16_t reg){
    uint8_t testArr[2];
    uint8_t highPos, lowPos;
    GetRegisterData(reg, 1, testArr);
    highPos = testArr[0];
    lowPos = testArr[1];
}
#endif

// 这里开始将数据跟头尾信息拼在一起
void sendPackageByMQTT(uint16_t packageLength, uint8_t packageNumber, uint8_t chickenHouseNumber){
    uint8_t *idata = NULL;
    uint16_t upcrc = 0;
    uint32_t nowrtc = 0;
    uint16_t icount = 0;
    uint16_t rawPackagePlusHeadTailCRCLength = packageLength + 16;
    uint32_t idnumhex = GetIDToHex();
    idata = MMEMORY_ALLOC(rawPackagePlusHeadTailCRCLength);

   
    if(idata != NULL) {
        memset((char*)idata, 0xff, rawPackagePlusHeadTailCRCLength);
        
        // 首2字节
        *idata = 0xaf;
        *(idata + 1) = 0xfa;
        
        // 数据包编号
        *(idata + 2) = packageNumber;
        
        // 鸡舍号
        *(idata + 3) = chickenHouseNumber;
        
        // 设备id
        *(idata + 4) = (uint8_t)(idnumhex >> 24);
        *(idata + 5) = (uint8_t)(idnumhex >> 16);
        *(idata + 6) = (uint8_t)(idnumhex >> 8);
        *(idata + 7) = (uint8_t)(idnumhex);
        
        // 时间戳
        nowrtc = RTC_ReadTick();
        *(idata + 8) = (uint8_t)(nowrtc >> 24);
        *(idata + 9) = (uint8_t)(nowrtc >> 16);
        *(idata + 10) = (uint8_t)(nowrtc >> 8);
        *(idata + 11) = (uint8_t)(nowrtc);
                            
        //将数据从UpDataArray挨个填充到待发送的数组idata中.
        for(icount = 0; icount < packageLength; icount++) {
            *(idata + 12 + icount) = gTxBuffer[icount];
        }
        // CRC校验用2字节
        upcrc = CRC_16(0xffff, idata + 2, rawPackagePlusHeadTailCRCLength - 6);
        *(idata + rawPackagePlusHeadTailCRCLength - 4) = (uint8_t)(upcrc >> 8);
        *(idata + rawPackagePlusHeadTailCRCLength - 3) = (uint8_t)(upcrc);
        *(idata + rawPackagePlusHeadTailCRCLength - 2) = 0xaa;
        *(idata + rawPackagePlusHeadTailCRCLength - 1) = 0x55;
        
        // 使用mqtt发布
        Publish_MQTT(publishTopic, QOS0, idata, rawPackagePlusHeadTailCRCLength);                 
       
        // 测试数据准确度用
#ifdef UNIT_TEST       
        testSingalReg(512); 
#endif
        //Publish_MQTT(publishTopic, QOS0, UpDataArray, FIRSTPACKETSIZE + 16);
        MMEMORY_FREE(idata);
    }    

}

uint32_t GetIDToHex() {
    uint8_t  gi = 0;
    uint32_t gidnum = 0;
    uint8_t idlen = strlen(WorkParam.mqtt.MQTT_ClientID);
    idlen -= 6;
    for(; idlen > 0; idlen--) {
        gidnum *= 10;
        gidnum += (*(WorkParam.mqtt.MQTT_ClientID + 6 + gi) - 0x30);
        gi++;
    }
    //DBG_LOG("gidnum:%d,%x", gidnum, gidnum);
    return gidnum;
}

#if 0
uint32_t readIndex = 0;
void getDataFromRelayController(void){
    if(GetRegisterData(HouseParAddr[0], HOMEPAGE_REG_QUANTITY, &UpDataArray[readIndex])){
        //DBG_LOG("Got check house 1# setting.");
    }
    readIndex += 8;
       
    if(GetRegisterData(ChickenHouseMainPageDisplay[0],CHICKEN_HOUSE_MAIN_PAGE_REG_LENGTH,&UpDataArray[readIndex * 2])){
        //DBG_LOG("Got check house main page display");
    };    

}
#endif

void InquireTask(void* argument) {
    TWDT_DEF(PTask, 30000);
    TWDT_ADD(PTask);
    TWDT_CLEAR(PTask);
    uint8_t *idata = NULL;
    uint16_t icount = 0;
    uint8_t ireadamount = 0;
    uint16_t chickenHouseOnlineState = 0;
    uint8_t i = 0;
    uint8_t checkHouseRegDate = 0;
    static BOOL queryen = TRUE;
    static uint8_t  qchicken = 0;
    static uint16_t upcrc = 0;
    static uint32_t qtick = 0, cmdtick = 0;
    static uint32_t qreadcount = 0;
    static uint32_t idnumhex = 0;
    static uint32_t nowrtc = 0;
    CmdHandle = xQueueCreate(50, sizeof(CmdTYPE_t));
    while(CmdHandle == NULL)
        DBG_WAR("xQueueCreate failure");
        idnumhex = GetIDToHex();
    while(!MQTT_IsConnected()) {
        osDelay(100);
    }
    DBG_LOG("idnumhex:%d,%x", idnumhex, idnumhex);
    DBG_LOG("InquireTask Start");
    
    // 先设置鸡舍号, 具体切换鸡舍, 9月22号问过匡总再定, 跟后面无法获取寄存器地址的问题一起.
    // SetChickenNumber(0);

    osDelay(3000);
    while(1){
        // 现在的逻辑清晰, 首先, 通过查询鸡舍目前的启用状态, 获取在线的鸡舍.   
        // 轮询查询在线鸡舍
        for(i = 0; i < 16; i++){
            //checkHouseRegDate = ifCheckHouseOnline(i);
            //如果该鸡舍在线
            if(ifChickenHouseOnline(i) == 1){
                chickenHouseOnlineState = chickenHouseOnlineState || (1<<i);
            }
        }
        /*
        for(){
            if()
            chickenHouseOnlineState
        }
        */
        //DBG_LOG("Task delay start");
        //osDelay(10000);
        //DBG_LOG("Task delay stop");

        readDataFromRelay(chickenHouseOnlineState);
        if(TS_VOERFLOW(cmdtick, 1000)) {
            cmdtick = HAL_GetTick();
            CmdHanldeLogic();
        }
        TWDT_CLEAR(PTask);
    }
#if 0
    while(1) {
        // 第一步, 设置鸡舍号
        //SetChickenNumber(qchicken);
        // 测试从中继连续读取数据
                
        if((queryen == FALSE) && (TS_VOERFLOW(qtick, 300000))) {
            qtick = HAL_GetTick();
            queryen = TRUE;
            //为UpDataArray申请内存空间
            memset(UpDataArray, 0xff, UPDATA_LENGTH);
        } else if(queryen) {
            if(qchicken != GetChickenNumber()) {
                if(qchicken != SetChickenNumber(qchicken)) {
                    DBG_WAR("QueryParameter()->Chicken house switching failed:%x", qchicken);
                }
            } else {
                getDataFromRelayController();
                
                if(qreadcount < HOMEPAGE_REG_QUANTITY) {
                    // 首先获取主页寄存器, 即鸡舍适宜温度等8项
                    if(GetRegisterData((HouseParAddr[0] + (qchicken * 10)), HOMEPAGE_REG_QUANTITY, &UpDataArray[qreadcount * 2])) {
                        qreadcount += HOMEPAGE_REG_QUANTITY;
                    }
                } else {
                   
                    //return;
                    if(ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 307) {
                        ireadamount = 14;
                    } else if((ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 501) ||
                              (ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 170)) {
                        ireadamount = 7;
                    } else if((ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 1161) ||
                              (ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 75)) {
                        ireadamount = 8;
                    } else if((ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 541) ||
                              (ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 2154)) {
                        ireadamount = 10;
                    } else if(ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 257) {
                        ireadamount = 16;
                    } else if((ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 100) ||
                              (ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 221)) {
                        ireadamount = 30;
                    } else if((ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 731) ||
                              (ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 802) ||
                              (ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 2001) ||
                              (ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 2054) ||
                              (ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 321)) {
                        ireadamount = 48;
                    } else if(ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 2223) {
                        ireadamount = 5;
                    } else if(ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 180) {
                        ireadamount = 9;
                    } else if(ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY] == 191) {
                        ireadamount = 20;
                    } else {
                        ireadamount = 1;
                    }
                    if(GetRegisterData(ParRgeAddr[qreadcount - HOMEPAGE_REG_QUANTITY], ireadamount, &UpDataArray[qreadcount * 2])) {
                        qreadcount += ireadamount;
                    }
                    if(qreadcount >= (UPDATA_REG_AMOUNT - 1)) {
                        qreadcount = 0;
                        qchicken++;
                        idata = MMEMORY_ALLOC(FIRSTPACKETSIZE + 16);
                        if(idata != NULL) {
                            memset((char*)idata, 0xff, FIRSTPACKETSIZE + 16);
                            *idata = 0xaf;
                            *(idata + 1) = 0xfa;
                            *(idata + 2) = 1;
                            *(idata + 3) = qchicken;
                            *(idata + 4) = (uint8_t)(idnumhex >> 24);
                            *(idata + 5) = (uint8_t)(idnumhex >> 16);
                            *(idata + 6) = (uint8_t)(idnumhex >> 8);
                            *(idata + 7) = (uint8_t)(idnumhex);
                            nowrtc = RTC_ReadTick();
                            *(idata + 8) = (uint8_t)(nowrtc >> 24);
                            *(idata + 9) = (uint8_t)(nowrtc >> 16);
                            *(idata + 10) = (uint8_t)(nowrtc >> 8);
                            *(idata + 11) = (uint8_t)(nowrtc);
                            
                            //将数据从UpDataArray挨个填充到待发送的数组idata中.
                            for(icount = 0; icount < FIRSTPACKETSIZE; icount++) {
                                *(idata + 12 + icount) = UpDataArray[icount];
                            }
                            
                            upcrc = CRC_16(0xffff, idata + 2, FIRSTPACKETSIZE + 16 - 6);
                            *(idata + FIRSTPACKETSIZE + 16 - 4) = (uint8_t)(upcrc >> 8);
                            *(idata + FIRSTPACKETSIZE + 16 - 3) = (uint8_t)(upcrc);
                            *(idata + FIRSTPACKETSIZE + 16 - 2) = 0xaa;
                            *(idata + FIRSTPACKETSIZE + 16 - 1) = 0x55;
                            //直接用socket发送
                            //WIFI_SocketSendData(idata, FIRSTPACKETSIZE + 16);
                            
                            Publish_MQTT(publishTopic, QOS0, idata, FIRSTPACKETSIZE + 16);
                            MMEMORY_FREE(idata);
                        }
                        //发送第二个包
                        idata = MMEMORY_ALLOC(THESECONDPACKETSIZE + 16);
                        if(idata != NULL) {
                            memset((char*)idata, 0xff, THESECONDPACKETSIZE + 16);
                            *idata = 0xaf;
                            *(idata + 1) = 0xfa;
                            *(idata + 2) = 2;
                            *(idata + 3) = qchicken;
                            *(idata + 4) = (uint8_t)(idnumhex >> 24);
                            *(idata + 5) = (uint8_t)(idnumhex >> 16);
                            *(idata + 6) = (uint8_t)(idnumhex >> 8);
                            *(idata + 7) = (uint8_t)(idnumhex);
                            *(idata + 8) = (uint8_t)(nowrtc >> 24);
                            *(idata + 9) = (uint8_t)(nowrtc >> 16);
                            *(idata + 10) = (uint8_t)(nowrtc >> 8);
                            *(idata + 11) = (uint8_t)(nowrtc);
                            for(icount = 0; icount < THESECONDPACKETSIZE; icount++) {
                                *(idata + 12 + icount) = UpDataArray[FIRSTPACKETSIZE + icount];
                            }
                            upcrc = CRC_16(0xffff, idata + 2, THESECONDPACKETSIZE + 16 - 6);
                            *(idata + THESECONDPACKETSIZE + 16 - 4) = (uint8_t)(upcrc >> 8);
                            *(idata + THESECONDPACKETSIZE + 16 - 3) = (uint8_t)(upcrc);
                            *(idata + THESECONDPACKETSIZE + 16 - 2) = 0xaa;
                            *(idata + THESECONDPACKETSIZE + 16 - 1) = 0x55;
                            WIFI_SocketSendData(idata, THESECONDPACKETSIZE + 16);    
                            
                            //Publish_MQTT(publishTopic, QOS0, idata, THESECONDPACKETSIZE + 16);
                            MMEMORY_FREE(idata);
                        }
                        if(qchicken >= 15) {
                        //if(qchicken >= 16) {
                            queryen = FALSE;
                            qchicken = 0;
                        }
                    }
                }
            }
        }
        if(TS_VOERFLOW(cmdtick, 1000)) {
            cmdtick = HAL_GetTick();
            CmdHanldeLogic();
        }
        TWDT_CLEAR(PTask);
    }
#endif
}

// void UpDataArray(uint8_t *uarr) {
//     uint8_t *udata = NULL;

// }

/**
 * @brief  获取当前鸡舍号
 * @note
 * @retval
 */
uint16_t GetChickenNumber() {
    uint16_t gchick = 0xffff;
    UART_SendData(UART_SP232_PORT, (uint8_t*)GetChickenNumberCmd, sizeof(GetChickenNumberCmd));
    gchick = WaitModbusReply_Single(GetChickenNumberCmd, 100);
    // DBG_LOG("GetChickenNumber:%d", gchick);
    return gchick;
}

/**
 * @brief  设置鸡舍号
 * @note
 * @param  snum:
 * @retval
 */
// 地址4X0710(0x02C4, 下面的02C5为偏移地址)为鸡舍地址号，先设置好鸡舍地址号再读写参数，所有鸡舍共
// 用以下相同地址；先确定鸡舍地址号，再读取相应地址的值即为对应鸡舍的
// 参数值；例如：地址4X0710 的值为0，4x0001 值为20，则表示鸡舍1 脉冲
// 温度偏差为2.0℃。若要读取鸡舍2 的脉冲温度偏差，则需要把地址4X0710
// 的值设置为1，然后读取4X0001 的值，读到的值即为2#鸡舍的脉冲温度偏
// 差
uint16_t SetChickenNumber(uint16_t snum) {
    uint8_t sarray[8];
    uint16_t scrc = 0;
    sarray[0] = MODBUS_SLAVE_ADDR;
    sarray[1] = MODBUS_WRITE_REG;
    sarray[2] = 0x02;
    sarray[3] = 0xC5;
    sarray[4] = (uint8_t)(snum >> 8);
    sarray[5] = (uint8_t)(snum);
    scrc = CRC_16(0xffff, sarray, 6);
    sarray[6] = (uint8_t)(scrc);
    sarray[7] = (uint8_t)(scrc >> 8);
    UART_SendData(UART_SP232_PORT, sarray, sizeof(sarray));
    return WaitModbusReply_Single(sarray, 100);
}

volatile uint8_t rxData = 0;

/**
 * @brief  获取寄存器参数
 * @note
 * @param  gaddr:寄存器地址
 * @param  glen:连续读寄存器长度
 * @param  *gbuf:
 * @retval
 */
BOOL GetRegisterData(uint16_t gaddr, uint16_t glen, uint8_t *gbuf) {
    BOOL gret = FALSE;
    uint8_t *grxbuf = NULL;
    uint8_t garray[8];
    uint8_t grxcrch = 0, grxcrcl = 0, gdatalen = 0, gi = 0;
    uint16_t gcrc = 0;
    uint16_t guartrxdatalen = 0;
    uint32_t guartidleticks = 0;
    uint32_t gsystick = 0;
    // 注意, 这里有个寄存器地址偏移, 为什么coder之前没有在这里做注释? 
    // 为什么?为什么?为什么?灵魂三问!
    gaddr -= 1;
    garray[0] = MODBUS_SLAVE_ADDR;
    garray[1] = MODBUS_READ_REG;
    garray[2] = (uint8_t)(gaddr >> 8);
    garray[3] = (uint8_t)(gaddr);
    garray[4] = (uint8_t)(glen >> 8);
    garray[5] = (uint8_t)(glen);
    gcrc = CRC_16(0xffff, garray, 6);
    garray[6] = (uint8_t)(gcrc);
    garray[7] = (uint8_t)(gcrc >> 8);
    UART_SendData(UART_SP232_PORT, garray, sizeof(garray));
    gsystick = HAL_GetTick();
    while(((guartrxdatalen == 0) || (guartidleticks < UART_IDLETICKOVER)) && (!TS_VOERFLOW(gsystick, 1000))) {
        guartrxdatalen = UART_DataSize(UART_SP232_PORT);
        guartidleticks = UART_GetDataIdleTicks(UART_SP232_PORT);
        osDelay(2);
    }

    if(!TS_VOERFLOW(gsystick, 1000)) {
        //DBG_LOG("GetRegisterAata()->guartrxdatalen:%d", guartrxdatalen);
        if((guartrxdatalen > 0) && (guartidleticks >= UART_IDLETICKOVER)) {
            // 申请内存               
            grxbuf = MMEMORY_ALLOC(guartrxdatalen);
            if(grxbuf != NULL) {
                // 从FIFO缓存中读出232串口传过来的数据.
                guartrxdatalen = UART_ReadData(UART_SP232_PORT, grxbuf, guartrxdatalen);
               
                // 做CRC校验
                gcrc = CRC_16(0xffff, grxbuf, guartrxdatalen - 2);
                grxcrch = *(grxbuf + guartrxdatalen - 1);
                grxcrcl = *(grxbuf + guartrxdatalen - 2);
                if((garray[0] == *grxbuf) && (garray[1] == *(grxbuf + 1)) &&
                        ((glen * 2) == *(grxbuf + 2)) && (gcrc == TWOBYTE_16BIT(grxcrch, grxcrcl))) {
                    gdatalen = *(grxbuf + 2);
                   
                    //DBG_LOG("GetRegisterAata()->gdatalen:%d", gdatalen);
                    for(; gdatalen > gi; gi++) {
                        *(gbuf + gi)=  *(grxbuf + 3 + gi);   
                        //*(gbuf + gi) = *(grxbuf + 3 + gi);
                    }
                    gret = TRUE;
                }
                MMEMORY_FREE(grxbuf);
            }
        }
    }
    return gret;
}


uint16_t SetRegisterData(uint16_t setreg, uint16_t setsata) {
    uint8_t setarray[8];
    uint16_t setcrc = 0;
    setreg -= 1;
    setarray[0] = MODBUS_SLAVE_ADDR;
    setarray[1] = MODBUS_WRITE_REG;
    setarray[2] = (uint8_t)(setreg >> 8);
    setarray[3] = (uint8_t)(setreg);
    setarray[4] = (uint8_t)(setsata >> 8);
    setarray[5] = (uint8_t)(setsata);
    setcrc = CRC_16(0xffff, setarray, 6);
    setarray[6] = (uint8_t)(setcrc);
    setarray[7] = (uint8_t)(setcrc >> 8);
    UART_SendData(UART_SP232_PORT, setarray, sizeof(setarray));
    return WaitModbusReply_Single(setarray, 100);
}


/**
 * @brief  等待Modbus响应，注意：该函数只是适应单个寄存器操作情况
 * @note
 * @param  *wcmd:
 * @param  wtick:
 * @retval
 */
uint16_t WaitModbusReply_Single(const uint8_t *wcmd, uint32_t wtick) {
    uint8_t  *wrxbuf = NULL;
    uint8_t  i = 0;
    uint8_t  wrxcrch = 0, wrxcrcl = 0;
    __IO uint16_t wreutrn = 0xffff;
    uint16_t wcrc = 0, wrxaddr = 0;
    uint16_t wuartrxdatalen = 0;
    uint32_t wuartidleticks = 0;
    uint32_t wsystick = 0;
    wsystick = HAL_GetTick();
    while(((wuartrxdatalen == 0) || (wuartidleticks < UART_IDLETICKOVER)) && (!TS_VOERFLOW(wsystick, wtick))) {
        wuartrxdatalen = UART_DataSize(UART_SP232_PORT);
        wuartidleticks = UART_GetDataIdleTicks(UART_SP232_PORT);
        osDelay(2);
    }
    if(!TS_VOERFLOW(wsystick, wtick)) {
        // DBG_LOG("wuartrxdatalen:%d", wuartrxdatalen);
        if((wuartrxdatalen > 0) && (wuartidleticks >= UART_IDLETICKOVER)) {
            wrxbuf = MMEMORY_ALLOC(wuartrxdatalen);
            if(wrxbuf != NULL) {
                wuartrxdatalen = UART_ReadData(UART_SP232_PORT, wrxbuf, wuartrxdatalen);
                wcrc = CRC_16(0xffff, wrxbuf, wuartrxdatalen - 2);
                wrxcrch = *(wrxbuf + wuartrxdatalen - 1);
                wrxcrcl = *(wrxbuf + wuartrxdatalen - 2);
                // wcrc == TWOBYTE_16BIT(*(wrxbuf + wuartrxdatalen - 1), *(wrxbuf + wuartrxdatalen - 2));
                // DBG_LOG("TWOBYTE_16BIT,CRC:%x,%x", TWOBYTE_16BIT(wrxcrch, wrxcrcl), wcrc);
                // DBG_LOG("wrxbuf:%x,%x", *wrxbuf, *(wrxbuf + 1));
                if((*(wcmd + 1) == MODBUS_WRITE_REG)) {
                    wrxaddr = TWOBYTE_16BIT(*(wrxbuf + 2), *(wrxbuf + 3));
                    wrxaddr -= 1;
                    *(wrxbuf + 2) = (uint8_t)(wrxaddr >> 8);
                    *(wrxbuf + 3) = (uint8_t)(wrxaddr);
                    // for(; wuartrxdatalen > i; i++) {
                    //     DBG_LOG("wrxbuf,wcmd:%x,%x", *(wrxbuf + i), *(wcmd + i));
                    //     // i++;
                    // }
                    if(arrayncmp((char*)wrxbuf, (char*)wcmd, wuartrxdatalen)) {
                        wreutrn &= (uint16_t)(*(wrxbuf + 4) << 8);
                        wreutrn |= (uint16_t)(*(wrxbuf + 5));
                    } else
                        DBG_LOG("WaitModbusReply_SingleError,FunctionCode:%x", MODBUS_WRITE_REG);
                } else if((*wrxbuf == *wcmd) && (*(wrxbuf + 1) == *(wcmd + 1)) && (wcrc == TWOBYTE_16BIT(wrxcrch, wrxcrcl))) {
                    // DBG_LOG("wcmd:%x,%x", *wcmd, *(wcmd + 1));
                    if((*(wrxbuf + 1) == MODBUS_READ_COIL) || (*(wrxbuf + 1) == MODBUS_READ_REG)) {
                        if(*(wrxbuf + 2) == 1) {
                            wreutrn &= (uint16_t)(*(wrxbuf + 3));
                        } else if(*(wrxbuf + 2) == 2) {
                            wreutrn &= (uint16_t)(*(wrxbuf + 3) << 8);
                            wreutrn |= (uint16_t)(*(wrxbuf + 4));
                        } else {
                            DBG_WAR("WaitModbusReply_Single->Data is too long:%x", *(wrxbuf + 2));
                        }
                    } else if(*(wrxbuf + 1) == MODBUS_WRITE_COIL) {
                        // wrxaddr = TWOBYTE_16BIT(*(wrxbuf + 2), *(wrxbuf + 3));
                        // wrxaddr -= 1;
                        wreutrn &= (uint16_t)(*(wrxbuf + 4) << 8);
                        wreutrn |= (uint16_t)(*(wrxbuf + 5));
                    }
                } else {
                    DBG_LOG("WaitModbusReply_SingleError%x,%x", wcrc, TWOBYTE_16BIT(wrxcrch, wrxcrcl));
                }
                MMEMORY_FREE(wrxbuf);
            }
        } else
            DBG_LOG("WaitModbusReply_Single UART ERROR");
    } else {
        DBG_LOG("WaitModbusReply_SingleOutTime");
        //for(i = 0; i < 8; i++)
            //DBG_LOG("Send:%x", *(wcmd + i));
    }
    // DBG_LOG("WaitModbusReply_Single->return:%x", wreutrn);
    return wreutrn;
}

/**
 * @brief  设置鸡舍启用或关闭
 * @note
 * @param  saddr: 鸡舍线圈地址
 * @param  en: 1为启用，0为关闭
 * @retval
 */
uint8_t SetChickenStatus(uint16_t saddr, uint8_t en) {
    uint8_t sarr[8];
    uint16_t scrc = 0;
    uint16_t sret = 0;
    saddr -= 1;
    sarr[0] = MODBUS_SLAVE_ADDR;
    sarr[1] = MODBUS_WRITE_COIL;
    sarr[2] = (uint8_t)(saddr >> 8);
    sarr[3] = (uint8_t)(saddr);
    sarr[4] = 0;
    if(en)
        sarr[5] = 1;
    else
        sarr[5] = 0;
    scrc = CRC_16(0xffff, sarr, sizeof(sarr) - 2);
    sarr[6] = (uint8_t)(scrc);
    sarr[7] = (uint8_t)(scrc >> 8);
    UART_SendData(UART_SP232_PORT, sarr, sizeof(sarr));
    sret =  WaitModbusReply_Single(sarr, 100);
    return (uint8_t)sret;
}

uint8_t ifChickenHouseOnline(uint8_t chickenHouse)
{
    uint8_t garr[8];
    uint16_t gcrc = 0;
    garr[0] = MODBUS_SLAVE_ADDR;
    garr[1] = MODBUS_READ_COIL;
    garr[2] = 0;
    // 注意这里并没有地址偏移, 50就是1号鸡舍的寄存器地址, chickinHouse 0即1号鸡舍
    garr[3] = 50 + chickenHouse;
    garr[4] = 0;
    garr[5] = 1;
    gcrc = CRC_16(0xffff, garr, sizeof(garr) - 2);
    garr[6] = (uint8_t)(gcrc);
    garr[7] = (uint8_t)(gcrc >> 8);
    UART_SendData(UART_SP232_PORT, garr, sizeof(garr));
    return (uint8_t)WaitModbusReply_Single(garr, 100); 
}
/**
 * @brief  获取鸡舍状态
 * @note
 * @param  gaddr: 鸡舍线圈地址
 * @retval 1为开启，0为关闭
 */
uint8_t GetChickenStatus(uint16_t gaddr) {
    uint8_t garr[8];
    uint16_t gcrc = 0;
    gaddr -= 1;
    garr[0] = MODBUS_SLAVE_ADDR;
    garr[1] = MODBUS_READ_COIL;
    garr[2] = (uint8_t)(gaddr >> 8);
    garr[3] = (uint8_t)(gaddr);
    garr[4] = 0;
    garr[5] = 1;
    gcrc = CRC_16(0xffff, garr, sizeof(garr) - 2);
    garr[6] = (uint8_t)(gcrc);
    garr[7] = (uint8_t)(gcrc >> 8);
    UART_SendData(UART_SP232_PORT, garr, sizeof(garr));
    return (uint8_t)WaitModbusReply_Single(garr, 100);
}

/**
 * @brief  上报鸡舍状态
 * @note
 * @param  umessageid:
 * @retval TRUE:获取成功，FALSE:获取失败
 */
BOOL UpChickenStatus(uint32_t umessageid) {
    BOOL uret = FALSE;
    uint8_t ui = 0, getret = 0;
    uint8_t unumarr[5];
    // uint16_t usta = 0;
    cJSON *udis = NULL;
    udis = cJSON_CreateObject();
    if(udis != NULL) {
        for(ui = 0; ui < 16; ui++) {
            // usta >>= 1;
            getret = GetChickenStatus(HouseAddr[ui]);
            // DBG_LOG("UpChickenStatus->getret:%x,ui:%x", getret, ui);
            // memset((char *)unumarr, 0, 5);
            if(getret != 0xff) {
                if(getret) {
                    cJSON_AddNumberToObject(udis, uitoa(ui + 1, (char*)unumarr), 1);
                } else {
                    cJSON_AddNumberToObject(udis, uitoa(ui + 1, (char*)unumarr), 0);
                }
            } else
                break;
        }
        if(ui < 16) {
            cJSON_Delete(udis);
            CMD_Confirm_Rsp(umessageid, ret_fault);
        } else {
            if(CMD_Updata("CMD-103", udis))
                uret = TRUE;
        }
    }
    return uret;
}

/**
 * @brief  上报报警参数
 * @note
 * @param  uanum: 鸡舍号（1-16）注17为选择所有
 * @param  umessageid:
 * @retval
 */
void UpAlarmParameter(uint8_t uanum, uint32_t umessageid) {
    uint8_t uai = 0, tnum = 0;
    uint8_t readcount = 0;
    uint8_t readq = 0;
    static uint8_t readpar[ALARMDATA_REG_QUANTITY * 2];
    uint32_t uptick = 0;
    cJSON *UpAlarm = NULL;
    tnum = uanum - 1;
    if(uanum != 17) {
        uai = tnum + 1;
    } else {
        uai = 16;
        tnum = 0;
    }
    for(; tnum < uai; tnum++) {
        // DBG_LOG("UpAlarmParameter->tnum:%x", tnum);
        if(tnum != GetChickenNumber()) {
            if(tnum != SetChickenNumber(tnum)) {
                DBG_WAR("UpAlarmParameter->Chicken house switching failed");
                break;
            } else {
                uptick = HAL_GetTick();
                while((readcount <= ALARMDATA_REG_QUANTITY - 1) && !(TS_VOERFLOW(uptick, 1000))) {
                    if(ChickenAlarmPar[readcount] == 409)
                        readq = 10;
                    else if(ChickenAlarmPar[readcount] == 382)
                        readq = 20;
                    else
                        readq = 1;
                    if(GetRegisterData(ChickenAlarmPar[readcount], readq, &readpar[readcount * 2]))
                        readcount += readq;
                }
                if(!(TS_VOERFLOW(uptick, 1000))) {
                    UpAlarm = cJSON_CreateObject();
                    if(UpAlarm != NULL) {
                        cJSON_AddNumberToObject(UpAlarm, "chicken", tnum + 1);
                        cJSON_AddNumberToObject(UpAlarm, "ms", TWOBYTE_16BIT(readpar[0], readpar[1]));
                        cJSON_AddNumberToObject(UpAlarm, "alarmen", TWOBYTE_16BIT(readpar[2], readpar[3]));
                        cJSON_AddNumberToObject(UpAlarm, "sms", TWOBYTE_16BIT(readpar[4], readpar[5]));
                        cJSON_AddNumberToObject(UpAlarm, "inspection", TWOBYTE_16BIT(readpar[6], readpar[7]));
                        cJSON_AddNumberToObject(UpAlarm, "alarmh", TWOBYTE_16BIT(readpar[8], readpar[9]));
                        cJSON_AddNumberToObject(UpAlarm, "alarms", TWOBYTE_16BIT(readpar[10], readpar[11]));
                        cJSON_AddNumberToObject(UpAlarm, "inspectionh", TWOBYTE_16BIT(readpar[12], readpar[13]));
                        cJSON_AddNumberToObject(UpAlarm, "inspections", TWOBYTE_16BIT(readpar[14], readpar[15]));
                        cJSON_AddNumberToObject(UpAlarm, "useren1", TWOBYTE_16BIT(readpar[16], readpar[17]));
                        cJSON_AddNumberToObject(UpAlarm, "useren2", TWOBYTE_16BIT(readpar[18], readpar[19]));
                        cJSON_AddNumberToObject(UpAlarm, "useren3", TWOBYTE_16BIT(readpar[20], readpar[21]));
                        cJSON_AddNumberToObject(UpAlarm, "useren4", TWOBYTE_16BIT(readpar[22], readpar[23]));
                        cJSON_AddNumberToObject(UpAlarm, "useren5", TWOBYTE_16BIT(readpar[24], readpar[25]));
                        cJSON_AddNumberToObject(UpAlarm, "inspection1", TWOBYTE_16BIT(readpar[26], readpar[27]));
                        cJSON_AddNumberToObject(UpAlarm, "inspection2", TWOBYTE_16BIT(readpar[28], readpar[29]));
                        cJSON_AddNumberToObject(UpAlarm, "inspection3", TWOBYTE_16BIT(readpar[30], readpar[31]));
                        cJSON_AddNumberToObject(UpAlarm, "inspection4", TWOBYTE_16BIT(readpar[32], readpar[33]));
                        cJSON_AddNumberToObject(UpAlarm, "inspection5", TWOBYTE_16BIT(readpar[34], readpar[35]));
                        cJSON_AddNumberToObject(UpAlarm, "ago1", TWOBYTE_16BIT(readpar[36], readpar[37]));
                        cJSON_AddNumberToObject(UpAlarm, "mid1", TWOBYTE_16BIT(readpar[38], readpar[39]));
                        cJSON_AddNumberToObject(UpAlarm, "rear1", TWOBYTE_16BIT(readpar[40], readpar[41]));
                        cJSON_AddNumberToObject(UpAlarm, "usersms1", TWOBYTE_16BIT(readpar[42], readpar[43]));
                        cJSON_AddNumberToObject(UpAlarm, "ago2", TWOBYTE_16BIT(readpar[44], readpar[45]));
                        cJSON_AddNumberToObject(UpAlarm, "mid2", TWOBYTE_16BIT(readpar[46], readpar[47]));
                        cJSON_AddNumberToObject(UpAlarm, "rear2", TWOBYTE_16BIT(readpar[48], readpar[49]));
                        cJSON_AddNumberToObject(UpAlarm, "usersms2", TWOBYTE_16BIT(readpar[50], readpar[51]));
                        cJSON_AddNumberToObject(UpAlarm, "ago3", TWOBYTE_16BIT(readpar[52], readpar[53]));
                        cJSON_AddNumberToObject(UpAlarm, "mid3", TWOBYTE_16BIT(readpar[54], readpar[55]));
                        cJSON_AddNumberToObject(UpAlarm, "rear3", TWOBYTE_16BIT(readpar[56], readpar[57]));
                        cJSON_AddNumberToObject(UpAlarm, "usersms3", TWOBYTE_16BIT(readpar[58], readpar[59]));
                        cJSON_AddNumberToObject(UpAlarm, "ago4", TWOBYTE_16BIT(readpar[60], readpar[61]));
                        cJSON_AddNumberToObject(UpAlarm, "mid4", TWOBYTE_16BIT(readpar[62], readpar[63]));
                        cJSON_AddNumberToObject(UpAlarm, "rear4", TWOBYTE_16BIT(readpar[64], readpar[65]));
                        cJSON_AddNumberToObject(UpAlarm, "usersms4", TWOBYTE_16BIT(readpar[66], readpar[67]));
                        cJSON_AddNumberToObject(UpAlarm, "ago5", TWOBYTE_16BIT(readpar[68], readpar[69]));
                        cJSON_AddNumberToObject(UpAlarm, "mid5", TWOBYTE_16BIT(readpar[70], readpar[71]));
                        cJSON_AddNumberToObject(UpAlarm, "rear5", TWOBYTE_16BIT(readpar[72], readpar[73]));
                        cJSON_AddNumberToObject(UpAlarm, "usersms5", TWOBYTE_16BIT(readpar[74], readpar[75]));
                        CMD_Updata("CMD-105", UpAlarm);
                        // uret = TRUE;
                    }
                } else {
                    CMD_Confirm_Rsp(umessageid, ret_fault);
                }
            }
        }
    }
}

/**
 * @brief  设置命令处理逻辑
 * @note
 * @retval None
 */
void CmdHanldeLogic(void) {
    CmdTYPE_t newcmd, newtcmd;
    BOOL cret = FALSE;
    if((uxQueueMessagesWaiting(CmdHandle) > 0) && (xQueueReceive(CmdHandle, &newcmd, 0) == pdTRUE)) {
        if(newcmd.addr == 0xffff) {
            UpChickenStatus(newcmd.mid);
        } else if(newcmd.addr == 0xfffe) {
            UpAlarmParameter(newcmd.chid, newcmd.mid);
        } else if((50 <= newcmd.addr) && (65 >= newcmd.addr)) {
            if(newcmd.value != SetChickenStatus(newcmd.addr, newcmd.value))
                CMD_Confirm_Rsp(newcmd.mid, ret_fault);
            else
                CMD_Confirm_Rsp(newcmd.mid, ret_ok);
        } else {
            if((newcmd.chid - 1) != GetChickenNumber()) {
                if((newcmd.chid - 1) != SetChickenNumber(newcmd.chid - 1)) {
                    DBG_WAR("CmdHanldeLogic->Chicken house switching failed messageid:%x", newcmd.mid);
                }
            }
            if(newcmd.chid - 1 == GetChickenNumber()) {
                if(newcmd.value == SetRegisterData(newcmd.addr, newcmd.value)) {
                    cret = TRUE;
                }
            }
            if(cret == TRUE) {
                if((uxQueueMessagesWaiting(CmdHandle) > 0) && (xQueuePeek(CmdHandle, &newtcmd, 0) == pdTRUE)) {
                    if(newtcmd.mid != newcmd.mid)
                        CMD_Confirm_Rsp(newcmd.mid, ret_ok);
                } else
                    CMD_Confirm_Rsp(newcmd.mid, ret_ok);
            } else {
                CMD_Confirm_Rsp(newcmd.mid, ret_fault);
            }
        }
    }
}

/**
 * 向服务器发送数据
 *
 * @param cmd     发送的命令
 * @param desired 子结构
 * @return 返回发送结果
 */
BOOL CMD_Updata(char* cmd, cJSON * desired) {
    BOOL ret = FALSE;
    char* s = NULL;
    // char msgidBuf[20];
    cJSON* root = NULL;
    root = cJSON_CreateObject();
    if (root != NULL) {
        // uitoa(HAL_GetTick(), msgidBuf);
        // cJSON_AddStringToObject(root, "messageid", msgidBuf);
        cJSON_AddNumberToObject(root, "messageid", HAL_GetTick());
        cJSON_AddNumberToObject(root, "timestamp", RTC_ReadTick());
        cJSON_AddStringToObject(root, "cmd", cmd);
        cJSON_AddStringToObject(root, "deviceid", WorkParam.mqtt.MQTT_ClientID);
        cJSON_AddItemToObjectCS(root, "desired", desired);
        s = cJSON_PrintUnformatted(root);
        if (s != NULL) {
            DBG_INFO("CMD_Updata ts:%u,data:%s", HAL_GetTick(), s);
            ret = Publish_MQTT(publishTopic, QOS0, (uint8_t*)s, strlen(s));
            MMEMORY_FREE(s);
        }
        cJSON_Delete(root);
    }
    return ret;
}

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


/**
 * 通讯板参数与状态上传
 */
void Status_Updata(void) {
    cJSON* desired = NULL;
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
 * @brief  鸡舍管理命令解析
 * @note
 * @param  messageid: 消息ID
 * @param  *dis:
 * @retval None
 */
void ChickenManagementAnalysis(uint32_t messageid, cJSON *dis) {
    CmdTYPE_t  *cmcmdtype = NULL;
    cJSON *number = NULL, *status = NULL;
    if(dis != NULL) {
        number = cJSON_GetObjectItem(dis, "number");
        status = cJSON_GetObjectItem(dis, "status");
        if((number != NULL) && (status != NULL) && (number->type == cJSON_Number) && (status->type == cJSON_Number)) {
            cmcmdtype = MMEMORY_ALLOC(sizeof(CmdTYPE_t));
            if(cmcmdtype != NULL) {
                cmcmdtype->mid = messageid;
                cmcmdtype->chid = number->valueint;
                cmcmdtype->addr = HouseAddr[number->valueint - 1];
                cmcmdtype->value = status->valueint;
                xQueueSend(CmdHandle, cmcmdtype, portMAX_DELAY);
                MMEMORY_FREE(cmcmdtype);
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

// /**
//  * 接收处理
//  *
//  * @param dat    接收到的数据指针
//  * @param len    数据长度
//  */
// static void ArrivePath(uint8_t* dat, uint16_t len) {
//     uint32_t tsdiff = 0, ts = 0;
//     char temp[48] = { 0, 0 };
//     proret_t ret = ret_ok;
//     cJSON* root = NULL, *msgid = NULL, *timestamp = NULL, *cmd = NULL, *desired = NULL, *deviceid = NULL;
//     *(dat + len) = 0;
//     DBG_INFO("ArrivePath ts:%u,data:%s", HAL_GetTick(), dat);
//     root = cJSON_Parse((const char*)dat);
//     if (root != NULL) {
//         msgid = cJSON_GetObjectItem(root, "messageid");
//         deviceid = cJSON_GetObjectItem(root, "deviceid");
//         if (msgid != NULL && msgid->type == cJSON_String
//                 && deviceid != NULL &&
//                 (strcmp(deviceid->valuestring, WorkParam.mqtt.MQTT_ClientID) == 0
//                  || deviceid->valuestring[0] == '0')) {
//             timestamp = cJSON_GetObjectItem(root, "timestamp");
//             desired = cJSON_GetObjectItem(root, "desired");
//             cmd = cJSON_GetObjectItem(root, "cmd");
//             /*RTC校时*/
//             if (timestamp != NULL && timestamp->type == cJSON_Number) {
//                 ts = timestamp->valueint;
//                 tsdiff = RTC_ReadTick();
//                 tsdiff = abs(ts - tsdiff);
//             }
//             if (STR_EQUAL(cmd->valuestring, "CMD-07")) {
//                 strcpy(playid, msgid->valuestring);
//             }
//             if (tsdiff < 30 || STR_EQUAL(cmd->valuestring, "CMD-02")) {
//                 strcpy(temp, msgid->valuestring);
//                 ret = ArriveDataProc(cmd->valuestring, desired);
//             } else {
//                 DBG_WAR("tsdiff error:%d", tsdiff);
//             }
//         }
//         cJSON_Delete(root);
//     }
//     if (temp[0] != 0) {
//         // CMD_Confirm_Rsp(temp, ret);
//     }
// }

// /**
//  * 消息处理
//  *
//  * @param desired 接收到的消息参数
//  * @return 返回执行结果
//  */
// static proret_t ArriveDataProc(char* cmd, cJSON * desired) {
//     proret_t ret = ret_ok;
//     cJSON* child = NULL;
//     uint8_t save = 0;
//     if (STR_EQUAL(cmd, "CMD-01")) {
//         child = cJSON_GetObjectItem(desired, "devicereset");
//         if (child != NULL && child->type == cJSON_True) {
//             NVIC_SystemReset();
//         }
//         child = cJSON_GetObjectItem(desired, "devicefactoryreset");
//         if (child != NULL && child->type == cJSON_True) {
//             WorkParam.mqtt.MQTT_Port = MQTT_PORT_DEF;
//             WorkParam.mqtt.MQTT_PingInvt = MQTT_PING_INVT_DEF;
//             strcpy(WorkParam.mqtt.MQTT_Server, MQTT_SERVER_DEF);
//             strcpy(WorkParam.mqtt.MQTT_UserName, MQTT_USER_DEF);
//             strcpy(WorkParam.mqtt.MQTT_UserPWD, MQTT_PWD_DEF);
//             WorkParam_Save();
//         }
//         child = cJSON_GetObjectItem(desired, "deviceparamget");
//         if (child != NULL && child->type == cJSON_True) {
//             Status_Updata();
//         }
//         child = cJSON_GetObjectItem(desired, "scanstation");
//         if (child != NULL && child->type == cJSON_True) {
//             MCPU_ENTER_CRITICAL();
//             // ScanStationStart = TRUE;
//             MCPU_EXIT_CRITICAL();
//         }
//     } else if (STR_EQUAL(cmd, "CMD-02")) {
//         child = cJSON_GetObjectItem(desired, "timestamp");
//         if (child != NULL && child->type == cJSON_Number) {
//             timeRTC_t time;
//             RTC_TickToTime(child->valueint, &time);
//             RTC_SetTime(&time);
//         }
//         child = cJSON_GetObjectItem(desired, "ip");
//         if (child != NULL && child->type == cJSON_String
//                 && !STR_EQUAL(WorkParam.mqtt.MQTT_Server, child->valuestring)) {
//             strcpy(WorkParam.mqtt.MQTT_Server, child->valuestring);
//             save++;
//         }
//         child = cJSON_GetObjectItem(desired, "username");
//         if (child != NULL && child->type == cJSON_String
//                 && !STR_EQUAL(WorkParam.mqtt.MQTT_UserName, child->valuestring)) {
//             strcpy(WorkParam.mqtt.MQTT_UserName, child->valuestring);
//             save++;
//         }
//         child = cJSON_GetObjectItem(desired, "userpwd");
//         if (child != NULL && child->type == cJSON_String
//                 && !STR_EQUAL(WorkParam.mqtt.MQTT_UserPWD, child->valuestring)) {
//             strcpy(WorkParam.mqtt.MQTT_UserPWD, child->valuestring);
//             save++;
//         }
//         child = cJSON_GetObjectItem(desired, "port");
//         if (child != NULL && child->type == cJSON_Number
//                 && WorkParam.mqtt.MQTT_Port != child->valueint) {
//             WorkParam.mqtt.MQTT_Port = child->valueint;
//             save++;
//         }
//         child = cJSON_GetObjectItem(desired, "heartbeat");
//         if (child != NULL && child->type == cJSON_Number
//                 && WorkParam.mqtt.MQTT_PingInvt != child->valueint) {
//             WorkParam.mqtt.MQTT_PingInvt = child->valueint;
//             save++;
//         }
//         if (save > 0) {
//             WorkParam_Save();
//         }
//     }
//     /*开门*/
//     else if (STR_EQUAL(cmd, "CMD-03")) {
//         child = cJSON_GetObjectItem(desired, "addr");
//         if (child != NULL && child->type == cJSON_Number) {
//             // xaddr =
//             // if (STR_EQUAL(child->valuestring, "in")) {
//             //     DBG_LOG("door open get in");
//             //     // Control_DoorOpen(DOOR_OPEN_IN);
//             // } else if (STR_EQUAL(child->valuestring, "out")) {
//             //     DBG_LOG("door open get out");
//             //     // Control_DoorOpen(DOOR_OPEN_OUT);
//             // } else if(STR_EQUAL(child->valuestring, "over")) {
//             //     DBG_LOG("no money");
//             //     // GPRS_TTS("亲，余额不足，请及时充值欧");
//             // }
//         }
//     }
//     return ret;
// }