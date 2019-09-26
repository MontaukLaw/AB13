/**
 * **********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file Process.h
 * @author 罗勇
 * @version V0.1
 * @date 2019.9.19
 * @brief 中继PLC协议读取头文件.
 *
 * **********************************************************************
 * @note
 *
 * **********************************************************************
 */

#ifndef _RELAY_COM_H
#define _RELAY_COM_H
     
/* Includes ------------------------------------------------------------------*/     
  
     
// 读取次数
#define READ_TIMES_PER_CHICKEN_HOUSE        26 
#define CHICKEN_HOUSE_QUANTITY_MAX            16
#define CHICKEN_HOUSE_SETTING_START_REG_ADDR  1250     
     
// 每个鸡舍设置 8个寄存器    
#define CHICKEN_HOUSE_SETTING_REG_QUANTITY    8    
     
// 每个鸡舍8个寄存器, 16个byte     
#define CHICKEN_HOUSE_SETTING_ARRAY_LENGTH  (CHICKEN_HOUSE_QUANTITY_MAX * CHICKEN_HOUSE_SETTING_REG_QUANTITY * 2)     
     
// 总共目前数据总量是680个寄存器, 即1360个byte
#define USEFUL_REG_DATA_BUFFER_LENGTH   (680*2)

// 一共要读取的连续寄存器有754个, 即最终数组长度应该是754*2
#define RELAY_REG_DATA_BUFFER_LENGTH   (754*2)

void collectDataFromRegDataBuffer(void);

//void readDataFromRelay(void);
void readDataFromRelay(uint8_t chickenHouseOnlineState);

#endif