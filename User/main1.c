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




/*
**********************************************************************************************************
											函数声明
**********************************************************************************************************
*/
static void vTaskGUI(void *pvParameters);
static void vTaskTaskUserIF(void *pvParameters);
static void vTaskLED(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void AppTaskCreate (void);
static void AppObjCreate (void);
static void App_Printf(char *format, ...);
static void vTimerCallback(xTimerHandle pxTimer);


/*
**********************************************************************************************************
											变量声明
**********************************************************************************************************
*/
static TaskHandle_t xHandleTaskUserIF = NULL;
static TaskHandle_t xHandleTaskLED = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static TaskHandle_t xHandleTaskStart = NULL;
static SemaphoreHandle_t  xMutex = NULL;
TimerHandle_t xTimers= NULL;
QueueHandle_t xQueue1 = NULL;
QueueHandle_t xQueue2 = NULL;

extern void can_LedCtrl(void);
extern void can_demo(void);


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
	
	/* 硬件初始化 */
	bsp_Init(); 
	
	/* 1. 初始化一个定时器中断，精度高于滴答定时器中断，这样才可以获得准确的系统信息 仅供调试目的，实际项
		  目中不要使用，因为这个功能比较影响系统实时性。
	   2. 为了正确获取FreeRTOS的调试信息，可以考虑将上面的关闭中断指令__set_PRIMASK(1); 注释掉。 
	*/
	vSetupSysInfoTest();
	
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
//	uint8_t ucKeyCode;
//	uint8_t pcWriteBuffer[500];
	

    while(1)
    {
//		ucKeyCode = bsp_GetKey();
//		
//		if (ucKeyCode != KEY_NONE)
//		{
//			switch (ucKeyCode)
//			{
//				/* K1键按下 打印任务执行情况 */
//				case KEY_DOWN_K1:			 
//					App_Printf("=================================================\r\n");
//					App_Printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
//					vTaskList((char *)&pcWriteBuffer);
//					App_Printf("%s\r\n", pcWriteBuffer);
//				
//					App_Printf("\r\n任务名       运行计数         使用率\r\n");
//					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
//					App_Printf("%s\r\n", pcWriteBuffer);
//					printf("当前动态内存剩余大小 = %d字节\r\n", xPortGetFreeHeapSize());
//					break;
//				
//				/* 其他的键值不处理 */
//				default:                     
//					break;
//			}
//		}
		can_demo();
		
//		vTaskDelay(20);
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskLED
*	功能说明: LED闪烁
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 3 
*********************************************************************************************************
*/
static void vTaskLED(void *pvParameters)
{
//	TickType_t xLastWakeTime;
//	const TickType_t xFrequency = 200;

//	/* 获取当前的系统时间 */
//    xLastWakeTime = xTaskGetTickCount();
	BaseType_t xResult;
	uint8_t ucQueueMsgValue;
	
    while(1)
    {
		/* 注释掉LED的闪烁，因为LED和SDRAM都使用FMC总线，多任务会有冲突。
		   使用IAR版本，优化等级NONE的情况下，发现显示屏上电后后黑条线。
		   使用MDK，优化等级为0的时候没有这个问题。不过，为了保险还是将LED
		   闪烁注释掉。
		*/
       	//bsp_LedToggle(2);
		//bsp_LedToggle(3);

		/* vTaskDelayUntil是绝对延迟，vTaskDelay是相对延迟。*/
//        vTaskDelayUntil(&xLastWakeTime, xFrequency);
		xResult = xQueueReceive(xQueue1,                   /* 消息队列句柄 */
		                        (void *)&ucQueueMsgValue,  /* 存储接收到的数据到变量ucQueueMsgValue中 */
		                        (TickType_t)portMAX_DELAY);/* 设置阻塞时间 */
		
		if(xResult == pdPASS)
		{
			/* 成功接收，并通过串口将数据打印出来 */
			/* 先关闭所有的LED，再点亮其中一个LED */
			bsp_LedOff(1);
			bsp_LedOff(2);
			bsp_LedOff(3);
			bsp_LedOff(4);					
			bsp_LedOn(ucQueueMsgValue + 1);
		}
    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskMsgPro
*	功能说明: U盘中文件处理和浏览
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 4  
*********************************************************************************************************
*/
extern void DemoFatFS(void);
static void vTaskMsgPro(void *pvParameters)
{
    while(1)
    {
		DemoFatFS();			
    }
	
//	BaseType_t xResult;
//	uint8_t ucQueueMsgValue;
//	
//    while(1)
//    {
//		xResult = xQueueReceive(xQueue2,                   /* 消息队列句柄 */
//		                        (void *)&ucQueueMsgValue,  /* 存储接收到的数据到变量ucQueueMsgValue中 */
//		                        (TickType_t)portMAX_DELAY);/* 设置阻塞时间 */
//		
//		if(xResult == pdPASS)
//		{
//			BEEP_KeyTone();
//		}
//    }
}

/*
*********************************************************************************************************
*	函 数 名: vTaskStart
*	功能说明: 启动任务，也就是最高优先级任务。主要实现按键检测和触摸检测。
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 5  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
	uint8_t  ucCount = 0;
	uint8_t  ucCount1 = 0;
    
	
    while(1)
    {
		BEEP_Pro();	    /* 蜂鸣器处理函数 */
		/* 1ms一次触摸扫描，电阻触摸屏 */
		if(g_tTP.Enable == 1)
		{
			TOUCH_Scan();
			
			/* 按键扫描 */
			ucCount++;
			if(ucCount == 10)
			{
				ucCount = 0;
				bsp_KeyScan();
			}
		    vTaskDelay(1);			
		}
		
		/* 10ms一次触摸扫描，电容触摸屏GT811 */
		if(g_GT811.Enable == 1)
		{
			bsp_KeyScan();
			ucCount1++;
			if(ucCount1 == 2)
			{
				ucCount1 = 0;
				GT811_OnePiontScan();
			}
		    vTaskDelay(10);	
		}
		
		/* 10ms一次触摸扫描，电容触摸屏FT5X06 */
		if(g_tFT5X06.Enable == 1)
		{
			bsp_KeyScan();
			FT5X06_OnePiontScan();
		    vTaskDelay(10);	
		}
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
	xTaskCreate(  vTaskGUI,             /* 任务函数  */
                  "vTaskGUI",           /* 任务名    */
                  1024,                 /* 任务栈大小，单位word，也就是4字节 */
                  NULL,                 /* 任务参数  */
                  1,                    /* 任务优先级*/
                  NULL );               /* 任务句柄  */
	
    xTaskCreate( vTaskTaskUserIF,   	/* 任务函数  */
                 "vTaskUserIF",     	/* 任务名    */
                 512,               	/* 任务栈大小，单位word，也就是4字节 */
                 NULL,              	/* 任务参数  */
                 2,                 	/* 任务优先级*/
                 &xHandleTaskUserIF );  /* 任务句柄  */
	
	
	xTaskCreate( vTaskLED,    		/* 任务函数  */
                 "vTaskLED",  		/* 任务名    */
                 512,         		/* stack大小，单位word，也就是4字节 */
                 NULL,        		/* 任务参数  */
                 3,           		/* 任务优先级*/
                 &xHandleTaskLED ); /* 任务句柄  */
	
	xTaskCreate( vTaskMsgPro,     		/* 任务函数  */
                 "vTaskMsgPro",   		/* 任务名    */
                 2048,             		/* 任务栈大小，单位word，也就是4字节 */
                 NULL,           		/* 任务参数  */
                 4,               		/* 任务优先级*/
                 &xHandleTaskMsgPro );  /* 任务句柄  */
	
	
	xTaskCreate( vTaskStart,     		/* 任务函数  */
                 "vTaskStart",   		/* 任务名    */
                 512,            		/* 任务栈大小，单位word，也就是4字节 */
                 NULL,           		/* 任务参数  */
                 5,              		/* 任务优先级*/
                 &xHandleTaskStart );   /* 任务句柄  */
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
//	const TickType_t  xTimerPer = 50;
//	
//	xTimers = xTimerCreate("Timer",          /* 定时器名字 */
//							xTimerPer,       /* 定时器周期,单位时钟节拍 */
//							pdTRUE,          /* 周期性 */
//							(void *) 0,      /* 定时器ID */
//							vTimerCallback); /* 定时器回调函数 */

//	if(xTimers == NULL)
//	{
//		/* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
//	}
	
	/* 创建10个uint8_t型消息队列 */
	xQueue1 = xQueueCreate(10, sizeof(uint8_t));
    if( xQueue1 == 0 )
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
	
	/* 创建10个uint8_t型消息队列 */
	xQueue2 = xQueueCreate(10, sizeof(uint8_t));
    if( xQueue2 == 0 )
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
	
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
	configASSERT(pxTimer);
	
	can_LedCtrl();
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
