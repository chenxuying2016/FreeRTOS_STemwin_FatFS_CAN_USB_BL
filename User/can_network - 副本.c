/*
*********************************************************************************************************
*	                                  
*	模块名称 : CAN网络演示程序。
*	文件名称 : can_network.c
*	版    本 : V1.3
*	说    明 : 演示如何实现多个CAN节点（节点的程序相同）之间的通信。
*	修改记录 :
*		版本号  日期       作者      说明
*		v1.3    2015-01-29 Eric2013  首发
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"
#include "can_network.h"

uint8_t FPGA_data[3][8]={0};
uint8_t BL_data[8]={0};
uint8_t PIC_normal_abmormal=0;

/*
*********************************************************************************************************
                                            CAN1引脚配置
由于跳线帽的设置：
	CAN1的TX可以选择引脚PB9或者PA12。
	CAN1的RX可以选择引脚PB8或者PA11。
	从而有下面四种组合：
	PB9_TX      PB8_RX 
	PA12_TX     PB8_RX 
	PA12_TX     PA11_RX 
	PB9_TX      PA11_RX 

本例子是用的PB9_TX   PB8_RX，也就是:
	1. J12跳线帽短路1-2： CAN收发器的RX连接到CPU的PB8引脚。
	2. J13跳线帽短路1-2： CAN收发器的TX连接到CPU的PB9引脚。
********************************************************************************************************
*/
#define CAN1_CLK                    RCC_APB1Periph_CAN1
#define CAN1_RX_PIN                 GPIO_Pin_11
#define CAN1_TX_PIN                 GPIO_Pin_12
#define CAN1_GPIO_TX_PORT           GPIOA
#define CAN1_GPIO_TX_CLK            RCC_AHB1Periph_GPIOA
#define CAN1_GPIO_RX_PORT           GPIOA
#define CAN1_GPIO_RX_CLK            RCC_AHB1Periph_GPIOA
#define CAN1_AF_PORT                GPIO_AF_CAN1
#define CAN1_RX_SOURCE              GPIO_PinSource11
#define CAN1_TX_SOURCE              GPIO_PinSource12

/*
*********************************************************************************************************
                                            CAN2引脚配置
********************************************************************************************************
*/
#define CAN2_CLK                 RCC_APB1Periph_CAN2
#define CAN2_RX_PIN              GPIO_Pin_12
#define CAN2_TX_PIN              GPIO_Pin_13
#define CAN2_GPIO_PORT           GPIOB
#define CAN2_GPIO_CLK            RCC_AHB1Periph_GPIOB
#define CAN2_AF_PORT             GPIO_AF_CAN2
#define CAN2_RX_SOURCE           GPIO_PinSource12
#define CAN2_TX_SOURCE           GPIO_PinSource13


/* 定义全局变量 */
CanTxMsg g_tCanTxMsg;	/* 用于发送 */
CanRxMsg g_tCanRxMsg;	/* 用于接收 */
uint8_t g_ucLedNo = 0;	/* 点亮的LED灯序号，0-3 */

/* 仅允许本文件内调用的函数声明 */
//static void can1_Init(void);
//static void can1_NVIC_Config(void);
//static void can2_Init(void);
//static void can2_NVIC_Config(void);

void can_LedCtrl(void);
void can_BeepCtrl(void);

extern TimerHandle_t xTimers;
extern QueueHandle_t xQueue1;
extern QueueHandle_t xQueue2;

/*
*********************************************************************************************************
*	函 数 名: can_demo
*	功能说明: CAN网络例程
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void can_demo(void)
{	
//	uint8_t ucKeyCode;	   	/* 按键代码 */
	
	can1_Init();			/* 初始化STM32 CAN1硬件 */
	can1_NVIC_Config();		/* 配置CAN1中断 */
	
//	can2_Init();			/* 初始化STM32 CAN2硬件 */
//	can2_NVIC_Config();		/* 配置CAN2中断 */
	
	g_ucLedNo = 0;
	/* 先关闭所有的LED，再点亮其中一个LED */
	bsp_LedOff(1);
	bsp_LedOff(2);
	bsp_LedOff(3);
	bsp_LedOff(4);
	bsp_LedOn(g_ucLedNo + 1);

	while(1)
	{
		/* 处理按键事件 */
//		ucKeyCode = bsp_GetKey();
//		if (ucKeyCode > 0)
//		{
//			/* 有键按下 */
//			switch (ucKeyCode)
//			{
//				case KEY_DOWN_K1:			/* K1键按下 */
//					can_LedCtrl();		
//					break;

//				case KEY_DOWN_K2:			/* K2键按下 */
//					xTimerStart(xTimers, 50);
//					break;

//				case KEY_DOWN_K3:			/* K3键按下 */
//					xTimerStop( xTimers, portMAX_DELAY);
//					break;
//				
//				case JOY_DOWN_OK:			/* K3键按下 */
//					can_BeepCtrl();
//					break;

//				default:
//					/* 其它的键值不处理 */
//					break;
//			}
//		}
		
		vTaskDelay(5);
	}
}

/*
*********************************************************************************************************
*	函 数 名: can_LedCtrl
*	功能说明: 发送一包数据，控制LED
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void can_LedCtrl(void)
{
	/* 每按1次按键，点亮下一个LED，并向CAN网络发送一包数据 */
	if (++g_ucLedNo > 3)
	{
		g_ucLedNo = 0;
	}

	
	/* 填充发送参数 */
	g_tCanTxMsg.StdId = 0x321;
	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_STD;
	
	/* 向CAN网络发送1个字节数据, 指LED灯序号 */
	g_tCanTxMsg.DLC = 1;          /* 每包数据支持0-8个字节，这里设置为发送1个字节 */
    g_tCanTxMsg.Data[0] = g_ucLedNo;
	
    CAN_Transmit(CAN1, &g_tCanTxMsg);	
}

/*
*********************************************************************************************************
*	函 数 名: can_BeepCtrl
*	功能说明: 发送一包数据，控制蜂鸣器
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void can_BeepCtrl(void)
{		
	/* 填充发送参数， 也可以再每次发送的时候填 */
	g_tCanTxMsg.StdId = 0x321;
	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_STD;
	
	g_tCanTxMsg.DLC = 1;
						
	g_tCanTxMsg.Data[1] = 0x01;		/* 01表示鸣叫1次 */
    CAN_Transmit(CAN2, &g_tCanTxMsg);	
}

void CAN_SendMessage(unsigned int ID,unsigned char *array)
{
	g_tCanTxMsg.ExtId=ID;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_EXT;	
	g_tCanTxMsg.DLC = 8;
	g_tCanTxMsg.Data[0]=array[0];
	g_tCanTxMsg.Data[1]=array[1];
	g_tCanTxMsg.Data[2]=array[2];
	g_tCanTxMsg.Data[3]=array[3];
	g_tCanTxMsg.Data[4]=array[4];
	g_tCanTxMsg.Data[5]=array[5];
	g_tCanTxMsg.Data[6]=array[6];
	g_tCanTxMsg.Data[7]=array[7];
  CAN_Transmit(CAN1, &g_tCanTxMsg);	
}

/*
*********************************************************************************************************
*	函 数 名: can1_Init
*	功能说明: 配置CAN硬件,设置波特率为1Mbps
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
 void can1_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;

	/* CAN GPIOs 配置*/
	/* 使能GPIO时钟 */
	RCC_AHB1PeriphClockCmd(CAN1_GPIO_TX_CLK|CAN1_GPIO_RX_CLK, ENABLE);

	/* 引脚映射为CAN功能  */
	GPIO_PinAFConfig(CAN1_GPIO_RX_PORT, CAN1_RX_SOURCE, CAN1_AF_PORT);
	GPIO_PinAFConfig(CAN1_GPIO_TX_PORT, CAN1_TX_SOURCE, CAN1_AF_PORT); 

	/* 配置 CAN RX 和 TX 引脚 */
	GPIO_InitStructure.GPIO_Pin = CAN1_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(CAN1_GPIO_RX_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = CAN1_TX_PIN;
	GPIO_Init(CAN1_GPIO_TX_PORT, &GPIO_InitStructure);

	/* CAN配置*/  
	/* 使能CAN时钟 */
	RCC_APB1PeriphClockCmd(CAN1_CLK, ENABLE);
	/* 复位CAN寄存器 */
	CAN_DeInit(CAN1);
	
	/*
		TTCM = time triggered communication mode
		ABOM = automatic bus-off management 
		AWUM = automatic wake-up mode
		NART = no automatic retransmission
		RFLM = receive FIFO locked mode 
		TXFP = transmit FIFO priority		
	*/
	CAN_InitStructure.CAN_TTCM = DISABLE;			/* 禁止时间触发模式（不生成时间戳), T  */
	CAN_InitStructure.CAN_ABOM = DISABLE;			/* 禁止自动总线关闭管理 */
	CAN_InitStructure.CAN_AWUM = DISABLE;			/* 禁止自动唤醒模式 */
	CAN_InitStructure.CAN_NART = DISABLE;			/* 禁止仲裁丢失或出错后的自动重传功能 */
	CAN_InitStructure.CAN_RFLM = DISABLE;			/* 禁止接收FIFO加锁模式 */
	CAN_InitStructure.CAN_TXFP = DISABLE;			/* 禁止传输FIFO优先级 */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	/* 设置CAN为正常工作模式 */
	
	/* 
		CAN 波特率 = RCC_APB1Periph_CAN1 / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		本例中，设置CAN波特率为1 Mbps		
		CAN 波特率 = 420000000 / 2 / (1 + 12 + 8) / = 1 Mbps		
	*/
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	CAN_InitStructure.CAN_Prescaler = 4;
	CAN_Init(CAN1, &CAN_InitStructure);
	
	/* 设置CAN 筛选器0 */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;		/*  筛选器序号，0-13，共14个滤波器 */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;		/* 筛选器模式，设置ID掩码模式 */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	/* 32位筛选器 */
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;					/* 掩码后ID的高16bit */
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;					/* 掩码后ID的低16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;				/* ID掩码值高16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;				/* ID掩码值低16bit */
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;		/* 筛选器绑定FIFO 0 */
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;				/* 使能筛选器 */
	CAN_FilterInit(&CAN_FilterInitStructure);

	/* 填充发送参数， 也可以再每次发送的时候填 */
//	g_tCanTxMsg.StdId = 0x321;
//	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
//	g_tCanTxMsg.IDE = CAN_ID_STD;
	g_tCanTxMsg.IDE = CAN_ID_EXT;
	g_tCanTxMsg.DLC = 8;	
}    

/*
*********************************************************************************************************
*	函 数 名: can2_Init
*	功能说明: 配置CAN硬件
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void can2_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	CAN_InitTypeDef        CAN_InitStructure;
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;

	/* CAN GPIOs 配置*/
	/* 使能GPIO时钟 */
	RCC_AHB1PeriphClockCmd(CAN2_GPIO_CLK, ENABLE);

	/* 引脚映射为CAN功能  */
	GPIO_PinAFConfig(CAN2_GPIO_PORT, CAN2_RX_SOURCE, CAN2_AF_PORT);
	GPIO_PinAFConfig(CAN2_GPIO_PORT, CAN2_TX_SOURCE, CAN2_AF_PORT); 

	/* 配置 CAN RX 和 TX 引脚 */
	GPIO_InitStructure.GPIO_Pin = CAN2_RX_PIN|CAN2_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(CAN2_GPIO_PORT, &GPIO_InitStructure);

	/* CAN配置*/  
	/* 使能CAN时钟, 使用CAN2必须打开CAN1的时钟 */
	RCC_APB1PeriphClockCmd(CAN2_CLK, ENABLE);

	/* 复位CAN寄存器 */
	CAN_DeInit(CAN2);
	
	/*
		TTCM = time triggered communication mode
		ABOM = automatic bus-off management 
		AWUM = automatic wake-up mode
		NART = no automatic retransmission
		RFLM = receive FIFO locked mode 
		TXFP = transmit FIFO priority		
	*/
	CAN_InitStructure.CAN_TTCM = DISABLE;			/* 禁止时间触发模式（不生成时间戳), T  */
	CAN_InitStructure.CAN_ABOM = DISABLE;			/* 禁止自动总线关闭管理 */
	CAN_InitStructure.CAN_AWUM = DISABLE;			/* 禁止自动唤醒模式 */
	CAN_InitStructure.CAN_NART = DISABLE;			/* 禁止仲裁丢失或出错后的自动重传功能 */
	CAN_InitStructure.CAN_RFLM = DISABLE;			/* 禁止接收FIFO加锁模式 */
	CAN_InitStructure.CAN_TXFP = DISABLE;			/* 禁止传输FIFO优先级 */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	/* 设置CAN为正常工作模式 */
	
	/* 
		CAN 波特率 = RCC_APB1Periph_CAN1 / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		本例中，设置CAN波特率为1 Mbps		
		CAN 波特率 = 420000000 / 2 / (1 + 12 + 8) / = 1 Mbps		
	*/
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	CAN_InitStructure.CAN_Prescaler = 2;
	CAN_Init(CAN2, &CAN_InitStructure);
	
	/* 设置CAN筛选器序号14 ，CAN2筛选器序号 14--27，而CAN1的是0--13*/
	CAN_FilterInitStructure.CAN_FilterNumber = 14;						/* 筛选器序号，14-27，共14个滤波器 */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;		/* 筛选器模式，设置ID掩码模式 */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	/* 32位筛选器 */
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;					/* 掩码后ID的高16bit */
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;					/* 掩码后ID的低16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;				/* ID掩码值高16bit */
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;				/* ID掩码值低16bit */
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO1;		/*  筛选器绑定FIFO 1 */
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;				/* 使能筛选器 */
	CAN_FilterInit(&CAN_FilterInitStructure);

	/* 填充发送参数， 也可以再每次发送的时候填 */
//	g_tCanTxMsg.StdId = 0x321;
//	g_tCanTxMsg.ExtId = 0x01;
	g_tCanTxMsg.RTR = CAN_RTR_DATA;
	g_tCanTxMsg.IDE = CAN_ID_EXT;
	g_tCanTxMsg.DLC = 8;
} 

/*
*********************************************************************************************************
*	函 数 名: can1_NVIC_Config
*	功能说明: 配置CAN中断
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/  
void can1_NVIC_Config(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* CAN FIFO0 消息接收中断使能 */ 
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);	
}

/*
*********************************************************************************************************
*	函 数 名: can2_NVIC_Config
*	功能说明: 配置CAN中断
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/  
void can2_NVIC_Config(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* CAN FIFO1 消息接收中断使能 */ 
	CAN_ITConfig(CAN2, CAN_IT_FMP1, ENABLE);	
}

/*
*********************************************************************************************************
*	函 数 名: CAN1_RX0_IRQHandler
*	功能说明: CAN中断服务程序.
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/ 
void CAN1_RX0_IRQHandler(void)
{
	u8 i;
	
	CAN_Receive(CAN1, CAN_FIFO0, &g_tCanRxMsg);
	
	switch (g_tCanRxMsg.ExtId)
	{
		
		case 0x000203ff:
			PIC_normal_abmormal=g_tCanRxMsg.Data[0];
			break;
		
		case 0x00020101:
			for(i=0;i<8;i++)
			FPGA_data[0][i]=g_tCanRxMsg.Data[i];
			break;
		case 0x00020102:	
			for(i=0;i<8;i++)
			FPGA_data[1][i]=g_tCanRxMsg.Data[i];
			break;
		case 0x00020103:
			for(i=0;i<8;i++)
			FPGA_data[2][i]=g_tCanRxMsg.Data[i];
			break;
		
		case 0x00010000:	
			for(i=0;i<8;i++)
			BL_data[i]=g_tCanRxMsg.Data[i];
			break;
		
		case 0x00020200:
//			FPGA_init_number=((uint16_t)(g_tCanRxMsg.Data[0])<<8)|g_tCanRxMsg.Data[1];
//			if(FPGA_init_number!=0x35f)
//			repeat_send=1;	
			break;
		default:
			break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: CAN2_RX1_IRQHandler
*	功能说明: CAN中断服务程序.
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/ 
void CAN2_RX1_IRQHandler(void)
{	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	CAN_Receive(CAN2, CAN_FIFO1, &g_tCanRxMsg);
	
	if ((g_tCanRxMsg.StdId == 0x321) && (g_tCanRxMsg.IDE == CAN_ID_STD) && (g_tCanRxMsg.DLC == 1))
	{
		xQueueSendFromISR(xQueue2,
						  (void *)&(g_tCanRxMsg.Data[0]),
						   &xHigherPriorityTaskWoken);
					  
		/* 如果xHigherPriorityTaskWoken = pdTRUE，那么退出中断后切到当前最高优先级任务执行 */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}	
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
