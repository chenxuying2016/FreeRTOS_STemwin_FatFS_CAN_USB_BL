/*
*********************************************************************************************************
*
*	模块名称 : 外部SDRAM驱动模块
*	文件名称 : bsp_fmc_sdram.c
*	版    本 : V2.4
*	说    明 : 安富莱STM32-X6 V6开发板标配的 SDRAM为美光 MT48LC4M32B2TG-7  容量32M字节，32Bit, 7ns速度 (143MHz)
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2014-05-04 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"

/* FMC SDRAM Bank address */
#define SDRAM_BANK_ADDR     ((uint32_t)0xC0000000)

/* FMC SDRAM Memory Width */
/* #define SDRAM_MEMORY_WIDTH   FMC_SDMemory_Width_8b  */
/* #define SDRAM_MEMORY_WIDTH    FMC_SDMemory_Width_16b */
#define SDRAM_MEMORY_WIDTH    FMC_SDMemory_Width_32b  /* Default configuration used with LCD */

/* FMC SDRAM Memory clock period */
#define SDCLOCK_PERIOD    FMC_SDClock_Period_2        /* Default configuration used with LCD */
/* #define SDCLOCK_PERIOD    FMC_SDClock_Period_3 */

/* SDRAM超时 */
#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

/* FMC SDRAM Mode definition register defines */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_1              ((uint16_t)0x0010)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

static void SDRAM_GPIOConfig(void);
static void SDRAM_InitSequence(void);
void FMC_Config1(void);
/*
*********************************************************************************************************
*	函 数 名: bsp_InitExtSDRAM
*	功能说明: 配置连接外部SDRAM的GPIO和FMC
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitExtSDRAM(void)
{
	FMC_SDRAMInitTypeDef  FMC_SDRAMInitStructure;
	FMC_SDRAMTimingInitTypeDef  FMC_SDRAMTimingInitStructure;

	/* GPIO configuration for FMC SDRAM bank */
	SDRAM_GPIOConfig();

	/* Enable FMC clock */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);

	/* FMC Configuration ---------------------------------------------------------*/
	/* FMC SDRAM Bank configuration */
	/* Timing configuration for 90 Mhz of SD clock frequency (180Mhz/2)  11.11ns
	   Timing configuration for 84 Mhz of SD clock frequency (168Mhz/2)  11.90ns
	*/
	/* TMRD: 2 Clock cycles */
	FMC_SDRAMTimingInitStructure.FMC_LoadToActiveDelay    = 2;
	/* TXSR: min=70ns (7x11.11ns or 11.90) */
	FMC_SDRAMTimingInitStructure.FMC_ExitSelfRefreshDelay = 7;
	/* TRAS: min=42ns (4x11.11ns or 11.90) max=120k (ns) */
	FMC_SDRAMTimingInitStructure.FMC_SelfRefreshTime      = 4;
	/* TRC:  min=70 (7x11.11ns or 11.90) */
	FMC_SDRAMTimingInitStructure.FMC_RowCycleDelay        = 7;
	/* TWR:  min=1+ 7ns (1+1x11.11ns or 11.90) 2修改为1 ---------*/
	FMC_SDRAMTimingInitStructure.FMC_WriteRecoveryTime    = 2; 
	/* TRP:  min=20ns  2x11.11ns or 11.90 */
	FMC_SDRAMTimingInitStructure.FMC_RPDelay              = 2;
	/* TRCD: min=20ns  2x11.11ns or 11.90 */
	FMC_SDRAMTimingInitStructure.FMC_RCDDelay             = 2;

	/* FMC SDRAM control configuration */
	FMC_SDRAMInitStructure.FMC_Bank               = FMC_Bank1_SDRAM;
	/* Row addressing: [7:0] */
	FMC_SDRAMInitStructure.FMC_ColumnBitsNumber   = FMC_ColumnBits_Number_8b;
	/* Column addressing: [10:0] --> [11:0], _12b = 16M,  _11b = 8M */
	FMC_SDRAMInitStructure.FMC_RowBitsNumber      = FMC_RowBits_Number_12b;		
	FMC_SDRAMInitStructure.FMC_SDMemoryDataWidth  = SDRAM_MEMORY_WIDTH;
	FMC_SDRAMInitStructure.FMC_InternalBankNumber = FMC_InternalBank_Number_4;
	/* CL: Cas Latency = 3 clock cycles 3修改为2 --------------*/
	FMC_SDRAMInitStructure.FMC_CASLatency         = FMC_CAS_Latency_3;			/* 选 FMC_CAS_Latency_2 不行 */
	FMC_SDRAMInitStructure.FMC_WriteProtection    = FMC_Write_Protection_Disable;
	FMC_SDRAMInitStructure.FMC_SDClockPeriod      = SDCLOCK_PERIOD;
	FMC_SDRAMInitStructure.FMC_ReadBurst          = FMC_Read_Burst_Enable;
	/* CL: Cas Latency = 3 clock cycles 1修改为0----------------- */
	FMC_SDRAMInitStructure.FMC_ReadPipeDelay      = FMC_ReadPipe_Delay_1;
	FMC_SDRAMInitStructure.FMC_SDRAMTimingStruct  = &FMC_SDRAMTimingInitStructure;

	/* FMC SDRAM bank initialization */
	FMC_SDRAMInit(&FMC_SDRAMInitStructure);

	/* FMC SDRAM device initialization sequence */
	SDRAM_InitSequence();
}

/*
*********************************************************************************************************
*	函 数 名: SDRAM_GPIOConfig
*	功能说明: 配置连接外部SDRAM的GPIO
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void SDRAM_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIOs clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE |
	                     RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH |
	                     RCC_AHB1Periph_GPIOI, ENABLE);

	/*-- 安富莱STM32-X6、V6开发板 SDRAM GPIO 定义 -----------------------------------------------------*/
	/*
	 +-------------------+--------------------+--------------------+--------------------+
	 +                       SDRAM pins assignment                                      +
	 +-------------------+--------------------+--------------------+--------------------+
	 | PD0  <-> FMC_D2   | PE0  <-> FMC_NBL0  | PF0  <-> FMC_A0    | PG0 <-> FMC_A10    |
	 | PD1  <-> FMC_D3   | PE1  <-> FMC_NBL1  | PF1  <-> FMC_A1    | PG1 <-> FMC_A11    |
	 | PD8  <-> FMC_D13  | PE7  <-> FMC_D4    | PF2  <-> FMC_A2    | PG4 <-> FMC_A14    |
	 | PD9  <-> FMC_D14  | PE8  <-> FMC_D5    | PF3  <-> FMC_A3    | PG5 <-> FMC_A15    |
	 | PD10 <-> FMC_D15  | PE9  <-> FMC_D6    | PF4  <-> FMC_A4    | PG8 <-> FC_SDCLK   |
	 | PD14 <-> FMC_D0   | PE10 <-> FMC_D7    | PF5  <-> FMC_A5    | PG15 <-> FMC_NCAS  |
	 | PD15 <-> FMC_D1   | PE11 <-> FMC_D8    | PF11 <-> FC_NRAS   |--------------------+
	 +-------------------| PE12 <-> FMC_D9    | PF12 <-> FMC_A6    |
	                     | PE13 <-> FMC_D10   | PF13 <-> FMC_A7    |
	                     | PE14 <-> FMC_D11   | PF14 <-> FMC_A8    |
	                     | PE15 <-> FMC_D12   | PF15 <-> FMC_A9    |
	 +-------------------+--------------------+--------------------+
	 | PH2 <-> FMC_SDCKE0| PI4 <-> FMC_NBL2   |
	 | PH3 <-> FMC_SDNE0 | PI5 <-> FMC_NBL3   |
	 | PH5 <-> FMC_SDNW  |--------------------+
	 +-------------------+
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

	 +-------------------+
	 +  Pins remapping   +
	 +-------------------+
	 | PC0 <-> FMC_SDNWE |
	 | PC2 <-> FMC_SDNE0 |
	 | PC3 <-> FMC_SDCKE0|
	 +-------------------+

	*/

	/* 公共的 GPIO 设置 */
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	/* 配置 GPIOD */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  |GPIO_Pin_1  |GPIO_Pin_8 |GPIO_Pin_9 |
	                            GPIO_Pin_10 |GPIO_Pin_14 |GPIO_Pin_15;

	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* 配置 GPIOE */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_7 | GPIO_Pin_8  |
	                            GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11| GPIO_Pin_12 |
	                            GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* 配置 GPIOF */
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  |
	                            GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_11 | GPIO_Pin_12 |
	                            GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* 配置 GPIOG */
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource15 , GPIO_AF_FMC);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 |GPIO_Pin_4 |GPIO_Pin_5 |
	                            GPIO_Pin_8 | GPIO_Pin_15;

	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* 配置 GPIOH */
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource2 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource3 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource8 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource9 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource10 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource11 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource12 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource13 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource14 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOH, GPIO_PinSource15 , GPIO_AF_FMC);


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2  | GPIO_Pin_3  | GPIO_Pin_5 | GPIO_Pin_8  |
	                            GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 |
	                            GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOH, &GPIO_InitStructure);

	/* 配置 GPIOI */
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource0 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource1 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource2 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource3 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource4 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource5 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource6 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource7 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource9 , GPIO_AF_FMC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource10 , GPIO_AF_FMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
	                            GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
				                        GPIO_Pin_9 | GPIO_Pin_10;

	GPIO_Init(GPIOI, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: SDRAM_InitSequence
*	功能说明: 执行SDRAM初始化序列
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void SDRAM_InitSequence(void)
{
	FMC_SDRAMCommandTypeDef FMC_SDRAMCommandStructure;
	uint32_t tmpr = 0;
	uint32_t timeout = SDRAM_TIMEOUT;

	/* Step 3 --------------------------------------------------------------------*/
	/* Configure a clock configuration enable command */
	FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_CLK_Enabled;
	FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
	FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
	FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
	/* Wait until the SDRAM controller is ready */
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
		timeout--;
	}
	/* Send the command */
	FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

	/* Step 4 --------------------------------------------------------------------*/
	/* Insert 100 ms delay */
	bsp_DelayMS(100);

	/* Step 5 --------------------------------------------------------------------*/
	/* Configure a PALL (precharge all) command */
	FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_PALL;
	FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
	FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
	FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;

	/* Wait until the SDRAM controller is ready */
	timeout = SDRAM_TIMEOUT;
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
		timeout--;
	}
	/* Send the command */
	FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

	/* Step 6 --------------------------------------------------------------------*/
	/* Configure a Auto-Refresh command */
	FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_AutoRefresh;
	FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
	FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 8;
	FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;

	/* Wait until the SDRAM controller is ready */
	timeout = SDRAM_TIMEOUT;
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
		timeout--;
	}
	/* Send the command */
	FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

	/* Step 7 --------------------------------------------------------------------*/
	/* Program the external memory mode register */
	tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1    |
	           SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
	           SDRAM_MODEREG_CAS_LATENCY_3           |
	           SDRAM_MODEREG_OPERATING_MODE_STANDARD |
	           SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	/* Configure a load Mode register command*/
	FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_LoadMode;
	FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
	FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
	FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = tmpr;

	/* Wait until the SDRAM controller is ready */
	timeout = SDRAM_TIMEOUT;
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
		timeout--;
	}
	/* Send the command */
	FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

	/* Step 8 --------------------------------------------------------------------*/

	/* Set the refresh rate counter */
	/* (15.62 us x Freq) - 20 */
	/* Set the device refresh counter */
	FMC_SetRefreshCount(1385);

	/* Wait until the SDRAM controller is ready */
	timeout = SDRAM_TIMEOUT;
	while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
	{
		timeout--;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_TestExtSDRAM
*	功能说明: 扫描测试外部SRAM, 全部单元。
*	形    参: 无
*	返 回 值: 0 表示测试通过； 大于0表示错误单元的个数。
*********************************************************************************************************
*/
uint32_t bsp_TestExtSDRAM1(void)
{
	uint32_t i;
	uint32_t *pSRAM;
	uint8_t *pBytes;
	uint32_t err;
	const uint8_t ByteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

	/* 写SRAM */
	pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
	for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
	{
		*pSRAM++ = i;
	}

	/* 读SRAM */
	err = 0;
	pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
	for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
	{
		if (*pSRAM++ != i)
		{
			err++;
		}
	}

	if (err >  0)
	{
		return  (4 * err);
	}

	/* 对SRAM 的数据求反并写入 */
	pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
	for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
	{
		*pSRAM = ~*pSRAM;
		pSRAM++;
	}

	/* 再次比较SDRAM的数据 */
	err = 0;
	pSRAM = (uint32_t *)EXT_SDRAM_ADDR;
	for (i = 0; i < EXT_SDRAM_SIZE / 4; i++)
	{
		if (*pSRAM++ != (~i))
		{
			err++;
		}
	}

	if (err >  0)
	{
		return (4 * err);
	}

	/* 测试按字节方式访问, 目的是验证 FSMC_NBL0 、 FSMC_NBL1 口线 */
	pBytes = (uint8_t *)EXT_SDRAM_ADDR;
	for (i = 0; i < sizeof(ByteBuf); i++)
	{
		*pBytes++ = ByteBuf[i];
	}

	/* 比较SDRAM的数据 */
	err = 0;
	pBytes = (uint8_t *)EXT_SDRAM_ADDR;
	for (i = 0; i < sizeof(ByteBuf); i++)
	{
		if (*pBytes++ != ByteBuf[i])
		{
			err++;
		}
	}
	if (err >  0)
	{
		return err;
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_TestExtSDRAM2
*	功能说明: 扫描测试外部SDRAM. 不扫描前面4M字节的显存。
*	形    参: 无
*	返 回 值: 0 表示测试通过； 大于0表示错误单元的个数。
*********************************************************************************************************
*/
uint32_t bsp_TestExtSDRAM2(void)
{
	uint32_t i;
	uint32_t *pSRAM;
	uint8_t *pBytes;
	uint32_t err;
	const uint8_t ByteBuf[4] = {0x55, 0xA5, 0x5A, 0xAA};

	/* 写SRAM */
	pSRAM = (uint32_t *)SDRAM_APP_BUF;
	for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
	{
		*pSRAM++ = i;
	}

	/* 读SRAM */
	err = 0;
	pSRAM = (uint32_t *)SDRAM_APP_BUF;
	for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
	{
		if (*pSRAM++ != i)
		{
			err++;
		}
	}

	if (err >  0)
	{
		return  (4 * err);
	}

#if 0
	/* 对SRAM 的数据求反并写入 */
	pSRAM = (uint32_t *)SDRAM_APP_BUF;
	for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
	{
		*pSRAM = ~*pSRAM;
		pSRAM++;
	}

	/* 再次比较SDRAM的数据 */
	err = 0;
	pSRAM = (uint32_t *)SDRAM_APP_BUF;
	for (i = 0; i < SDRAM_APP_SIZE / 4; i++)
	{
		if (*pSRAM++ != (~i))
		{
			err++;
		}
	}

	if (err >  0)
	{
		return (4 * err);
	}
#endif	

	/* 测试按字节方式访问, 目的是验证 FSMC_NBL0 、 FSMC_NBL1 口线 */
	pBytes = (uint8_t *)SDRAM_APP_BUF;
	for (i = 0; i < sizeof(ByteBuf); i++)
	{
		*pBytes++ = ByteBuf[i];
	}

	/* 比较SDRAM的数据 */
	err = 0;
	pBytes = (uint8_t *)SDRAM_APP_BUF;
	for (i = 0; i < sizeof(ByteBuf); i++)
	{
		if (*pBytes++ != ByteBuf[i])
		{
			err++;
		}
	}
	if (err >  0)
	{
		return err;
	}
	return 0;
}

void FMC_Config1(void)
{
  GPIO_InitTypeDef            GPIO_InitStructure;
  FMC_SDRAMInitTypeDef        FMC_SDRAMInitStructure;
  FMC_SDRAMTimingInitTypeDef  FMC_SDRAMTimingInitStructure;
  FMC_SDRAMCommandTypeDef     FMC_SDRAMCommandStructure;

  uint32_t tmpr = 0;
  uint32_t timeout = SDRAM_TIMEOUT;

  /* GPIO configuration ------------------------------------------------------*/
  /* Enable GPIOs clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE |
                         RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH |
                         RCC_AHB1Periph_GPIOI, ENABLE);

  /* Common GPIO configuration */
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  /* GPIOD configuration */
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  |GPIO_Pin_1  |GPIO_Pin_8 |GPIO_Pin_9 |
                                GPIO_Pin_10 |GPIO_Pin_14 |GPIO_Pin_15;

  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* GPIOE configuration */
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_7 | GPIO_Pin_8  |
                                GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11| GPIO_Pin_12 |
                                GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource2 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource3 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource4 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource11 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource12 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource13 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource14 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource15 , GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  |
                                GPIO_Pin_4  | GPIO_Pin_5  | GPIO_Pin_11 | GPIO_Pin_12 |
                                GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

  GPIO_Init(GPIOF, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource8 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource15 , GPIO_AF_FMC);


  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 |GPIO_Pin_4 |GPIO_Pin_5 |
                                GPIO_Pin_8 | GPIO_Pin_15;

  GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* GPIOH configuration */
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource2 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource3 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource8 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource9 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource10 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource11 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource12 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource13 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource14 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource15 , GPIO_AF_FMC);


  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2  | GPIO_Pin_3  | GPIO_Pin_5 | GPIO_Pin_8  |
                                GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 |
                                GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

  GPIO_Init(GPIOH, &GPIO_InitStructure);

  /* GPIOI configuration */
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource0 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource1 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource2 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource3 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource4 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource5 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource6 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource7 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource9 , GPIO_AF_FMC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource10 , GPIO_AF_FMC);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
				                        GPIO_Pin_9 | GPIO_Pin_10;

  GPIO_Init(GPIOI, &GPIO_InitStructure);

  /* Enable FMC clock */
  RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FMC, ENABLE);

  /* FMC SDRAM device initialization sequence --------------------------------*/
  /* Step 1 ----------------------------------------------------*/
  /* Timing configuration for 84 Mhz of SD clock frequency (168Mhz/2) */
  /* TMRD: 2 Clock cycles */
  FMC_SDRAMTimingInitStructure.FMC_LoadToActiveDelay    = 2;
  /* TXSR: min=70ns (6x11.90ns) */
  FMC_SDRAMTimingInitStructure.FMC_ExitSelfRefreshDelay = 6;
  /* TRAS: min=42ns (4x11.90ns) max=120k (ns) */
  FMC_SDRAMTimingInitStructure.FMC_SelfRefreshTime      = 4;
  /* TRC:  min=70 (6x11.90ns) */
  FMC_SDRAMTimingInitStructure.FMC_RowCycleDelay        = 6;
  /* TWR:  min=1+ 7ns (1+1x11.90ns) */
  FMC_SDRAMTimingInitStructure.FMC_WriteRecoveryTime    = 2;
  /* TRP:  20ns => 2x11.90ns */
  FMC_SDRAMTimingInitStructure.FMC_RPDelay              = 2;
  /* TRCD: 20ns => 2x11.90ns */
  FMC_SDRAMTimingInitStructure.FMC_RCDDelay             = 2;

  /* Step 2 ----------------------------------------------------*/

  /* FMC SDRAM control configuration */
  FMC_SDRAMInitStructure.FMC_Bank = FMC_Bank1_SDRAM;

  /* Row addressing: [7:0] */
  FMC_SDRAMInitStructure.FMC_ColumnBitsNumber   = FMC_ColumnBits_Number_8b;
  /* Column addressing: [10:0] */
  FMC_SDRAMInitStructure.FMC_RowBitsNumber      = FMC_RowBits_Number_11b;
  FMC_SDRAMInitStructure.FMC_SDMemoryDataWidth  = SDRAM_MEMORY_WIDTH;
  FMC_SDRAMInitStructure.FMC_InternalBankNumber = FMC_InternalBank_Number_4;
  /* CL: Cas Latency = 3 clock cycles */
  FMC_SDRAMInitStructure.FMC_CASLatency         = FMC_CAS_Latency_3;
  FMC_SDRAMInitStructure.FMC_WriteProtection    = FMC_Write_Protection_Disable;
  FMC_SDRAMInitStructure.FMC_SDClockPeriod      = SDCLOCK_PERIOD;
  FMC_SDRAMInitStructure.FMC_ReadBurst          = FMC_Read_Burst_Enable;
  FMC_SDRAMInitStructure.FMC_ReadPipeDelay      = FMC_ReadPipe_Delay_1;
  FMC_SDRAMInitStructure.FMC_SDRAMTimingStruct  = &FMC_SDRAMTimingInitStructure;
  /* FMC SDRAM bank initialization */
  FMC_SDRAMInit(&FMC_SDRAMInitStructure);

/* Step 3 --------------------------------------------------------------------*/
  /* Configure a clock configuration enable command */
  FMC_SDRAMCommandStructure.FMC_CommandMode            = FMC_Command_Mode_CLK_Enabled;
  FMC_SDRAMCommandStructure.FMC_CommandTarget          = FMC_Command_Target_bank1;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber      = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */
  while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

/* Step 4 --------------------------------------------------------------------*/
  /* Insert 100 ms delay */
  bsp_DelayMS(100);

/* Step 5 --------------------------------------------------------------------*/
  /* Configure a PALL (precharge all) command */
  FMC_SDRAMCommandStructure.FMC_CommandMode            = FMC_Command_Mode_PALL;
  FMC_SDRAMCommandStructure.FMC_CommandTarget          = FMC_Command_Target_bank1;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber      = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;

  /* Wait until the SDRAM controller is ready */
  timeout = SDRAM_TIMEOUT;
  while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

/* Step 6 --------------------------------------------------------------------*/
  /* Configure a Auto-Refresh command */
  FMC_SDRAMCommandStructure.FMC_CommandMode            = FMC_Command_Mode_AutoRefresh;
  FMC_SDRAMCommandStructure.FMC_CommandTarget          = FMC_Command_Target_bank1;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber      = 8;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;

  /* Wait until the SDRAM controller is ready */
  timeout = SDRAM_TIMEOUT;
  while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

/* Step 7 --------------------------------------------------------------------*/
  /* Program the external memory mode register */
  tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_8          |
                   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                   SDRAM_MODEREG_CAS_LATENCY_3           |
                   SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                   SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  /* Configure a load Mode register command*/
  FMC_SDRAMCommandStructure.FMC_CommandMode            = FMC_Command_Mode_LoadMode;
  FMC_SDRAMCommandStructure.FMC_CommandTarget          = FMC_Command_Target_bank1;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber      = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = tmpr;

  /* Wait until the SDRAM controller is ready */
  timeout = SDRAM_TIMEOUT;
  while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

/* Step 8 --------------------------------------------------------------------*/

  /* Set the refresh rate counter */
  /* (7.81 us x Freq) - 20 */
  /* Set the device refresh counter */
  FMC_SetRefreshCount(636);

  /* Wait until the SDRAM controller is ready */
  timeout = SDRAM_TIMEOUT;
  while((FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) && (timeout > 0))
  {
    timeout--;
  }
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/

