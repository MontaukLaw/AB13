/* Includes ------------------------------------------------------------------*/
#include "user_comm.h"

#define FIRST_PKG_REGS_LENGTH     194
#define SECOND_PKG_REGS_LENGTH    192

#define THIRD_PKG_REGS_LENGTH     193
#define FORTH_PKG_REGS_LENGTH     137

#define FAULT_DATA_START_BYTE     ((FORTH_PKG_REGS_LENGTH - FAULT_REG_LIST_LENGTH) * 2)
#define FAULT_REG_LIST_LENGTH      34

#define FAULT_BYTE_LENGTH     (FAULT_REG_LIST_LENGTH*2)    
#define CONTINOUSE_FAULT_REG_LIST_LENGTH   7

uint8_t faultArr[FAULT_BYTE_LENGTH];

// 连续要读的故障寄存器地址开始位
// 1500开始的寄存器, 要连续读24位
// 66开始的寄存器, 读1位
 
const uint16_t CHICKEN_HOUSE_FAULT_START_REG_ADDR[] = {1500, 66, 1885, 1154, 632, 631, 633};
// 下面几个长度的和, 就是上面fautl数组的寄存器数量
const uint8_t CONTINOUSE_CHICKEN_HOUSE_FAULT_START_REG_LENGTH[FAULT_BYTE_LENGTH/2] = {24, 1, 5, 1, 1, 1, 1};

// 读取寄存器的首地址列表     
const uint16_t READING_START_REG[READ_TIMES_PER_CHICKEN_HOUSE]= {1,60,107,160,210,260,310,360,466,501,541,582,695,\
                                  731,783,802,888,948,1054,1118,1140,1187,2001,2054,2151,2201};

// 读取每段寄存器长度
const uint8_t READING_LENGTH_ARR[READ_TIMES_PER_CHICKEN_HOUSE]={39,47,23,50,50,50,50,18,4,18,20,9,7,48,15,48,2,30,9,1,29,9,48,48,50,32};     

// 第一个包的寄存器地址数组
const uint16_t FIRST_PKG_REGS[]={700,1062,701,508,509,510,582,513,512,511,583,584,518,1187,1193,305,256,255,552,551,300,\
        306,301,304,17,307,308,309,310,311,312,313,314,315,316,317,318,319,320,501,502,503,504,505,506,507,1161,1162,1163,1164,\
        1165,1166,1167,1168,541,542,543,544,545,546,547,548,549,550,560,257,\
        258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,275,\
        276,888,889,1,2,695,5,6,7,8,10,11,13,14,69,70,71,72,12,3,4,60,\
        15,16,9,18,19,20,21,22,23,24,25,26,27,28,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,\
        119,120,121,122,123,124,125,126,127,128,129,948,949,950,951,952,953,954,955,956,957,958,959,960,961,962,963,964,\
        965,966,967,968,969,970,971,972,973,974,975,976,977,582,586,587,783,784,785,786,63,64,65,588,589,590,1054,1055,1056
};

// 第二个包的寄存器地址数组
const uint16_t SECOND_PKG_REGS[]={88,85,86,89,87,93,94,95,96,97,98,99,90,91,75,76,77,78,79,80,81,82,281,\
    282,283,284,285,286,287,288,289,290,291,292,293,294,295,296,788,789,466,467,468,469,797,374,377,376,731,\
        732,733,734,735,736,737,738,739,740,741,742,743,744,745,746,747,748,749,750,751,752,753,754,755,756,\
    757,758,759,760,761,762,763,764,765,766,767,768,769,770,771,772,773,774,775,776,777,778,802,803,804,805,\
        806,807,808,809,810,811,812,813,814,815,816,817,818,819,820,821,822,823,824,825,826,827,828,829,830,\
    831,832,833,834,835,836,837,838,839,840,841,842,843,844,845,846,847,848,849,2001,2002,2003,2004,2005,2006,\
        2007,2008,2009,2010,2011,2012,2013,2014,2015,2016,2017,2018,2019,2020,2021,2022,2023,2024,2025,2026,2027,\
    2028,2029,2030,2031,2032,2033,2034,2035,2036,2037,2038,2039,2040,2041,2042,2043,2044,2045,2046,2047,2048
};

// 第三个包的寄存器地址数组
const uint16_t THIRD_PKG_REGS[]={2054,2055,2056,2057,2058,2059,2060,2061,2062,2063,2064,2065,2066,2067,2068,2069,\
    2070,2071,2072,2073,2074,2075,2076,2077,2078,2079,2080,2081,2082,2083,2084,2085,2086,2087,2088,2089,2090,2091,\
        2092,2093,2094,2095,2096,2097,2098,2099,2100,2101,2223,2224,2225,2226,2227,2228,2229,2230,2231,2232,321,322,\
    323,324,325,326,327,328,329,330,331,332,333,334,335,336,337,338,339,340,341,342,343,344,345,346,347,348,349,350,\
        351,352,353,354,355,356,357,358,359,360,361,362,363,364,365,366,367,368,2151,2152,2153,2154,2155,2156,2157,2158,\
    2159,2160,2161,2162,2163,2164,2165,2166,2167,2168,2169,2170,2171,2172,2173,2174,2175,2176,2177,2178,2179,2180,2181,\
    2182,2183,2184,2185,2186,2187,2188,2189,2190,2191,2192,2193,2194,2195,2196,2197,2198,2199,2200,2201,2202,2203,2204,\
    2205,2206,2207,2208,2209,2210,2211,2212,2213,2214,2215,2216,2217,2218,2219,2220,2221,2222,180,181,182,183,184,185,186,\
    187,188,62,190,73,177,297,298
};

// 第四个包的寄存器地址数组
const uint16_t FORTH_PKG_REGS[]={170,171,172,173,174,175,176,211,212,213,214,1118,1189,216,217,218,219,1194,1195,39,61,\
              29,30,31,32,33,34,35,36,37,38,160,161,162,163,164,165,166,167,168,169,191,192,193,194,195,196,197,198,199,\
              200,201,202,203,204,205,206,207,208,209,210,1140,1141,1142,1143,1144,1145,1146,1147,1148,1149,251,3,221,\
              222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,\
              248,249,250,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
// 寄存器数据缓存
static uint8_t regDataBuffer[RELAY_REG_DATA_BUFFER_LENGTH]; 

// 所有鸡舍设置
uint8_t allChickenHouseSetting[CHICKEN_HOUSE_SETTING_ARRAY_LENGTH];

// 发送字节数组最大应该是第一个包, 404个字节.
extern uint8_t gTxBuffer[MAX_PACKAGE_SIZE];

//发送01 03到中继
void sendReadingSignal2Relay(uint8_t startRegAddress, uint8_t regcontinuousNumber,uint16_t regDataBufferIndex){
   

}

void cleanRegDataBuffer(void){
    uint16_t i = 0 ;
    for(i = 0; i<RELAY_REG_DATA_BUFFER_LENGTH; i++){
        regDataBuffer[i] = 0;    
    }
}

// 清零数组
void clearArrayData(uint8_t* array, uint16_t length){
    uint16_t index = 0;
    for(;index < length; index++){
       *array = 0;    
    }
}

// 寻找寄存器数据在缓存中的位置
uint16_t findRegDataPos(uint16_t reg){
    uint8_t i=0;
    // 缓存的地址段
    uint8_t regDataPosSegment = 0;
    //uint8_t highPos, lowPos;
    uint16_t regDataPosSegmentStartAddr = 0;
    uint16_t regBufferIndex = 0;
    // 实地测试
    // uint8_t testArr[2]={0,0};
    // 测试成功, 注释掉.
    
    // 例如, 查找寄存器 700
    for(i=0; i< READ_TIMES_PER_CHICKEN_HOUSE; i++){  
        
        // 段起始寄存器地址
        regDataPosSegmentStartAddr = READING_START_REG[i];
        
     
        // 如果下一个连续寄存器段的起始地址比reg要大, 就break出来.
        if(READING_START_REG[i+1] > reg){
            break;
        }  
        
        // 寄存器缓存的段地址      
        regBufferIndex = regBufferIndex + READING_LENGTH_ARR[i];                 
                       
    }
    
    // reg700break之后, 段首地址为695
    // 获取缓冲区内的确切地址(寄存器地址, 非字节地址)
    regBufferIndex = regBufferIndex + (reg - regDataPosSegmentStartAddr);
    
    return regBufferIndex;
    //highPos = regDataBuffer[regBufferIndex * 2];
    //lowPos = regDataBuffer[regBufferIndex * 2 + 1];   
    
    // 测试完, 注释掉
    // GetRegisterData(reg, 1, testArr);
    
    // 测试一下, 经过测试(5个实际实时寄存器读与buffer比较), 能拿到跟实测一致的数据.
    // DBG_LOG("..");
    
}

// 更decent的写法是直接传指针
void transDataFromRegDataBufferToTxBuffer(const uint16_t* regAddrList, uint16_t regAddrListLength, uint8_t packageNumber){
    uint8_t highPos, lowPos;
    uint16_t i, arrLength; 
    uint16_t txBufferIndex = 0;
    uint16_t regBufferIndex = 0;
    uint16_t dataRegAddr = 0;    
    
    // 数组清零
    clearArrayData(gTxBuffer,MAX_PACKAGE_SIZE);
    
    // 只有第一个包需要先转鸡舍对应的设置
    if(packageNumber == 1){
        // 先搬第一个鸡舍的设置
        for(; txBufferIndex < 16; txBufferIndex ++){
            gTxBuffer[txBufferIndex] = allChickenHouseSetting[txBufferIndex];            
        } 
    }
    
    // 这里除2的原因是uint16
    // arrLength = regAddrListLength;
    
    // 关键的部分
    for(i=0; i< regAddrListLength; i++){
        // 取出需要的寄存器地址
        dataRegAddr = *(regAddrList + i);
        
        // 找到寄存器数据所保存的位置
        regBufferIndex = findRegDataPos(dataRegAddr);
        
        // 分解出2个byte
        highPos = regDataBuffer[regBufferIndex * 2];
        lowPos = regDataBuffer[regBufferIndex * 2 + 1];
        
        // 分别写入到待发送缓存数组中
        gTxBuffer[txBufferIndex]=highPos;
        gTxBuffer[txBufferIndex + 1]=lowPos;
        
        // 索引向后移两位
        txBufferIndex = txBufferIndex + 2;

    }
        
    
#ifdef UNIT_TEST
    
    //DBG_LOG("Just for break point.");
    
#endif    
    
}


#ifdef UNIT_TEST
void transTest(const uint16_t* regList){
    uint16_t a= *regList;
    uint16_t b= *(regList + 1);
    DBG_LOG("Just for break point.");
}
#endif 

//这里鸡舍号以0开始
void readFaultRegs(uint8_t chickenHouseNumber)
{
    uint8_t i;
    uint16_t regContinousLength = 0;
    uint16_t writeIdx = 0;
    for(i =0; i< CONTINOUSE_FAULT_REG_LIST_LENGTH; i++){
        // CONTINOUSE_CHICKEN_HOUSE_FAULT_START_REG_LENGTH
        // CHICKEN_HOUSE_FAULT_START_REG_ADDR
        // 获取本次连续读的长度
        regContinousLength = CONTINOUSE_CHICKEN_HOUSE_FAULT_START_REG_LENGTH[i];
            
        GetRegisterData(CHICKEN_HOUSE_FAULT_START_REG_ADDR[i] + (chickenHouseNumber*regContinousLength), regContinousLength, &faultArr[writeIdx]);
        
        // 将寄存器地址位转成byte地址位要乘2
        writeIdx = writeIdx + regContinousLength*2;
    }

}
// 新的版本, 发现一个bug, 鸡舍设置寄存器并非全部连续, 需要挨个读. 
// 挨个读的还有故障码
void getAllChickenHouseSetting(uint8_t chickenHouseOnlineState){
    uint16_t regStartAddr = CHICKEN_HOUSE_SETTING_START_REG_ADDR;   
    uint8_t i;
    uint16_t regCounter = 0;
    for(i = 0 ; i< CHICKEN_HOUSE_QUANTITY_MAX; i++){
        //如果该鸡舍在线, 就读, 并赋值, 不然就拉**倒
        if(chickenHouseOnlineState >> i && 1){
            // 读8个寄存器
            GetRegisterData(regStartAddr, CHICKEN_HOUSE_SETTING_REG_QUANTITY, &allChickenHouseSetting[regCounter]); 
            
            readFaultRegs(i);
            //faltArr[34];
        }
        // 该数数还得数数
        regCounter = regCounter + 16;
        regStartAddr = regStartAddr + 8;
    }

}
#if 0
// 获取一共128个寄存器的16个鸡舍的设置信息
void getAllChickenHouseSetting_old(void){
    uint16_t regStartAddr = CHICKEN_HOUSE_SETTING_START_REG_ADDR;
    
    // 一共128个寄存器, 分三次读吧, 似乎一次读128个寄存器没有值返回.
    if(GetRegisterData(regStartAddr, 50, &allChickenHouseSetting[0])){
        
    }
    
    regStartAddr = regStartAddr + 50;
    
    if(GetRegisterData(regStartAddr, 50, &allChickenHouseSetting[50])){
        
    }    
    
    regStartAddr = regStartAddr + 50;
    
    if(GetRegisterData(regStartAddr, 28, &allChickenHouseSetting[100])){
        
    }
}
#endif

#ifdef UNIT_TEST
// 把buffer重新写满, 为了测试读取的完整性.
void fillBufferForTest(void){
    uint16_t i;
    for(i=0; i< RELAY_REG_DATA_BUFFER_LENGTH; i++){
        regDataBuffer[i] = 0x0F;         
    }    
}    
#endif

// 复制三次代码的人是傻逼
void transAndSend(const uint16_t* regAddList, uint16_t regListLength, uint8_t packageNumber, uint8_t chickenHouseNumber){
    
    uint16_t sendingDataRawBytes = regListLength * 2;
    uint16_t i = 0;
    uint16_t faultArraCounter = 0;
    
    // 包1有点儿不一样, 需要加上前面16个byte的鸡舍设置.    
    if(packageNumber == 1){
        sendingDataRawBytes = sendingDataRawBytes + 16;
        
    }

    transDataFromRegDataBufferToTxBuffer(regAddList, regListLength, packageNumber);
    
    if(packageNumber == 4){
        for(i = FAULT_DATA_START_BYTE; i < ((FORTH_PKG_REGS_LENGTH)*2); i++){
            // 将故障数据搬到发送缓存中
            gTxBuffer[i] = faultArr[faultArraCounter];
            
            faultArraCounter++;
        }
    }
    
#if UNIT_TEST    
    if(packageNumber == 4){    
        DBG_LOG("Just for break point.");    
    }
#endif
    
    //将数据跟头尾信息拼在一起发送
    sendPackageByMQTT(sendingDataRawBytes, packageNumber, chickenHouseNumber);

}

// 读取每个鸡舍的相同的寄存器地址.
void readRegs(void){
    uint8_t i = 0;  
    uint16_t bufferWriteIndex = 0;
    // 每个鸡舍一共要读READ_TIMES_PER_CHICKEN_HOUSE次
    for (i = 0; i< READ_TIMES_PER_CHICKEN_HOUSE; i++){
        
        // 首先, 首地址从数组READING_START_REG中获取, 连续读取的长度从READING_LENGTH_ARR数组中读取
        //sendReadingSignal2Relay(READING_START_REG[i], READING_LENGTH_ARR[i], bufferWriteIndex);
        
        if(GetRegisterData(READING_START_REG[i], READING_LENGTH_ARR[i],&regDataBuffer[bufferWriteIndex])){
            //DBG_LOG("Got %d reg data,", READING_LENGTH_ARR[i]);      
        }   
        
        // 将写入的索引号进行自增
        bufferWriteIndex = bufferWriteIndex + READING_LENGTH_ARR[i] * 2;
    }

}

// 从中继控制器读取信寄存器数据
void readDataFromRelay(uint8_t chickenHouseOnlineState){
    uint16_t i = 0;
    // 记录鸡舍号
    uint8_t checkHouseNumber = 0;

    uint16_t regListSize = 0 ;
    
    // 先把数组清零一遍.
    cleanRegDataBuffer();
    
    // 最前面, 可以通过鸡舍1#的绝对地址, 获取每个鸡舍8寄存器(16字节)一次性获取所有鸡舍设置信息
    getAllChickenHouseSetting(chickenHouseOnlineState);
            
    for(i = 0; i< CHICKEN_HOUSE_QUANTITY_MAX; i++){
        // 如果该鸡舍在线
        if(chickenHouseOnlineState >> i && 1){
            
            // 先发送要读取的鸡舍号
            SetChickenNumber(i); 
            
            // 等PLC准备数据
            osDelay(3000);
            
            // 再读
            readRegs();
            
            // 我们的目标: 通俗易懂可复用.
            // i+1 因为鸡舍号逻辑是从1开始
            transAndSend(FIRST_PKG_REGS, FIRST_PKG_REGS_LENGTH, 1, i+1);
            osDelay(3000);
            
            transAndSend(SECOND_PKG_REGS, SECOND_PKG_REGS_LENGTH, 2, i+1);
            osDelay(3000);
            
            transAndSend(THIRD_PKG_REGS, THIRD_PKG_REGS_LENGTH, 3, i+1);
            osDelay(3000);
            
            transAndSend(FORTH_PKG_REGS, FORTH_PKG_REGS_LENGTH, 4, i+1);    
            osDelay(3000);
        }
    }

    
    // 上面, 所有寄存器信息(包括1,2,3,4个数据包的所有需要的数据)都读取完毕.
    
    // transTest(FIRST_PKG_REGS);
    // 测试完成, 注释掉.
        
    // DBG_LOG("readDataFromRelay completed");
    // 废弃不优雅的写法, level up一下.
    // transDataFromRegDataBufferToTxBuffer(1);
    
    // 写满buffer, 测试完整性.
    // fillBufferForTest();
    // 测试完成, 注释掉.

#if 0    
    //获取长度
    regListSize= (sizeof(FIRST_PKG_REGS)) /2;
    transDataFromRegDataBufferToTxBuffer(FIRST_PKG_REGS, regListSize);
    sendingDataRawBytes = regListSize * 2 + 16;
    sendPackageByMQTT(sendingDataRawBytes, 1);

    regListSize= (sizeof(SECOND_PKG_REGS)) /2;
    transDataFromRegDataBufferToTxBuffer(SECOND_PKG_REGS, regListSize);
    sendingDataRawBytes = regListSize * 2 + 16;
    sendPackageByMQTT(sendingDataRawBytes, 2);
#endif   
        
    // 发送第一个包
    //sendPackageByMQTT(regListSize, 2);
    // 准备第二个数据包
    //transDataFromRegDataBufferToTxBuffer(SECOND_PKG_REGS, regListSize);
            
    //sendFirstPackage();
    // 改为发送所有包
    //sendRelayDataPackages();
    
    DBG_LOG("4 package sent..");
   
}

