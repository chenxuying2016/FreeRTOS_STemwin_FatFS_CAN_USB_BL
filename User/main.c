/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 : 本实验主要实现FreeRTOS+STemWin+FatFS+USB Host综合
*              实验目的：
*                1. 学习FreeRTOS+STemWin+FatFS+USB Host综合 
*                2. 这里的USB Host主要实现U盘相关处理，支持U盘热插拔。
*                   用户可以根据需要在usb_usr.c文件中的插入检测函数：
*                   USBH_USR_Configuration_DescAvailable或者函数USBH_USR_Init函数加入插入标志
*                   拔出检测函数：
*                   USBH_USR_DeviceDisconnected
*              实验内容：
*                1. 按下按键K1可以通过串口打印任务执行情况（波特率115200，数据位8，奇偶校验位无，停止位1）
*                   =================================================
*                   任务名      任务状态 优先级   剩余栈 任务序号
*                   vTaskUserIF     R       2       251     2
*                   vTaskGUI        R       1       714     1
*                   IDLE            R       0       103     6
*                   vTaskStart      B       5       482     5
*                   vTaskLED        B       3       481     3
*                   vTaskMsgPro     B       4       1937    4
*                   
*                   
*                   任务名       运行计数         使用率
*                   vTaskUserIF     1565            <1%
*                   vTaskGUI        32567           3%
*                   IDLE            966975          94%
*                   vTaskStart      10286           1%
*                   vTaskLED        0               <1%
*                   vTaskMsgPro     16608           1%
*                  串口软件建议使用SecureCRT（V6光盘里面有此软件）查看打印信息。
*                  各个任务实现的功能如下：
*                   vTaskGUI        任务: emWin任务
*                   vTaskTaskUserIF 任务: 接口消息处理	
*                   vTaskLED        任务: LED闪烁
*                   vTaskMsgPro     任务: U盘中文件处理和浏览
*                   vTaskStart      任务: 启动任务，也就是最高优先级任务，这里实现按键扫描和触摸检测
*                2. 任务运行状态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		( 'B' )  阻塞
*                    #define tskREADY_CHAR		    ( 'R' )  就绪
*                    #define tskDELETED_CHAR		( 'D' )  删除
*                    #define tskSUSPENDED_CHAR	    ( 'S' )  挂起
*                3. 本实验的USB Host主要是对U盘的操作，通过电脑端的串口软件SecureCRT软件，
*                   给板子发送相关命令实现操作，具体实现在demo_fatfs文件里面。
*                   	printf("请选择操作命令:\r\n");
*                   	printf("1 - 显示根目录下的文件列表\r\n");
*                   	printf("2 - 创建一个新文件armfly.txt\r\n");
*                   	printf("3 - 读armfly.txt文件的内容\r\n");
*                   	printf("4 - 创建目录\r\n");
*                   	printf("5 - 删除文件和目录\r\n");
*                   	printf("6 - 读写文件速度测试\r\n");
*                   	printf("7 - 挂载U盘\r\n");
*                   	printf("8 - 卸载U盘\r\n");
*              注意事项：
*                1. 本实验推荐使用串口软件SecureCRT，要不串口打印效果不整齐。此软件在
*                   V6开发板光盘里面有。
*                2. 务必将编辑器的缩进参数和TAB设置为4来阅读本文件，要不代码显示不整齐。
*
*	修改记录 :
*		版本号    日期         作者            说明
*       V1.0    2016-03-15   Eric2013    1. ST固件库到V1.6.1版本
*                                        2. BSP驱动包V1.2
*                                        3. FreeRTOS版本V8.2.3
*                                        4. STemWin版本V5.28
*                                        5. FatFS版本V0.11a
*                                        6. USB库版本V2.2.0
*
*
*	Copyright (C), 2016-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"
#include "MainTask.h"
#include "SysInfoTest.h"

/*
**********************************************************************************************************
											函数声明
**********************************************************************************************************
*/
static void vTaskGUI(void *pvParameters);
static void vTaskTaskUserIF(void *pvParameters);
//static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void vTaskCom(void *pvParameters);
static void AppTaskCreate (void);
static void AppObjCreate (void);
//static void App_Printf(char *format, ...);
//static void vTimerCallback(xTimerHandle pxTimer);
//static void sendToFPGA(char picNum);
static void picUp(BaseType_t res,uint32_t val);
static void picDown(BaseType_t res,uint32_t val);
/*
**********************************************************************************************************
											变量声明
**********************************************************************************************************
*/
TaskHandle_t xHandleTaskGUI = NULL;
TaskHandle_t xHandleTaskUserIF = NULL;
//static TaskHandle_t xHandleTaskLED = NULL;
TaskHandle_t xHandleTaskMsgPro = NULL;
TaskHandle_t xHandleTaskUSBPro = NULL;
TaskHandle_t xHandleTaskStart = NULL;
TaskHandle_t xHandleTaskCom = NULL;
//static SemaphoreHandle_t  xMutex = NULL;
//TimerHandle_t xTimers= NULL;
//QueueHandle_t xQueue1 = NULL;
//QueueHandle_t xQueue2 = NULL;
//QueueHandle_t xQueue3 = NULL;

static char project_number;
//static char picture_num;
static uint8_t cnt_state = 0;
char flag_bl = 0;

extern CanRxMsg g_tCanRxMsg;	/* 用于接收 */
//extern char DemoFatFS(void);

extern void can_LedCtrl(void);
extern void can_demo(void);

extern WM_HWIN  hWinMain;
extern WM_HWIN  hWinSet;

extern CONFIG_PARA cfg_para;
extern char file_name[22];
extern CONFIG_PIC  picArray;
extern char cfgname[40][16];
extern uint8_t BL_data[8];
extern struct roundbutton_struct ROUND_BUTTON;
extern char  cfg_flag;

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

int main(void)
{
	/* 
	  在启动调度前，为了防止初始化STM32外设时有中断服务程序执行，这里禁止全局中断(除了NMI和HardFault)。
	  这样做的好处是：
	  1. 防止执行的中断服务程序中有FreeRTOS的API函数。
	  2. 保证系统正常启动，不受别的中断影响。
	  3. 关于是否关闭全局中断，大家根据自己的实际情况设置即可。
	  在移植文件port.c中的函数prvStartFirstTask中会重新开启全局中断。通过指令cpsie i开启，__set_PRIMASK(1)
	  和cpsie i是等效的。
     */

	 __set_PRIMASK(1);
	
//	delay_ms(1000);
	
	/* 硬件初始化 */
	bsp_Init(); 
	
	/* 1. 初始化一个定时器中断，精度高于滴答定时器中断，这样才可以获得准确的系统信息 仅供调试目的，实际项
		  目中不要使用，因为这个功能比较影响系统实时性。
	   2. 为了正确获取FreeRTOS的调试信息，可以考虑将上面的关闭中断指令__set_PRIMASK(1); 注释掉。 
	*/
//	vSetupSysInfoTest();
	EXTI_Config();
  
	/* 创建任务 */
	AppTaskCreate();

	/* 创建任务通信机制 */
	AppObjCreate();
	
  /* 启动调度，开始执行任务 */
  vTaskStartScheduler();

	/* 
	  如果系统正常启动是不会运行到这里的，运行到这里极有可能是用于定时器任务或者空闲任务的
	  heap空间不足造成创建失败，此要加大FreeRTOSConfig.h文件中定义的heap大小：
	  #define configTOTAL_HEAP_SIZE	      ( ( size_t ) ( 17 * 1024 ) )
	*/
	while(1);
}

/*
*********************************************************************************************************
*	函 数 名: vTaskGUI
*	功能说明: emWin任务
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 1   (数值越小优先级越低，这个跟uCOS相反)
*********************************************************************************************************
*/
static void vTaskGUI(void *pvParameters)
{
	while (1) 
	{
		MainTask();
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskTaskUserIF
*	功能说明: 按键消息处理
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 2 
*********************************************************************************************************
*/
static void vTaskTaskUserIF(void *pvParameters)
{
	uint8_t ucKeyCode;
//	uint8_t pcWriteBuffer[500];
	unsigned char sendbuf[8]={0};
	BaseType_t xResult;
	uint32_t ulValue;
	char cnt = 0;
	char swFlag = 0;
	
    while(1)
    {
			xResult = xTaskNotifyWait(0x00000000,0xFFFFFFFF,&ulValue,xMaxBlockTime);
			if( xResult == pdPASS )
			{
				if((ulValue & BIT_21) != 0)
					swFlag = 1;
			}			
			switch(cnt_state)
			{
				case 1:
					cnt_state = 0;
					GUI_SendKeyMsg(GUI_KEY_UP, 1);
					if(swFlag)
					{
						swFlag = 0;
						picUp(xResult,ulValue);
					}
					break;
				case 2:
					cnt_state = 0;
					GUI_SendKeyMsg(GUI_KEY_DOWN, 1);
					if(swFlag)
					{
						swFlag = 0;
						picDown(xResult,ulValue);
					}
					break;
			}
		ucKeyCode = bsp_GetKey();
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:
          GUI_SendKeyMsg(GUI_KEY_TAB, 1);
					GUI_SendKeyMsg(GUI_KEY_TAB, 0);
					break;
				
				case KEY_2_DOWN:
					GUI_SendKeyMsg(GUI_KEY_SPACE, 1);
					break;
				
				case KEY_3_DOWN:
					GUI_SendKeyMsg(GUI_KEY_LEFT, 1);
					if(swFlag)
					{
						swFlag = 0;
						picUp(xResult,ulValue);
					}
					break;
				
				case KEY_DOWN_K4:		
					GUI_SendKeyMsg(GUI_KEY_RIGHT, 1);
					if(swFlag)
					{
						swFlag = 0;
						picDown(xResult,ulValue);
					}
					break;
				
				case KEY_DOWN_K5:
					if(swFlag)
					{
						swFlag = 0;
						
						memset(sendbuf,0,8);
						CAN_SendMessage(0x02030000,sendbuf);//enable play
		
						CAN_SendMessage(0x01030000,sendbuf);//enable backlight out
						memset(BL_data,0,8);
						flag_bl = 0;
					}
				
					GUI_SendKeyMsg(GUI_KEY_BACKTAB, 1);
					GUI_SendKeyMsg(GUI_KEY_BACKTAB, 0);
					break;
				
				case KEY_DOWN_K6:
					GUI_SendKeyMsg(GUI_KEY_ENTER, 1);
					GUI_SendKeyMsg(GUI_KEY_ENTER, 0);
					break;
				
				case KEY_DOWN_K7:
					if(swFlag)
					{
						swFlag = 0;
						picUp(xResult,ulValue);
					}
					break;
				
				case KEY_DOWN_K8:
					if(swFlag)
					{
						swFlag = 0;
						picDown(xResult,ulValue);
					}
					break;
				
				/* 其他的键值不处理 */
				default:
					break;
			}
		}
		
		if((cnt++>100) && flag_bl)
		{
			cnt =0 ;
			CAN_SendMessage(0x01020000,0);
		}
		
		vTaskDelay(20);
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskMsgPro
*	功能说明: 文件处理和浏览
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 3  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{			
//	FRESULT result;
//	FATFS fs;
//	FIL fileU;
//	DIR DirInf;
//	uint32_t bw;
//	char bufN[8];
//	char path[32];
	char i;
	char FileNamebuf[16]={0};
	BaseType_t xResult;
	uint32_t ulValue;
	char picture_num;
//	char page=0;
	char len=1;
	uint16_t setVol,setCur;
//	int setTime;
	
	while(1)
	{
//			DemoFatFS();
		xResult = xTaskNotifyWait(0x00000000,0xFFFFFFFF,&ulValue,xMaxBlockTime);
		
		if( xResult == pdPASS )
		{
			if((ulValue & BIT_16) != 0)
				project_number = ulValue & 0xffff;
			if((ulValue & BIT_17) != 0)
				picture_num = ulValue & 0xffff;
			if((ulValue & BIT_18) != 0)
			{
				memset(&cfg_para,0,sizeof(cfg_para));
				ReadFileData(FS_VOLUME_SD,file_name,&cfg_para,sizeof(cfg_para));
			}
			if((ulValue & BIT_19) != 0)
			{
				for(i=0;i<len;i++)
				{
					memset(&cfg_para,0,sizeof(cfg_para));
					sprintf(FileNamebuf, "%s%02d",cfgname[project_number], i);
					ReadFileData(FS_VOLUME_SD,FileNamebuf,&cfg_para,sizeof(cfg_para));
					sendToFPGA(i);
					len = cfg_para.picMaxN;
//					vTaskDelay(20);
				}
				flag_bl = 1;
//				GUI_Delay(300);
//				WM_SendMessageNoPara(hWinMain, MSG_DeleteInfo);
			}
			if((ulValue & BIT_20) != 0)
			{
				CreateNewFile(project_number,picture_num,0);
			}
			if((ulValue & BIT_22) != 0)
			{	
				vTaskPrioritySet(NULL, 7);
				for(i=0;i<len;i++)
				{
					memset(&cfg_para,0,sizeof(cfg_para));
					sprintf(FileNamebuf, "%s%02d",cfgname[project_number], i);
					ReadFileData(FS_VOLUME_SD,FileNamebuf,&cfg_para,sizeof(cfg_para));
					len = cfg_para.picMaxN;				
					WriteCfg2U(&cfg_para,i,cfgname[project_number]);
//					vTaskDelay(20);
				}
				vTaskPrioritySet(NULL, 3);
			}
			if((ulValue & BIT_24) != 0)
			{
				setVol = cfg_para.voltage;
				setCur = cfg_para.current;
//				setTime = cfg_para.totalTime;
				
				for(i=0;i<cfg_para.picMaxN;i++)
				{	
					if(i!=picture_num)
					{
						memset(&cfg_para,0,sizeof(cfg_para));
						sprintf(FileNamebuf, "%s%02d",cfgname[project_number], i);
						ReadFileData(FS_VOLUME_SD,FileNamebuf,&cfg_para,sizeof(cfg_para));
						
						cfg_para.voltage = setVol;
						cfg_para.current = setCur;
//						cfg_para.totalTime = setTime;

						CreateNewFile(project_number,i,0);
					}
				}
				memset(&cfg_para,0,sizeof(cfg_para));
				sprintf(FileNamebuf, "%s%02d",cfgname[project_number], picture_num);
				ReadFileData(FS_VOLUME_SD,FileNamebuf,&cfg_para,sizeof(cfg_para));
			}
		}
	
		vTaskDelay(20);	
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskStart
*	功能说明: 启动任务，也就是最高优先级任务。主要实现按键检测和触摸检测。
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 4  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{	
	uint16_t count=0;
	uint8_t flag=0;
	char cntLed = 0;
	
	while(1)
	{
		bsp_KeyScan();
		
		if(ROUND_BUTTON.Valid_flag)
		{
			flag = 1;
			ROUND_BUTTON.Valid_flag = 0x00;
			if(ROUND_BUTTON.status == LEFT)
			{				
				cnt_state = 1;
			}
			else{		
				cnt_state = 2;
			}
		}
		if(flag)
		{
			if(++count > 20)
			{
				flag = 0;
				EXIT_OPEN;
				count = 0;
			}
		}
		if(cntLed++ > 50)
		{
			cntLed = 0;
			bsp_LedToggle(1);
		}
			
		vTaskDelay(10);	
	}
}


static void vTaskCom(void *pvParameters)
{
	while(1)
	{
		DemoCOM();
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskUSBPro
*	功能说明: U盘监测及初始化
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 5  
*********************************************************************************************************
*/
static void vTaskUSBPro(void *pvParameters)
{
//		usbh_OpenMassStorage();
	
    while(1)
    {
			if(HCD_IsDeviceConnected(&USB_OTG_Core))
			{
				USBH_Process(&USB_OTG_Core, &USB_Host);
			}	
			
			vTaskDelay(10);	
    }
}
				
/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{	
	xTaskCreate( vTaskGUI,            /* 任务函数  */
							 "vTaskGUI",          /* 任务名    */
							 2048,                /* 任务栈大小，单位word，也就是4字节 */
							 NULL,                /* 任务参数  */
							 1,                   /* 任务优先级*/
							 &xHandleTaskGUI );		/* 任务句柄  */
	
	xTaskCreate( vTaskTaskUserIF,   	/* 任务函数  */
							 "vTaskUserIF",     	/* 任务名    */
							 512,               	/* 任务栈大小，单位word，也就是4字节 */
							 NULL,              	/* 任务参数  */
							 2,                 	/* 任务优先级*/
							 &xHandleTaskUserIF );  /* 任务句柄  */
	
	xTaskCreate( vTaskMsgPro,     		/* 任务函数  */
							 "vTaskMsgPro",   		/* 任务名    */
							 1024,             		/* 任务栈大小，单位word，也就是4字节 2048*/
							 NULL,           			/* 任务参数  */
							 3,               		/* 任务优先级*/
							 &xHandleTaskMsgPro );  /* 任务句柄  */	
	
	xTaskCreate( vTaskStart,     		/* 任务函数  */
							 "vTaskStart",   		/* 任务名    */
							 256,            		/* 任务栈大小，单位word，也就是4字节 */
							 NULL,           		/* 任务参数  */
							 4,              		/* 任务优先级*/
							 &xHandleTaskStart );   /* 任务句柄  */	
	
	xTaskCreate( vTaskCom,     		/* 任务函数  */
							 "vTaskCom",   		/* 任务名    */
							 512,            		/* 任务栈大小，单位word，也就是4字节 */
							 NULL,           		/* 任务参数  */
							 5,              		/* 任务优先级*/
							 &xHandleTaskCom );   /* 任务句柄  */
	
	xTaskCreate( vTaskUSBPro,     		/* 任务函数  */
							 "vTaskUSBPro",   		/* 任务名    */
							 256,             		/* 任务栈大小，单位word，也就是4字节 2048*/
							 NULL,           			/* 任务参数  */
							 6,               		/* 任务优先级*/
							 &xHandleTaskUSBPro );  /* 任务句柄  */
}

static void picUp(BaseType_t res,uint32_t val)
{
	int8_t qietu_num=0;
	char namebuf[16]={0};
	unsigned char sendbuf[8]={0};
//	BaseType_t xResult;
//	uint32_t ulValue;
	
	qietu_num=picArray.picNum;		

			qietu_num--;
			if(qietu_num<0)
				qietu_num=picArray.picMaxN-1;
			CAN_SendMessage(0x02030001 | (qietu_num<<8),sendbuf);
			memset(namebuf,0,sizeof(namebuf));
			strncpy(namebuf,cfgname[project_number],strlen(cfgname[project_number]));
			sprintf(file_name, "%s%02d", namebuf,qietu_num);
			ReadFileData(FS_VOLUME_SD,file_name,&picArray,sizeof(CONFIG_PIC));

}

static void picDown(BaseType_t res,uint32_t val)
{
	int8_t qietu_num=0;
	char namebuf[16]={0};
	unsigned char sendbuf[8]={0};
	
	qietu_num=picArray.picNum;

			qietu_num++;
			if(qietu_num>picArray.picMaxN-1)
				qietu_num=0;
			CAN_SendMessage(0x02030001 | (qietu_num<<8),sendbuf);
			memset(namebuf,0,sizeof(namebuf));
			strncpy(namebuf,cfgname[project_number],strlen(cfgname[project_number]));
			sprintf(file_name, "%s%02d", namebuf,qietu_num);
			ReadFileData(FS_VOLUME_SD,file_name,&picArray,sizeof(CONFIG_PIC));
		
}

/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
//	uint8_t i;
//	const TickType_t  xTimerPer[2] = {50, 100};
	
//	for(i = 0; i < 2; i++)
//	{
//		xTimers = xTimerCreate("Timer",          /* 定时器名字 */
//								xTimerPer[i],       /* 定时器周期,单位时钟节拍 */
//								pdTRUE,          /* 周期性 */
//								(void *) i,      /* 定时器ID */
//								vTimerCallback); /* 定时器回调函数 */

//		if(xTimers == NULL)
//		{
//			/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
//		}
//	}
	
//	/* 创建事件标志组 */
//	xCreatedEventGroup = xEventGroupCreate();	
//	if(xCreatedEventGroup == NULL)
//	{
//			/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
//	}
		
	/* 创建10个uint8_t型消息队列 */
//	xQueue1 = xQueueCreate(10, sizeof(uint8_t));
//		if( xQueue1 == 0 )
//		{
//				/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
//		}
//	
//	/* 创建10个uint8_t型消息队列 */
////		xQueue2 = xQueueCreate(10, sizeof(uint8_t *));
//	xQueue2 = xQueueCreate(10, sizeof(g_tCanRxMsg.Data));
//		if( xQueue2 == 0 )
//		{
//				/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
//		}
//	xQueue3 = xQueueCreate(10, sizeof(g_tCanRxMsg.Data));
//	
//	/* 创建互斥信号量 */
//    xMutex = xSemaphoreCreateMutex();
//	
//	if(xMutex == NULL)
//    {
//        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
//    }
}

/*
*********************************************************************************************************
*	函 数 名: vTimerCallback
*	功能说明: 定时器回调函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void vTimerCallback(xTimerHandle pxTimer)
{
//	uint32_t ulTimerID;
//	
//	configASSERT(pxTimer);

//	/* 获取那个定时器时间到 */
//	ulTimerID = (uint32_t)pvTimerGetTimerID(pxTimer);	
//	
//	/* 处理定时器0任务 */
//	if(ulTimerID == 0)
//	{
//		can_LedCtrl();
//	}
//	
//	/* 处理定时器1任务 */
//	if(ulTimerID == 1)
//	{
//		bsp_LedToggle(2);
//	}	
}

/*
*********************************************************************************************************
*	函 数 名: App_Printf
*	功能说明: 线程安全的printf方式		  			  
*	形    参: 同printf的参数。
*             在C中，当无法列出传递函数的所有实参的类型和数目时,可以用省略号指定参数表
*	返 回 值: 无
*********************************************************************************************************
*/
static void  App_Printf(char *format, ...)
{
//    char  buf_str[200 + 1];
//    va_list   v_args;


//    va_start(v_args, format);
//   (void)vsnprintf((char       *)&buf_str[0],
//                   (size_t      ) sizeof(buf_str),
//                   (char const *) format,
//                                  v_args);
//    va_end(v_args);

//	/* 互斥信号量 */
//	xSemaphoreTake(xMutex, portMAX_DELAY);

//    printf("%s", buf_str);

//   	xSemaphoreGive(xMutex);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
