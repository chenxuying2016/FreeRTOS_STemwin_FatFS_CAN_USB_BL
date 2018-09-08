/*
*********************************************************************************************************
*
*	模块名称 : STM32-V6开发板扩展IO驱动程序
*	文件名称 : bsp_ext_io.c
*	版    本 : V1.0
*	说    明 : V6开发板在FMC总线上扩展了32位输出IO。地址为 (0x6820 0000)
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2015-10-11  armfly  正式发布
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	安富莱STM32-V6 开发板扩展口线分配: 总线地址 = 0x6400 0000
	D0  - GPRS_RERM_ON
	D1  - GPRS_RESET
	D2  - NRF24L01_CE
	D3  - NRF905_TX_EN
	D4  - NRF905_TRX_CE/VS1053_XDCS
	D5  - NRF905_PWR_UP
	D6  - ESP8266_G0
	D7  - ESP8266_G2
	
	D8  - LED1
	D9  - LED2
	D10 - LED3
	D11 - LED4
	D12 - TP_NRST
	D13 - AD7606_OS0
	D14 - AD7606_OS1
	D15 - AD7606_OS2
	
	预留的8个5V输出IO: Y50_0 - Y50_1
	D16  - Y50_0
	D17  - Y50_1
	D18  - Y50_2
	D19  - Y50_3
	D20  - Y50_4
	D21  - Y50_5
	D22  - Y50_6
	D23  - Y50_7	

	预留的8个3.3V输出IO: Y33_0 - Y33_1
	D24  - AD7606_RESET
	D25  - AD7606_RAGE
	D26  - Y33_2
	D27  - Y33_3
	D28  - Y33_4
	D29  - Y33_5
	D30  - Y33_6
	D31  - Y33_7				
*/

#define  HC574_PORT	 *(uint32_t *)0x64001000

__IO uint32_t g_HC574;	/* 保存74HC574端口状态 */

static void HC574_ConfigGPIO(void);
static void HC574_ConfigFMC(void);
static void _ConfigLed(void);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitExtIO
*	功能说明: 配置扩展IO相关的GPIO. 上电只能执行一次。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitExtIO(void)
{
	_ConfigLed();
//	HC574_ConfigGPIO();
//	HC574_ConfigFMC();
//	
//	/* 将V6开发板一些片选，LED口设置为高 */
//	g_HC574 = (NRF24L01_CE | VS1053_XDCS | LED1 | LED2 | LED3 | LED4);
//	HC574_PORT = g_HC574;	/* 写硬件端口，更改IO状态 */
}

/*
*********************************************************************************************************
*	函 数 名: HC574_SetPin
*	功能说明: 设置74HC574端口值
*	形    参: _pin : 管脚号， 0-31; 只能选1个，不能多选
*			  _value : 设定的值，0或1
*	返 回 值: 无
*********************************************************************************************************
*/
void HC574_SetPin(uint32_t _pin, uint8_t _value)
{
	if (_value == 0)
	{
		g_HC574 &= (~_pin);
	}
	else
	{
		g_HC574 |= _pin;
	}
	HC574_PORT = g_HC574;
}

/*
*********************************************************************************************************
*	函 数 名: HC574_GetPin
*	功能说明: 判断指定的管脚输出是1还是0
*	形    参: _pin : 管脚号， 0-31; 只能选1个，不能多选
*	返 回 值: 0或1
*********************************************************************************************************
*/
uint8_t HC574_GetPin(uint32_t _pin)
{
	if (g_HC574 & _pin)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

static void _ConfigLed(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 第1步：打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);	

	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: HC574_ConfigGPIO
*	功能说明: 配置GPIO，FMC管脚设置为复用功能
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void HC574_ConfigGPIO(void)
{
/*
	安富莱STM32-V6开发板接线方法：4片74HC574挂在FMC 32位总线上。1个地址端口可以扩展出32个IO
	PD0/FMC_D2
	PD1/FMC_D3
	PD4/FMC_NOE		---- 读控制信号，OE = Output Enable ， N 表示低有效
	PD5/FMC_NWE		-XX- 写控制信号，AD7606 只有读，无写信号
	PD8/FMC_D13
	PD9/FMC_D14
	PD10/FMC_D15
	PD14/FMC_D0
	PD15/FMC_D1

	PE7/FMC_D4
	PE8/FMC_D5
	PE9/FMC_D6
	PE10/FMC_D7
	PE11/FMC_D8
	PE12/FMC_D9
	PE13/FMC_D10
	PE14/FMC_D11
	PE15/FMC_D12
	
	PG0/FMC_A10		--- 和主片选FMC_NE2一起译码
	PG1/FMC_A11		--- 和主片选FMC_NE2一起译码
	PG9/FMC_NE2		--- 主片选（OLED, 74HC574, DM9000, AD7606）	
	
	 +-------------------+------------------+
	 +   32-bits Mode: D31-D16              +
	 +-------------------+------------------+
	 | PH8 <-> FMC_D16   | PI0 <-> FMC_D24  |
	 | PH9 <-> FMC_D17   | PI1 <-> FMC_D25  |
	 | PH10 <-> FMC_D18  | PI2 <-> FMC_D26  |
	 | PH11 <-> FMC_D19  | PI3 <-> FMC_D27  |
	 | PH12 <-> FMC_D20  | PI6 <-> FMC_D28  |
	 | PH13 <-> FMC_D21  | PI7 <-> FMC_D29  |
	 | PH14 <-> FMC_D22  | PI9 <-> FMC_D30  |
	 | PH15 <-> FMC_D23  | PI10 <-> FMC_D31 |
	 +------------------+-------------------+	
*/	

	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能 GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG
			| RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOI, ENABLE);

	/* 使能FMC时钟 */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);

	/* 设置 GPIOD 相关的IO为复用推挽输出 */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);	
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
	                            GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
	                            GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* 设置 GPIOE 相关的IO为复用推挽输出 */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
	                            GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
	                            GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* 设置 GPIOG 相关的IO为复用推挽输出 */
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource0, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource1, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource9, GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_9;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* 设置 GPIOH 相关的IO为复用推挽输出 */
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource8, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource9, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource10, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource11, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource12, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource13, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource14, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource15, GPIO_AF_FMC);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12
						| GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	/* 设置 GPIOI 相关的IO为复用推挽输出 */
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource0, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource1, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource2, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource3, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource6, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource7, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource9, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource10, GPIO_AF_FMC);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6
						| GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: HC574_ConfigFMC
*	功能说明: 配置FMC并口访问时序
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void HC574_ConfigFMC(void)
{
	FMC_NORSRAMInitTypeDef  init;
	FMC_NORSRAMTimingInitTypeDef  timing;

	/*
		AD7606规格书要求(3.3V时)：RD读信号低电平脉冲宽度最短21ns，高电平脉冲最短宽度15ns。

		按照如下配置 读数均正常。为了和同BANK的LCD配置相同，选择3-0-6-1-0-0
		3-0-5-1-0-0  : RD高持续75ns， 低电平持续50ns.  1us以内可读取8路样本数据到内存。
		1-0-1-1-0-0  : RD高75ns，低电平执行12ns左右，下降沿差不多也12ns.  数据读取正确。
	*/
	/* FMC_Bank1_NORSRAM2 configuration */
	timing.FMC_AddressSetupTime = 3;
	timing.FMC_AddressHoldTime = 0;
	timing.FMC_DataSetupTime = 6;
	timing.FMC_BusTurnAroundDuration = 1;
	timing.FMC_CLKDivision = 0;
	timing.FMC_DataLatency = 0;
	timing.FMC_AccessMode = FMC_AccessMode_A;

	/*
	 LCD configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable
	*/
	init.FMC_Bank = FMC_Bank1_NORSRAM2;
	init.FMC_DataAddressMux = FMC_DataAddressMux_Disable;
	init.FMC_MemoryType = FMC_MemoryType_SRAM;
	init.FMC_MemoryDataWidth = FMC_NORSRAM_MemoryDataWidth_32b;	// FMC_NORSRAM_MemoryDataWidth_16b;  FMC_NORSRAM_MemoryDataWidth_32b
	init.FMC_BurstAccessMode = FMC_BurstAccessMode_Disable;
	init.FMC_WaitSignalPolarity = FMC_WaitSignalPolarity_Low;
	init.FMC_WrapMode = FMC_WrapMode_Disable;
	init.FMC_WaitSignalActive = FMC_WaitSignalActive_BeforeWaitState;
	init.FMC_WriteOperation = FMC_WriteOperation_Enable;
	init.FMC_WaitSignal = FMC_WaitSignal_Disable;
	init.FMC_ExtendedMode = FMC_ExtendedMode_Disable;
	init.FMC_AsynchronousWait = FMC_AsynchronousWait_Disable;	
	init.FMC_WriteBurst = FMC_WriteBurst_Disable;
	init.FMC_ContinousClock = FMC_CClock_SyncOnly;	//FMC_CClock_SyncAsync;	// FMC_CClock_SyncOnly;	/* 429比407多的一个参数 */

	init.FMC_ReadWriteTimingStruct = &timing;
	init.FMC_WriteTimingStruct = &timing;

	FMC_NORSRAMInit(&init);

	/* - BANK 1 (of NOR/SRAM Bank 1~4) is enabled */
	FMC_NORSRAMCmd(FMC_Bank1_NORSRAM2, ENABLE);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
