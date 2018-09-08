/*
*********************************************************************************************************
*
*	模块名称 : 定时器时基
*	文件名称 : SysInfoTest.c
*	版    本 : V1.0
*	说    明 : 为了获取FreeRTOS的任务信息，需要创建一个定时器，这个定时器的时间基准精度要高于
*              系统时钟节拍。这样得到的任务信息才准确。
*              本文件提供的函数仅用于测试目的，切不可将其用于实际项目，原因有两点：
*               1. FreeRTOS的系统内核没有对总的计数时间做溢出保护。
*               2. 定时器中断是50us进入一次，比较影响系统性能。
*              --------------------------------------------------------------------------------------
*              本文件使用的是32位变量来保存50us一次的计数值，最大支持计数时间：
*              2^32 * 50us / 3600s = 59.6分钟。使用中测试的任务运行计数和任务占用率超过了59.6分钟将不准确。
*
*	修改记录 :
*		版本号    日期        作者     说明
*		V1.0    2015-08-19  Eric2013   首发
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "includes.h"
#include "MainTask.h"
#include "SysInfoTest.h"

uint8_t data[1024];
FRESULT result;
FIL file;
FIL MyFiles;
FIL FileSave;   //专门用于图片的保存
FILINFO finfo;
DIR DirInf;
DIR DirInf_usb;
UINT bw;
FATFS fs;
FATFS fs_nand;
FATFS fs_usb;

/* 定时器频率，50us一次中断 */
#define  timerINTERRUPT_FREQUENCY	20000

/* 中断优先级 */
#define  timerHIGHEST_PRIORITY		2

/* 被系统调用 */
volatile uint32_t ulHighFrequencyTimerTicks = 0UL;
struct roundbutton_struct ROUND_BUTTON;
/*
*********************************************************************************************************
*	函 数 名: vSetupTimerTest
*	功能说明: 创建定时器
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void vSetupSysInfoTest(void)
{
	bsp_SetTIMforInt(TIM6, timerINTERRUPT_FREQUENCY, timerHIGHEST_PRIORITY, 0);
}

/*
*********************************************************************************************************
*	函 数 名: TIM6_DAC_IRQHandler
*	功能说明: TIM6中断服务程序。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void TIM6_DAC_IRQHandler( void )
{
	if(TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
	{
		ulHighFrequencyTimerTicks++;
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	}
}

/*
*********************************************************************************************************
*	函 数 名: EXTI0_Config
*	功能说明: 配置外部中断。K1、K2、K3 中断.
*				K1键 PC13 ：下降沿触发
*				K2键 PA0  : 上升沿触发
*				K3键 PG8  : 下降沿触发
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void EXTI_Config(void)
{
	/* 配置PB7 */
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		EXTI_InitTypeDef   EXTI_InitStructure;
		NVIC_InitTypeDef   NVIC_InitStructure;		
		
		/* 使能 GPIO 时钟 */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		/* Enable SYSCFG clock */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
		
		/* Configure PI8 pin as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		 /* Connect EXTI Line8 to PI8 pin */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource7);

		/* Configure EXTI8 line */
		EXTI_InitStructure.EXTI_Line = EXTI_Line7;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  	/*下降沿触发 */
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);

		/* Enable and set EXTI8 Interrupt to the lowest priority */
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0f;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
}

/*
*********************************************************************************************************
*	函 数 名: EXTI9_5_IRQHandler
*	功能说明: 外部中断服务程序
*	形    参：无
*	返 回 值: 无 40013C00
*********************************************************************************************************
*/
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		EXIT_CLOSE;
		ROUND_BUTTON.Valid_flag = 0x01;
		ROUND_BUTTON.status =  (GPIOB->IDR & (1 << 8))?LEFT:RIGHT;
		EXTI_ClearITPendingBit(EXTI_Line7);
	}
	
}
//void EXTI9_5_IRQHandler(void)
//{
//	if (EXTI_GetITStatus(EXTI_Line7) != RESET)
//	{
////		((union int_2_bit *)40013C00)->temp_bit.exit_en = 0 ;
//		*((unsigned int *)(0013C00))=0x00;
//		EXTI_ClearITPendingBit(EXTI_Line7);
//	}
//}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
