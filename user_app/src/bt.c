/* Includes ------------------------------------------------------------------*/
#include "user_comm.h"

static uint32_t BT_Opt;
static osMessageQId BT_SendQId;

static uint8_t BT_Pipe = 0;
void BT_Task(void* argument);
static void BTCmdHanlde();
void InquireTask(void* argument);
static void bt_Console(int argc, char* argv[]);
int16_t BT_Console(uint8_t* data, uint16_t len);
int16_t BT_SocketSendData(uint8_t* data, uint16_t len);

TaskHandle_t BTtaskHandle;
/**
 * BT初始化
 */
void bt_Init() {
    
    osMessageQDef(BT_SendQ, BT_SEND_Q_SIZE, void*);
    BT_SendQId = osMessageCreate(osMessageQ(BT_SendQ), NULL);

    //osThreadDef(BT, BT_Task, BT_TASK_PRIO, 0, BT_TASK_STK_SIZE);
    //osThreadCreate(osThread(LAN), NULL);
    //if(xTaskCreate(&InquireTask, "InquireTask", 1024, NULL, 3, &Inquiretask) != pdPASS)    
    if(xTaskCreate(&BT_Task, "BT_Task", BT_TASK_STK_SIZE, NULL, osPriorityHigh, &BTtaskHandle) != pdPASS)
        DBG_LOG("Create BT_Task failure");
    else
        DBG_LOG("Create BT_Task ok");
        

#if BT_CMD_EN > 0
    BT_Pipe = CMD_Pipe_Register((CMD_SendFun)BT_SocketSendData);
    DBG_LOG("BT CMD pipe is %d.", BT_Pipe);
#endif

    CMD_ENT_DEF(bt, bt_Console);
    Cmd_AddEntrance(CMD_ENT(bt));

    DBG_LOG("BT Init.");
}

void BT_Task(void* argument)
{

    TWDT_DEF(BTTask, 60000);
    TWDT_ADD(BTTask);
    TWDT_CLEAR(BTTask);
    uint8_t testCounter = 0;
    uint32_t btcmdtick = 0;
    //BTtaskHandle = xQueueCreate(50, sizeof(CmdTYPE_t));
        
    while (1) {
        osDelay(1);
        TWDT_CLEAR(BTTask);      
        if(TS_VOERFLOW(btcmdtick, 1000)) 
        {
            btcmdtick = HAL_GetTick();
            //BTCmdHanlde();
            
            testCounter ++;
        }
        if(testCounter > 10){
            DBG_LOG("bt task running");
            testCounter = 0;
        }
    }
    
}

static void BTCmdHanlde(void)
{

}

int16_t BT_SocketSendData(uint8_t* data, uint16_t len)
{
    return 0;
}

static void bt_Console(int argc, char* argv[])
{

}

