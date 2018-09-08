/*
*********************************************************************************************************
*
*	模块名称 : WM8978音频芯片驱动模块
*	文件名称 : bsp_wm8978.h
*	版    本 : V1.4
*	说    明 : WM8978音频芯片和STM32 I2S底层驱动。在安富莱STM32-V5开发板上调试通过。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 armfly  正式发布
*		V1.1    2013-06-12 armfly  解决单独Line 输入不能放音的问题。修改 wm8978_CfgAudioPath() 函数
*		V1.2    2013-07-14 armfly  增加设置Line输入接口增益的函数： wm8978_SetLineGain()
*		V1.3    2015-10-18 armfly  移植到STM32F429，改动很大。I2S接口修改为SAI音频接口。
*							-  wm8978_CfgAudioIF() 函数的字长形参，增加20bit
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"

/*
*********************************************************************************************************
*
*	重要提示:
*	1、wm8978_ 开头的函数是操作WM8978寄存器，操作WM8978寄存器是通过I2C模拟总线进行的
*	2、I2S_ 开头的函数是操作STM32  I2S相关寄存器
*	3、实现录音或放音应用，需要同时操作WM8978和STM32的I2S。
*	4、部分函数用到的形参的定义在ST固件库中，比如：I2S_Standard_Phillips、I2S_Standard_MSB、I2S_Standard_LSB
*			  I2S_MCLKOutput_Enable、I2S_MCLKOutput_Disable
*			  I2S_AudioFreq_8K、I2S_AudioFreq_16K、I2S_AudioFreq_22K、I2S_AudioFreq_44K、I2S_AudioFreq_48
*			  I2S_Mode_MasterTx、I2S_Mode_MasterRx
*	5、注释中 pdf 指的是 wm8978.pdf 数据手册，wm8978de寄存器很多，用到的寄存器会注释pdf文件的页码，便于查询
*
*********************************************************************************************************
*/

/* 
	安富莱STM32-V6开发板---  SAI接口 I2S总线传输音频数据口线	
		PF9/SAI1_FS_B
		PF8/SAI1_SCK_B
		PD6/SAI1_SD_A			ADC 录音
		PF6/SAI1_SD_B			DAC 放音
		PF7/SAI1_MCLK_B	
		

	STM32的SAI配置为主模式。SAIT_Block_A 和 SAIT_Block_B 同步模式工作，其中SAIT_Block_B作为主模块输出时钟.
	
	主模块 SAIT_Block_B 的 SAI1_SD_B 引脚用于放音；从模块 SAIT_Block_A的SAI1_SD_A用于录音。
	
	采用标准I2S协议。

    音频模块可声明为与第二个音频模块同步。在这种情况下，将共用位时钟和帧同步信号，以减少通信时占用外部引脚的数量。声明为与另一个模块同步的音频模块将释放其 SCK_x、
FS_x 和 MCLK_x 引脚以用作 GPIO


*/

#define AUDIO_MAL_MODE_NORMAL
//#define AUDIO_MAL_MODE_CIRCULAR

/* For the DMA modes select the interrupt that will be used */

/* Uncomment this line to enable DMA Transfer Complete interrupt -- DMA传输完成中断 */
#define PLAY_DMA_IT_TC_EN    /* 放音的 */
#define REC_DMA_IT_TC_EN     /* 录音的 */

/* Uncomment this line to enable DMA Half Transfer Complete interrupt -- 半传输中断 */      
/* #define PLAY_DMA_IT_HT_EN */

/* Uncomment this line to enable DMA Transfer Error interrupt -- DMA传输错误中断 */  
/* #define PLAY_DMA_IT_TE_EN */  

#define  EVAL_AUDIO_IRQ_PREPRIO  3
#define  EVAL_AUDIO_IRQ_SUBRIO	 0
	
/*----------------------------------------------------------------------------
                    USER SAI defines parameters
 -----------------------------------------------------------------------------*/
/* In Main Application the PLL_SAI or PLL_I2S is configured to have this specific
   output, used to have a clean Audio Frequency */
#define SAI_ClockPLLSAI             ((uint32_t)11289600)

#define SAI_ClockPLLI2S             ((uint32_t)49152000)
#define SAI_ClockExtern             ((uint32_t)14000000)

//#define SAI_CLOCK_SOURCE            SAI_ClockPLLI2S       /* Default configuration */
#define SAI_CLOCK_SOURCE            SAI_ClockPLLSAI
/* #define SAI_CLOCK_SOURCE            SAI_ClockExtern */

/* 音频子模块的音频数据寄存器, 用于DMA传输配置 */
#define SAI_BLOCK1_DR	        (uint32_t)(&SAI1_Block_B->DR) 		/* 放音的 */
#define SAI_BLOCK2_DR	        (uint32_t)(&SAI1_Block_A->DR) 		/* 录音的 */

#define SAI_RCC                  RCC_APB2Periph_SAI1
#define SAI_GPIO_AF              GPIO_AF_SAI1
#define SAI_BLOCK1           	 SAI1_Block_B		/* 主音频子模块 - 放音 */
#define SAI_BLOCK2           	 SAI1_Block_A		/* 从音频子模块 - 录音 */

#define SAI_GPIO_RCC             (RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOD)

#define SAI_GPIO_FS              GPIOF
#define SAI_PIN_FS               GPIO_Pin_9
#define SAI_PINSRC_FS            GPIO_PinSource9

#define SAI_GPIO_SCK             GPIOF
#define SAI_PIN_SCK              GPIO_Pin_8
#define SAI_PINSRC_SCK           GPIO_PinSource8

#define SAI_GPIO_MCK             GPIOF
#define SAI_PIN_MCK              GPIO_Pin_7
#define SAI_PINSRC_MCK           GPIO_PinSource7

#define SAI_GPIO_SD1             GPIOF
#define SAI_PIN_SD1              GPIO_Pin_6
#define SAI_PINSRC_SD1           GPIO_PinSource6

#define SAI_GPIO_SD2             GPIOD
#define SAI_PIN_SD2              GPIO_Pin_6
#define SAI_PINSRC_SD2           GPIO_PinSource6


#define DMA_MAX_SZE                    0xFFFF
#define DMA_MAX(x)                (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)


/* DMA传输相关的标志:
	半传输          HTIF  HTIE
	传输完成        TCIF  TCIE
	传输错误        TEIF  TEIE
	FIFO 上溢/下溢  FEIF  FEIE
	直接模式错误    DMEIF  DMEI
*/

/* SAI DMA Stream definitions  <---- 放音的 SAI1_B 使用 DMA2_Stream5 _Channel_0 */
#define PLAY_DMA_CLOCK            RCC_AHB1Periph_DMA2
#define PLAY_DMA_STREAM           DMA2_Stream5
#define PLAY_DMA_CHANNEL          DMA_Channel_0
#define PLAY_DMA_IRQ              DMA2_Stream5_IRQn
#define PLAY_DMA_FLAG_TC          DMA_FLAG_TCIF5
#define PLAY_DMA_FLAG_HT          DMA_FLAG_HTIF5
#define PLAY_DMA_FLAG_FE          DMA_FLAG_FEIF5
#define PLAY_DMA_FLAG_TE          DMA_FLAG_TEIF5
#define PLAY_DMA_FLAG_DME         DMA_FLAG_DMEIF5
#define PLAY_DMA_FLAG_ALL         (uint32_t)(PLAY_DMA_FLAG_TC | PLAY_DMA_FLAG_HT | PLAY_DMA_FLAG_FE | PLAY_DMA_FLAG_TE | PLAY_DMA_FLAG_DME)
#define PLAY_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
#define PLAY_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord

#define PLAY_DMA_IRQHandler      DMA2_Stream5_IRQHandler

/* SAI DMA Stream definitions <---- 录音的 SAI1_A 使用 DMA2_Stream1 _Channel_0;  DMA2_Stream3被SDIO占用  
	DMA2_Stream3 被SDIO占用
	DMA2_Stream1 被CAMERA占用；将CAMER 的 DMA Stream1 修改为 DMA Stream7.
*/
#define REC_DMA_CLOCK            RCC_AHB1Periph_DMA2
#define REC_DMA_STREAM           DMA2_Stream1
#define REC_DMA_CHANNEL          DMA_Channel_0
#define REC_DMA_IRQ              DMA2_Stream1_IRQn
#define REC_DMA_FLAG_TC          DMA_FLAG_TCIF1
#define REC_DMA_FLAG_HT          DMA_FLAG_HTIF1
#define REC_DMA_FLAG_FE          DMA_FLAG_FEIF1
#define REC_DMA_FLAG_TE          DMA_FLAG_TEIF1
#define REC_DMA_FLAG_DME         DMA_FLAG_DMEIF1
#define REC_DMA_FLAG_ALL         (uint32_t)(REC_DMA_FLAG_TC | REC_DMA_FLAG_HT | REC_DMA_FLAG_FE | REC_DMA_FLAG_TE | REC_DMA_FLAG_DME)
#define REC_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
#define REC_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord

#define REC_DMA_IRQHandler       DMA2_Stream1_IRQHandler


/* 仅在本模块内部使用的局部函数 */
static uint16_t wm8978_ReadReg(uint8_t _ucRegAddr);
static uint8_t wm8978_WriteReg(uint8_t _ucRegAddr, uint16_t _usValue);
static void SAI_GPIO_Config(void);

static void SAI_Mode_Config(uint8_t _mode, uint16_t _usStandard, uint32_t _uiWordLen, uint32_t _uiAudioFreq);

static void wm8978_Reset(void);

static void Play_DMA_Init(void);
static void Rec_DMA_Init(void);

static void PLLSAI_Config(void);
static void Rec_SAI_DMA_DeInit(void);
static void Play_SAI_DMA_DeInit(void);

/*
	wm8978寄存器缓存
	由于WM8978的I2C两线接口不支持读取操作，因此寄存器值缓存在内存中，当写寄存器时同步更新缓存，读寄存器时
	直接返回缓存中的值。
	寄存器MAP 在WM8978.pdf 的第67页，寄存器地址是7bit， 寄存器数据是9bit
*/
static uint16_t wm8978_RegCash[] = {
	0x000, 0x000, 0x000, 0x000, 0x050, 0x000, 0x140, 0x000,
	0x000, 0x000, 0x000, 0x0FF, 0x0FF, 0x000, 0x100, 0x0FF,
	0x0FF, 0x000, 0x12C, 0x02C, 0x02C, 0x02C, 0x02C, 0x000,
	0x032, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x038, 0x00B, 0x032, 0x000, 0x008, 0x00C, 0x093, 0x0E9,
	0x000, 0x000, 0x000, 0x000, 0x003, 0x010, 0x010, 0x100,
	0x100, 0x002, 0x001, 0x001, 0x039, 0x039, 0x039, 0x039,
	0x001, 0x001
};

/* 这个全部变量在初始化函数和中断服务程序中被使用，因此申明为全局变量 */
static DMA_InitTypeDef DMA_Init_Play; 

static DMA_InitTypeDef DMA_Init_Rec; 

typedef struct
{
	/* This variable holds the total size of the audio file */
	uint32_t AudioTotalSize;
	/* This variable holds the remaining data in audio file */ 
	uint32_t AudioRemSize;
	/* This variable holds the current position of audio pointer */ 
	uint16_t *CurrentPos;  	
}AUDIO_CTRL_T;


static AUDIO_CTRL_T s_tPlay;
static AUDIO_CTRL_T s_tRec;


/*
*********************************************************************************************************
*	函 数 名: wm8978_Init
*	功能说明: 配置I2C GPIO，并检查I2C总线上的WM8978是否正常
*	形    参:  无
*	返 回 值: 1 表示初始化正常，0表示初始化不正常
*********************************************************************************************************
*/
uint8_t wm8978_Init(void)
{
	uint8_t re;

	if (i2c_CheckDevice(WM8978_SLAVE_ADDRESS) == 0)	/* 这个函数会配置STM32的GPIO用于软件模拟I2C时序 */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	wm8978_Reset();			/* 硬件复位WM8978所有寄存器到缺省状态 */
	return re;
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_SetEarVolume
*	功能说明: 修改耳机输出音量
*	形    参:  _ucLeftVolume ：左声道音量值, 0-63
*			  _ucLRightVolume : 右声道音量值,0-63
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_SetEarVolume(uint8_t _ucVolume)
{
	uint16_t regL;
	uint16_t regR;

	if (_ucVolume > 0x3F)
	{
		_ucVolume = 0x3F;
	}

	regL = _ucVolume;
	regR = _ucVolume;

	/*
		R52	LOUT1 Volume control
		R53	ROUT1 Volume control
	*/
	/* 先更新左声道缓存值 */
	wm8978_WriteReg(52, regL | 0x00);

	/* 再同步更新左右声道的音量 */
	wm8978_WriteReg(53, regR | 0x100);	/* 0x180表示 在音量为0时再更新，避免调节音量出现的“嘎哒”声 */
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_SetSpkVolume
*	功能说明: 修改扬声器输出音量
*	形    参:  _ucLeftVolume ：左声道音量值, 0-63
*			  _ucLRightVolume : 右声道音量值,0-63
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_SetSpkVolume(uint8_t _ucVolume)
{
	uint16_t regL;
	uint16_t regR;

	if (_ucVolume > 0x3F)
	{
		_ucVolume = 0x3F;
	}

	regL = _ucVolume;
	regR = _ucVolume;

	/*
		R54	LOUT2 (SPK) Volume control
		R55	ROUT2 (SPK) Volume control
	*/
	/* 先更新左声道缓存值 */
	wm8978_WriteReg(54, regL | 0x00);

	/* 再同步更新左右声道的音量 */
	wm8978_WriteReg(55, regR | 0x100);	/* 在音量为0时再更新，避免调节音量出现的“嘎哒”声 */
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_ReadEarVolume
*	功能说明: 读回耳机通道的音量.
*	形    参:  无
*	返 回 值: 当前音量值
*********************************************************************************************************
*/
uint8_t wm8978_ReadEarVolume(void)
{
	return (uint8_t)(wm8978_ReadReg(52) & 0x3F );
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_ReadSpkVolume
*	功能说明: 读回扬声器通道的音量.
*	形    参:  无
*	返 回 值: 当前音量值
*********************************************************************************************************
*/
uint8_t wm8978_ReadSpkVolume(void)
{
	return (uint8_t)(wm8978_ReadReg(54) & 0x3F );
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_OutMute
*	功能说明: 输出静音.
*	形    参:  _ucMute ：1是静音，0是不静音.
*	返 回 值: 当前音量值
*********************************************************************************************************
*/
void wm8978_OutMute(uint8_t _ucMute)
{
	uint16_t usRegValue;

	if (_ucMute == 1) /* 静音 */
	{
		usRegValue = wm8978_ReadReg(52); /* Left Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(52, usRegValue);

		usRegValue = wm8978_ReadReg(53); /* Left Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(53, usRegValue);

		usRegValue = wm8978_ReadReg(54); /* Right Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(54, usRegValue);

		usRegValue = wm8978_ReadReg(55); /* Right Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(55, usRegValue);
	}
	else	/* 取消静音 */
	{
		usRegValue = wm8978_ReadReg(52);
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(52, usRegValue);

		usRegValue = wm8978_ReadReg(53); /* Left Mixer Control */
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(53, usRegValue);

		usRegValue = wm8978_ReadReg(54);
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(54, usRegValue);

		usRegValue = wm8978_ReadReg(55); /* Left Mixer Control */
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(55, usRegValue);
	}
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_SetMicGain
*	功能说明: 设置MIC增益
*	形    参:  _ucGain ：音量值, 0-63
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_SetMicGain(uint8_t _ucGain)
{
	if (_ucGain > GAIN_MAX)
	{
		_ucGain = GAIN_MAX;
	}

	/* PGA 音量控制  R45， R46   pdf 19页
		Bit8	INPPGAUPDATE
		Bit7	INPPGAZCL		过零再更改
		Bit6	INPPGAMUTEL		PGA静音
		Bit5:0	增益值，010000是0dB
	*/
	wm8978_WriteReg(45, _ucGain);
	wm8978_WriteReg(46, _ucGain | (1 << 8));
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_SetLineGain
*	功能说明: 设置Line输入通道的增益
*	形    参:  _ucGain ：音量值, 0-7. 7最大，0最小。 可衰减可放大。
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_SetLineGain(uint8_t _ucGain)
{
	uint16_t usRegValue;

	if (_ucGain > 7)
	{
		_ucGain = 7;
	}

	/*
		Mic 输入信道的增益由 PGABOOSTL 和 PGABOOSTR 控制
		Aux 输入信道的输入增益由 AUXL2BOOSTVO[2:0] 和 AUXR2BOOSTVO[2:0] 控制
		Line 输入信道的增益由 LIP2BOOSTVOL[2:0] 和 RIP2BOOSTVOL[2:0] 控制
	*/
	/*	pdf 21页，R47（左声道），R48（右声道）, MIC 增益控制寄存器
		R47 (R48定义与此相同)
		B8		PGABOOSTL	= 1,   0表示MIC信号直通无增益，1表示MIC信号+20dB增益（通过自举电路）
		B7		= 0， 保留
		B6:4	L2_2BOOSTVOL = x， 0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
		B3		= 0， 保留
		B2:0`	AUXL2BOOSTVOL = x，0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
	*/

	usRegValue = wm8978_ReadReg(47);
	usRegValue &= 0x8F;/* 将Bit6:4清0   1000 1111*/
	usRegValue |= (_ucGain << 4);
	wm8978_WriteReg(47, usRegValue);	/* 写左声道输入增益控制寄存器 */

	usRegValue = wm8978_ReadReg(48);
	usRegValue &= 0x8F;/* 将Bit6:4清0   1000 1111*/
	usRegValue |= (_ucGain << 4);
	wm8978_WriteReg(48, usRegValue);	/* 写右声道输入增益控制寄存器 */
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_PowerDown
*	功能说明: 关闭wm8978，进入低功耗模式
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_PowerDown(void)
{
	wm8978_Reset();			/* 硬件复位WM8978所有寄存器到缺省状态 */
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_CfgAudioIF
*	功能说明: 配置WM8978的音频接口(I2S)
*	形    参:
*			  _usStandard : 接口标准，I2S_Standard_Phillips, I2S_Standard_MSB 或 I2S_Standard_LSB
*			  _ucWordLen : 字长，16、24、32，20bit格式）
*			  _usMode : CPU I2S的工作模式，I2S_Mode_MasterTx、I2S_Mode_MasterRx、
*						安富莱开发板硬件不支持 I2S_Mode_SlaveTx、I2S_Mode_SlaveRx 模式，这需要WM8978连接
*						外部振荡器
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_CfgAudioIF(uint16_t _usStandard, uint8_t _ucWordLen)
{
	uint16_t usReg;

	/* pdf 67页，寄存器列表 */

	/*	REG R4, 音频接口控制寄存器
		B8		BCP	 = X, BCLK极性，0表示正常，1表示反相
		B7		LRCP = x, LRC时钟极性，0表示正常，1表示反相
		B6:5	WL = x， 字长，00=16bit，01=20bit，10=24bit，11=32bit （右对齐模式只能操作在最大24bit)
		B4:3	FMT = x，音频数据格式，00=右对齐，01=左对齐，10=I2S格式，11=PCM
		B2		DACLRSWAP = x, 控制DAC数据出现在LRC时钟的左边还是右边
		B1 		ADCLRSWAP = x，控制ADC数据出现在LRC时钟的左边还是右边
		B0		MONO	= 0，0表示立体声，1表示单声道，仅左声道有效
	*/
	usReg = 0;
	if (_usStandard == I2S_Standard_Phillips)	/* I2S飞利浦标准 */
	{
		usReg |= (2 << 3);
	}
	else if (_usStandard == I2S_Standard_MSB)	/* MSB对齐标准(左对齐) */
	{
		usReg |= (1 << 3);
	}
	else if (_usStandard == I2S_Standard_LSB)	/* LSB对齐标准(右对齐) */
	{
		usReg |= (0 << 3);
	}
	else	/* PCM标准(16位通道帧上带长或短帧同步或者16位数据帧扩展为32位通道帧) */
	{
		usReg |= (3 << 3);;
	}

	if (_ucWordLen == 24)
	{
		usReg |= (2 << 5);
	}
	else if (_ucWordLen == 32)
	{
		usReg |= (3 << 5);
	}
	else if (_ucWordLen == 20)
	{
		usReg |= (1 << 5);
	}	
	else
	{
		usReg |= (0 << 5);		/* 16bit */
	}
	wm8978_WriteReg(4, usReg);

	/* R5  pdf 57页 */


	/*
		R6，时钟产生控制寄存器
		MS = 0,  WM8978被动时钟，由MCU提供MCLK时钟
	*/
	wm8978_WriteReg(6, 0x000);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_ReadReg
*	功能说明: 从cash中读回读回wm8978寄存器
*	形    参:  _ucRegAddr ： 寄存器地址
*	返 回 值: 无
*********************************************************************************************************
*/
static uint16_t wm8978_ReadReg(uint8_t _ucRegAddr)
{
	return wm8978_RegCash[_ucRegAddr];
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_WriteReg
*	功能说明: 写wm8978寄存器
*	形    参:  _ucRegAddr ： 寄存器地址
*			  _usValue ：寄存器值
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t wm8978_WriteReg(uint8_t _ucRegAddr, uint16_t _usValue)
{
	uint8_t ucAck;

	/* 发送起始位 */
	i2c_Start();

	/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
	i2c_SendByte(WM8978_SLAVE_ADDRESS | I2C_WR);

	/* 检测ACK */
	ucAck = i2c_WaitAck();
	if (ucAck == 1)
	{
		return 0;
	}

	/* 发送控制字节1 */
	i2c_SendByte(((_ucRegAddr << 1) & 0xFE) | ((_usValue >> 8) & 0x1));

	/* 检测ACK */
	ucAck = i2c_WaitAck();
	if (ucAck == 1)
	{
		return 0;
	}

	/* 发送控制字节2 */
	i2c_SendByte(_usValue & 0xFF);

	/* 检测ACK */
	ucAck = i2c_WaitAck();
	if (ucAck == 1)
	{
		return 0;
	}

	/* 发送STOP */
	i2c_Stop();

	wm8978_RegCash[_ucRegAddr] = _usValue;
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_CfgInOut
*	功能说明: 配置wm8978音频通道
*	形    参:
*			_InPath : 音频输入通道配置
*			_OutPath : 音频输出通道配置
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_CfgAudioPath(uint16_t _InPath, uint16_t _OutPath)
{
	uint16_t usReg;

	/* 查看WM8978数据手册的 REGISTER MAP 章节， 第67页 */

	if ((_InPath == IN_PATH_OFF) && (_OutPath == OUT_PATH_OFF))
	{
		wm8978_PowerDown();
		return;
	}

	/* --------------------------- 第1步：根据输入通道参数配置寄存器 -----------------------*/
	/*
		R1 寄存器 Power manage 1
		Bit8    BUFDCOPEN,  Output stage 1.5xAVDD/2 driver enable
 		Bit7    OUT4MIXEN, OUT4 mixer enable
		Bit6    OUT3MIXEN, OUT3 mixer enable
		Bit5    PLLEN	.不用
		Bit4    MICBEN	,Microphone Bias Enable (MIC偏置电路使能)
		Bit3    BIASEN	,Analogue amplifier bias control 必须设置为1模拟放大器才工作
		Bit2    BUFIOEN , Unused input/output tie off buffer enable
		Bit1:0  VMIDSEL, 必须设置为非00值模拟放大器才工作
	*/
	usReg = (1 << 3) | (3 << 0);
	if (_OutPath & OUT3_4_ON) 	/* OUT3和OUT4使能输出到GSM模块 */
	{
		usReg |= ((1 << 7) | (1 << 6));
	}
	if ((_InPath & MIC_LEFT_ON) || (_InPath & MIC_RIGHT_ON))
	{
		usReg |= (1 << 4);
	}
	wm8978_WriteReg(1, usReg);	/* 写寄存器 */

	/*
		R2 寄存器 Power manage 2
		Bit8	ROUT1EN,	ROUT1 output enable 耳机右声道输出使能
		Bit7	LOUT1EN,	LOUT1 output enable 耳机左声道输出使能
		Bit6	SLEEP, 		0 = Normal device operation   1 = Residual current reduced in device standby mode
		Bit5	BOOSTENR,	Right channel Input BOOST enable 输入通道自举电路使能. 用到PGA放大功能时必须使能
		Bit4	BOOSTENL,	Left channel Input BOOST enable
		Bit3	INPGAENR,	Right channel input PGA enable 右声道输入PGA使能
		Bit2	INPGAENL,	Left channel input PGA enable
		Bit1	ADCENR,		Enable ADC right channel
		Bit0	ADCENL,		Enable ADC left channel
	*/
	usReg = 0;
	if (_OutPath & EAR_LEFT_ON)
	{
		usReg |= (1 << 7);
	}
	if (_OutPath & EAR_RIGHT_ON)
	{
		usReg |= (1 << 8);
	}
	if (_InPath & MIC_LEFT_ON)
	{
		usReg |= ((1 << 4) | (1 << 2));
	}
	if (_InPath & MIC_RIGHT_ON)
	{
		usReg |= ((1 << 5) | (1 << 3));
	}
	if (_InPath & LINE_ON)
	{
		usReg |= ((1 << 4) | (1 << 5));
	}
	if (_InPath & MIC_RIGHT_ON)
	{
		usReg |= ((1 << 5) | (1 << 3));
	}
	if (_InPath & ADC_ON)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(2, usReg);	/* 写寄存器 */

	/*
		R3 寄存器 Power manage 3
		Bit8	OUT4EN,		OUT4 enable
		Bit7	OUT3EN,		OUT3 enable
		Bit6	LOUT2EN,	LOUT2 output enable
		Bit5	ROUT2EN,	ROUT2 output enable
		Bit4	0,
		Bit3	RMIXEN,		Right mixer enable
		Bit2	LMIXEN,		Left mixer enable
		Bit1	DACENR,		Right channel DAC enable
		Bit0	DACENL,		Left channel DAC enable
	*/
	usReg = 0;
	if (_OutPath & OUT3_4_ON)
	{
		usReg |= ((1 << 8) | (1 << 7));
	}
	if (_OutPath & SPK_ON)
	{
		usReg |= ((1 << 6) | (1 << 5));
	}
	if (_OutPath != OUT_PATH_OFF)
	{
		usReg |= ((1 << 3) | (1 << 2));
	}
	if (_InPath & DAC_ON)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(3, usReg);	/* 写寄存器 */

	/*
		R44 寄存器 Input ctrl

		Bit8	MBVSEL, 		Microphone Bias Voltage Control   0 = 0.9 * AVDD   1 = 0.6 * AVDD
		Bit7	0
		Bit6	R2_2INPPGA,		Connect R2 pin to right channel input PGA positive terminal
		Bit5	RIN2INPPGA,		Connect RIN pin to right channel input PGA negative terminal
		Bit4	RIP2INPPGA,		Connect RIP pin to right channel input PGA amplifier positive terminal
		Bit3	0
		Bit2	L2_2INPPGA,		Connect L2 pin to left channel input PGA positive terminal
		Bit1	LIN2INPPGA,		Connect LIN pin to left channel input PGA negative terminal
		Bit0	LIP2INPPGA,		Connect LIP pin to left channel input PGA amplifier positive terminal
	*/
	usReg = 0 << 8;
	if (_InPath & LINE_ON)
	{
		usReg |= ((1 << 6) | (1 << 2));
	}
	if (_InPath & MIC_RIGHT_ON)
	{
		usReg |= ((1 << 5) | (1 << 4));
	}
	if (_InPath & MIC_LEFT_ON)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(44, usReg);	/* 写寄存器 */


	/*
		R14 寄存器 ADC Control
		设置高通滤波器（可选的） pdf 24、25页,
		Bit8 	HPFEN,	High Pass Filter Enable高通滤波器使能，0表示禁止，1表示使能
		BIt7 	HPFAPP,	Select audio mode or application mode 选择音频模式或应用模式，0表示音频模式，
		Bit6:4	HPFCUT，Application mode cut-off frequency  000-111选择应用模式的截止频率
		Bit3 	ADCOSR,	ADC oversample rate select: 0=64x (lower power) 1=128x (best performance)
		Bit2   	0
		Bit1 	ADC right channel polarity adjust:  0=normal  1=inverted
		Bit0 	ADC left channel polarity adjust:  0=normal 1=inverted
	*/
	if (_InPath & ADC_ON)
	{
		usReg = (1 << 3) | (0 << 8) | (4 << 0);		/* 禁止ADC高通滤波器, 设置截至频率 */
	}
	else
	{
		usReg = 0;
	}
	wm8978_WriteReg(14, usReg);	/* 写寄存器 */

	/* 设置陷波滤波器（notch filter），主要用于抑制话筒声波正反馈，避免啸叫.  暂时关闭
		R27，R28，R29，R30 用于控制限波滤波器，pdf 26页
		R7的 Bit7 NFEN = 0 表示禁止，1表示使能
	*/
	if (_InPath & ADC_ON)
	{
		usReg = (0 << 7);
		wm8978_WriteReg(27, usReg);	/* 写寄存器 */
		usReg = 0;
		wm8978_WriteReg(28, usReg);	/* 写寄存器,填0，因为已经禁止，所以也可不做 */
		wm8978_WriteReg(29, usReg);	/* 写寄存器,填0，因为已经禁止，所以也可不做 */
		wm8978_WriteReg(30, usReg);	/* 写寄存器,填0，因为已经禁止，所以也可不做 */
	}

	/* 自动增益控制 ALC, R32  - 34  pdf 19页 */
	{
		usReg = 0;		/* 禁止自动增益控制 */
		wm8978_WriteReg(32, usReg);
		wm8978_WriteReg(33, usReg);
		wm8978_WriteReg(34, usReg);
	}

	/*  R35  ALC Noise Gate Control
		Bit3	NGATEN, Noise gate function enable
		Bit2:0	Noise gate threshold:
	*/
	usReg = (3 << 1) | (7 << 0);		/* 禁止自动增益控制 */
	wm8978_WriteReg(35, usReg);

	/*
		Mic 输入信道的增益由 PGABOOSTL 和 PGABOOSTR 控制
		Aux 输入信道的输入增益由 AUXL2BOOSTVO[2:0] 和 AUXR2BOOSTVO[2:0] 控制
		Line 输入信道的增益由 LIP2BOOSTVOL[2:0] 和 RIP2BOOSTVOL[2:0] 控制
	*/
	/*	pdf 21页，R47（左声道），R48（右声道）, MIC 增益控制寄存器
		R47 (R48定义与此相同)
		B8		PGABOOSTL	= 1,   0表示MIC信号直通无增益，1表示MIC信号+20dB增益（通过自举电路）
		B7		= 0， 保留
		B6:4	L2_2BOOSTVOL = x， 0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
		B3		= 0， 保留
		B2:0`	AUXL2BOOSTVOL = x，0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
	*/
	usReg = 0;
	if ((_InPath & MIC_LEFT_ON) || (_InPath & MIC_RIGHT_ON))
	{
		usReg |= (1 << 8);	/* MIC增益取+20dB */
	}
	if (_InPath & AUX_ON)
	{
		usReg |= (3 << 0);	/* Aux增益固定取3，用户可以自行调整 */
	}
	if (_InPath & LINE_ON)
	{
		usReg |= (3 << 4);	/* Line增益固定取3，用户可以自行调整 */
	}
	wm8978_WriteReg(47, usReg);	/* 写左声道输入增益控制寄存器 */
	wm8978_WriteReg(48, usReg);	/* 写右声道输入增益控制寄存器 */

	/* 数字ADC音量控制，pdf 27页
		R15 控制左声道ADC音量，R16控制右声道ADC音量
		Bit8 	ADCVU  = 1 时才更新，用于同步更新左右声道的ADC音量
		Bit7:0 	增益选择； 0000 0000 = 静音
						   0000 0001 = -127dB
						   0000 0010 = -12.5dB  （0.5dB 步长）
						   1111 1111 = 0dB  （不衰减）
	*/
	usReg = 0xFF;
	wm8978_WriteReg(15, usReg);	/* 选择0dB，先缓存左声道 */
	usReg = 0x1FF;
	wm8978_WriteReg(16, usReg);	/* 同步更新左右声道 */

	/* 通过 wm8978_SetMicGain 函数设置mic PGA增益 */

	/*	R43 寄存器  AUXR – ROUT2 BEEP Mixer Function
		B8:6 = 0

		B5	 MUTERPGA2INV,	Mute input to INVROUT2 mixer
		B4	 INVROUT2,  Invert ROUT2 output 用于扬声器推挽输出
		B3:1 BEEPVOL = 7;	AUXR input to ROUT2 inverter gain
		B0	 BEEPEN = 1;	Enable AUXR beep input

	*/
	usReg = 0;
	if (_OutPath & SPK_ON)
	{
		usReg |= (1 << 4);	/* ROUT2 反相, 用于驱动扬声器 */
	}
	if (_InPath & AUX_ON)
	{
		usReg |= ((7 << 1) | (1 << 0));
	}
	wm8978_WriteReg(43, usReg);

	/* R49  Output ctrl
		B8:7	0
		B6		DACL2RMIX,	Left DAC output to right output mixer
		B5		DACR2LMIX,	Right DAC output to left output
		B4		OUT4BOOST,	0 = OUT4 output gain = -1; DC = AVDD / 2；1 = OUT4 output gain = +1.5；DC = 1.5 x AVDD / 2
		B3		OUT3BOOST,	0 = OUT3 output gain = -1; DC = AVDD / 2；1 = OUT3 output gain = +1.5；DC = 1.5 x AVDD / 2
		B2		SPKBOOST,	0 = Speaker gain = -1;  DC = AVDD / 2 ; 1 = Speaker gain = +1.5; DC = 1.5 x AVDD / 2
		B1		TSDEN,   Thermal Shutdown Enable  扬声器热保护使能（缺省1）
		B0		VROI,	Disabled Outputs to VREF Resistance
	*/
	usReg = 0;
	if (_InPath & DAC_ON)
	{
		usReg |= ((1 << 6) | (1 << 5));
	}
	if (_OutPath & SPK_ON)
	{
		usReg |=  ((1 << 2) | (1 << 1));	/* SPK 1.5x增益,  热保护使能 */
	}
	if (_OutPath & OUT3_4_ON)
	{
		usReg |=  ((1 << 4) | (1 << 3));	/* BOOT3  BOOT4  1.5x增益 */
	}
	wm8978_WriteReg(49, usReg);

	/*	REG 50    (50是左声道，51是右声道，配置寄存器功能一致) pdf 40页
		B8:6	AUXLMIXVOL = 111	AUX用于FM收音机信号输入
		B5		AUXL2LMIX = 1		Left Auxilliary input to left channel
		B4:2	BYPLMIXVOL			音量
		B1		BYPL2LMIX = 0;		Left bypass path (from the left channel input boost output) to left output mixer
		B0		DACL2LMIX = 1;		Left DAC output to left output mixer
	*/
	usReg = 0;
	if (_InPath & AUX_ON)
	{
		usReg |= ((7 << 6) | (1 << 5));
	}
	if ((_InPath & LINE_ON) || (_InPath & MIC_LEFT_ON) || (_InPath & MIC_RIGHT_ON))
	{
		usReg |= ((7 << 2) | (1 << 1));
	}
	if (_InPath & DAC_ON)
	{
		usReg |= (1 << 0);
	}
	wm8978_WriteReg(50, usReg);
	wm8978_WriteReg(51, usReg);

	/*	R56 寄存器   OUT3 mixer ctrl
		B8:7	0
		B6		OUT3MUTE,  	0 = Output stage outputs OUT3 mixer;  1 = Output stage muted – drives out VMID.
		B5:4	0
		B3		BYPL2OUT3,	OUT4 mixer output to OUT3  (反相)
		B4		0
		B2		LMIX2OUT3,	Left ADC input to OUT3
		B1		LDAC2OUT3,	Left DAC mixer to OUT3
		B0		LDAC2OUT3,	Left DAC output to OUT3
	*/
	usReg = 0;
	if (_OutPath & OUT3_4_ON)
	{
		usReg |= (1 << 3);
	}
	wm8978_WriteReg(56, usReg);

	/* R57 寄存器		OUT4 (MONO) mixer ctrl
		B8:7	0
		B6		OUT4MUTE,	0 = Output stage outputs OUT4 mixer  1 = Output stage muted – drives outVMID.
		B5		HALFSIG,	0 = OUT4 normal output	1 = OUT4 attenuated by 6dB
		B4		LMIX2OUT4,	Left DAC mixer to OUT4
		B3		LDAC2UT4,	Left DAC to OUT4
		B2		BYPR2OUT4,	Right ADC input to OUT4
		B1		RMIX2OUT4,	Right DAC mixer to OUT4
		B0		RDAC2OUT4,	Right DAC output to OUT4
	*/
	usReg = 0;
	if (_OutPath & OUT3_4_ON)
	{
		usReg |= ((1 << 4) |  (1 << 1));
	}
	wm8978_WriteReg(57, usReg);


	/* R11, 12 寄存器 DAC数字音量
		R11		Left DAC Digital Volume
		R12		Right DAC Digital Volume
	*/
	if (_InPath & DAC_ON)
	{
		wm8978_WriteReg(11, 255);
		wm8978_WriteReg(12, 255 | 0x100);
	}
	else
	{
		wm8978_WriteReg(11, 0);
		wm8978_WriteReg(12, 0 | 0x100);
	}

	/*	R10 寄存器 DAC Control
		B8	0
		B7	0
		B6	SOFTMUTE,	Softmute enable:
		B5	0
		B4	0
		B3	DACOSR128,	DAC oversampling rate: 0=64x (lowest power) 1=128x (best performance)
		B2	AMUTE,		Automute enable
		B1	DACPOLR,	Right DAC output polarity
		B0	DACPOLL,	Left DAC output polarity:
	*/
	if (_InPath & DAC_ON)
	{
		wm8978_WriteReg(10, 0);
	}
	;
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_NotchFilter
*	功能说明: 设置陷波滤波器（notch filter），主要用于抑制话筒声波正反馈，避免啸叫
*	形    参:  NFA0[13:0] and NFA1[13:0]
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_NotchFilter(uint16_t _NFA0, uint16_t _NFA1)
{
	uint16_t usReg;

	/*  page 26
		A programmable notch filter is provided. This filter has a variable centre frequency and bandwidth,
		programmable via two coefficients, a0 and a1. a0 and a1 are represented by the register bits
		NFA0[13:0] and NFA1[13:0]. Because these coefficient values require four register writes to setup
		there is an NFU (Notch Filter Update) flag which should be set only when all four registers are setup.
	*/
	usReg = (1 << 7) | (_NFA0 & 0x3F);
	wm8978_WriteReg(27, usReg);	/* 写寄存器 */

	usReg = ((_NFA0 >> 7) & 0x3F);
	wm8978_WriteReg(28, usReg);	/* 写寄存器 */

	usReg = (_NFA1 & 0x3F);
	wm8978_WriteReg(29, usReg);	/* 写寄存器 */

	usReg = (1 << 8) | ((_NFA1 >> 7) & 0x3F);
	wm8978_WriteReg(30, usReg);	/* 写寄存器 */
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_CtrlGPIO1
*	功能说明: 控制WM8978的GPIO1引脚输出0或1
*	形    参:  _ucValue ：GPIO1输出值，0或1
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_CtrlGPIO1(uint8_t _ucValue)
{
	uint16_t usRegValue;

	/* R8， pdf 62页 */
	if (_ucValue == 0) /* 输出0 */
	{
		usRegValue = 6; /* B2:0 = 110 */
	}
	else
	{
		usRegValue = 7; /* B2:0 = 111 */
	}
	wm8978_WriteReg(8, usRegValue);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Reset
*	功能说明: 复位wm8978，所有的寄存器值恢复到缺省值
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void wm8978_Reset(void)
{
	/* wm8978寄存器缺省值 */
	const uint16_t reg_default[] = {
	0x000, 0x000, 0x000, 0x000, 0x050, 0x000, 0x140, 0x000,
	0x000, 0x000, 0x000, 0x0FF, 0x0FF, 0x000, 0x100, 0x0FF,
	0x0FF, 0x000, 0x12C, 0x02C, 0x02C, 0x02C, 0x02C, 0x000,
	0x032, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x038, 0x00B, 0x032, 0x000, 0x008, 0x00C, 0x093, 0x0E9,
	0x000, 0x000, 0x000, 0x000, 0x003, 0x010, 0x010, 0x100,
	0x100, 0x002, 0x001, 0x001, 0x039, 0x039, 0x039, 0x039,
	0x001, 0x001
	};
	uint8_t i;

	wm8978_WriteReg(0x00, 0);

	for (i = 0; i < sizeof(reg_default) / 2; i++)
	{
		wm8978_RegCash[i] = reg_default[i];
	}
}

/*
*********************************************************************************************************
*	                     下面的代码是和STM32 SAI音频接口硬件相关的
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	函 数 名: I2S_CODEC_Init
*	功能说明: 配置GPIO引脚和中断通道用于codec应用
*	形    参: _ucMode : 1 表示放音，2表示录音， 3表示边录边放(暂未支持)
*	返 回 值: 无
*********************************************************************************************************
*/
void AUDIO_Init(uint8_t _ucMode, uint16_t _usStandard, uint32_t _uiWordLen, uint32_t _uiAudioFreq)
{	
	PLLSAI_Config();		/* 配置SAI的时钟源 */
			
	SAI_GPIO_Config();		/* 配置SAI相关的 GPIO口线 */
	
	Play_SAI_DMA_DeInit();	/* 禁止放音通道的DMA传输 */	
	
	Rec_SAI_DMA_DeInit();	/* 禁止录音通道的DMA传输 */
	
	
	/* 禁止SAI时钟 -> 重新使能SAI时钟 */
	RCC_APB2PeriphClockCmd(SAI_RCC, DISABLE);
	RCC_APB2PeriphClockCmd(SAI_RCC, ENABLE);
	
	SAI_Cmd(SAI_BLOCK1, DISABLE); 		
	SAI_Cmd(SAI_BLOCK2, DISABLE);  

	SAI_Mode_Config(_ucMode, _usStandard,  _uiWordLen, _uiAudioFreq);

	if (_ucMode == 1)	/* 放音 */
	{
		Play_DMA_Init();
		
		//SAI_Cmd(SAI_BLOCK1, ENABLE); 
		//SAI_SendData(SAI1_Block_B, 0x00);	
		
	}
	else if (_ucMode == 2)	/* 录音 */
	{
		Rec_DMA_Init();
		
		SAI_Cmd(SAI_BLOCK2, ENABLE); 
		SAI_ReceiveData(SAI1_Block_A);
	}
	else
	{
		Play_DMA_Init();
		Rec_DMA_Init();

		SAI_Cmd(SAI_BLOCK1, ENABLE); 		
		SAI_Cmd(SAI_BLOCK2, ENABLE); 
	}
}


/**
  * @brief  Starts playing audio stream from a data buffer for a determined size. 
  * @param  pBuffer: Pointer to the buffer 
  * @param  Size: Number of audio data BYTES.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t AUDIO_Play(uint16_t* pBuffer, uint32_t Size)
{		
	/* Update the Media layer and enable it for play */  
	{
		/* Configure the buffer address and size */
		DMA_Init_Play.DMA_Memory0BaseAddr = (uint32_t)pBuffer;
		DMA_Init_Play.DMA_BufferSize = (uint32_t)(DMA_MAX(s_tPlay.AudioTotalSize / 2));
		
		/* Configure the DMA Stream with the new parameters */
		DMA_Init(PLAY_DMA_STREAM, &DMA_Init_Play);
		
		/* Enable the SAI DMA Stream*/
		DMA_Cmd(PLAY_DMA_STREAM, ENABLE);
		
		/* If the SAI peripheral is still not enabled, enable it */
		if (SAI_GetCmdStatus(SAI_BLOCK1) == DISABLE)
		{
			SAI_Cmd(SAI_BLOCK1, ENABLE);
		} 		
	}

	/* Set the total number of data to be played (count in half-word) */
	s_tPlay.AudioTotalSize = Size/2;
	
	/* Update the remaining number of data to be played */
	s_tPlay.AudioRemSize = (Size/2) - DMA_MAX(s_tPlay.AudioTotalSize);
	
	/* Update the current audio pointer position */
	s_tPlay.CurrentPos = pBuffer + DMA_MAX(s_tPlay.AudioTotalSize);
	
	return 0;
}


/**
  * @brief  Starts playing audio stream from a data buffer for a determined size. 
  * @param  pBuffer: Pointer to the buffer 
  * @param  Size: Number of audio data BYTES.
  * @retval 0 if correct communication, else wrong communication
  */
uint32_t AUDIO_Record(uint16_t* pBuffer, uint32_t Size)
{	
	/* Update the Media layer and enable it for play */  
	{
		/* Configure the buffer address and size */
		DMA_Init_Rec.DMA_Memory0BaseAddr = (uint32_t)pBuffer;
		DMA_Init_Rec.DMA_BufferSize = (uint32_t)(DMA_MAX(s_tRec.AudioTotalSize / 2));
		
		/* Configure the DMA Stream with the new parameters */
		DMA_Init(REC_DMA_STREAM, &DMA_Init_Rec);
		
		/* Enable the SAI DMA Stream*/
		DMA_Cmd(REC_DMA_STREAM, ENABLE);
		
		/* If the SAI peripheral is still not enabled, enable it */
		if (SAI_GetCmdStatus(SAI_BLOCK2) == DISABLE)
		{
			SAI_Cmd(SAI_BLOCK2, ENABLE);
		} 		
	}

	/* Set the total number of data to be played (count in half-word) */
	s_tRec.AudioTotalSize = Size/2;	
	
	/* Update the remaining number of data to be played */
	s_tRec.AudioRemSize = (Size/2) - DMA_MAX(s_tRec.AudioTotalSize);
	
	/* Update the current audio pointer position */
	s_tRec.CurrentPos = pBuffer + DMA_MAX(s_tRec.AudioTotalSize);
	
	return 0;
}

/**
  * @brief  This function Pauses or Resumes the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used. In all cases the SAI 
  *         peripheral is disabled. 
  * 
  * @WARNING When calling EVAL_AUDIO_PauseResume() function for pause, only
  *          this function should be called for resume (use of EVAL_AUDIO_Play() 
  *          function for resume could lead to unexpected behavior).
  * 
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume. 
  * @retval 0 if correct communication, else wrong communication
  */
void AUDIO_Pause(void)
{    
	/* Disable the SAI DMA request */
	SAI_DMACmd(SAI_BLOCK1, DISABLE);
	
	/* Pause the SAI DMA Stream 
	Note. For the STM32F4xx devices, the DMA implements a pause feature, 
	      by disabling the stream, all configuration is preserved and data 
	      transfer is paused till the next enable of the stream.*/
	DMA_Cmd(PLAY_DMA_STREAM, DISABLE);
}

/**
  * @brief  This function Pauses or Resumes the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used. In all cases the SAI 
  *         peripheral is disabled. 
  * 
  * @WARNING When calling EVAL_AUDIO_PauseResume() function for pause, only
  *          this function should be called for resume (use of EVAL_AUDIO_Play() 
  *          function for resume could lead to unexpected behavior).
  * 
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume. 
  * @retval 0 if correct communication, else wrong communication
  */
void AUDIO_Resume(uint32_t Cmd)
{    
	/* Enable the I2S DMA request */
	SAI_DMACmd(SAI_BLOCK1, ENABLE);
	
	/* Resume the SAI DMA Stream 
	Note. For the STM32F4xx devices, the DMA implements a pause feature, 
	      by disabling the stream, all configuration is preserved and data 
	      transfer is paused till the next enable of the stream.*/
	DMA_Cmd(PLAY_DMA_STREAM, ENABLE);
	
	/* If the SAI peripheral is still not enabled, enable it */
	if (SAI_GetCmdStatus(SAI_BLOCK1) == DISABLE)
	{
		SAI_Cmd(SAI_BLOCK1, ENABLE);
	}   
}

/**
  * @brief  Stops audio playing and enables the MUTE mode by software. 
  * @param  Option: could be one of the following parameters 
  *           - CODEC_PDWN_SW: To enable the MUTE mode.                   
  *           - CODEC_PDWN_HW: To reset the codec by writing in 0x0000 address.
  *                            All registers will be reset to their default state. 
  *                            Then need to reconfigure the Codec after that.  
  * @note   Codec will not be physically powered-down.
  * @retval 0 if correct communication, else wrong communication
  */
void AUDIO_Stop(void)
{
//	wm8978_PowerDown();

 	{
		/* Stop the Transfer on the SAI side: Stop and disable the DMA stream */
		DMA_Cmd(PLAY_DMA_STREAM, DISABLE);
		
		/* Clear all the DMA flags for the next transfer */
		DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TC |PLAY_DMA_FLAG_HT | \
		                              PLAY_DMA_FLAG_FE | PLAY_DMA_FLAG_TE);
		
		/*  
		       The SAI DMA requests are not disabled here.
		                                                        */
		
		/* In all modes, disable the SAI peripheral */
		SAI_Cmd(SAI_BLOCK1, DISABLE);
	}
    
    /* Update the remaining data number */
    s_tPlay.AudioRemSize = s_tPlay.AudioTotalSize;    
    
}

/**
  * @brief  Controls the current audio volume level. 
  * @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for 
  *         Mute and 100 for Max volume level).
  * @retval 0 if correct communication, else wrong communication
  */
void AUDIO_SetVolume(uint8_t Volume)
{
	/* 调节音量，左右相同音量 */
	wm8978_SetEarVolume(Volume);
	wm8978_SetSpkVolume(Volume);
}


/*-----------------------------------------------------------------------------
                    Audio MAL Interface Control Functions
  ----------------------------------------------------------------------------*/

/**
  * @brief  Initializes and prepares the Media to perform audio data transfer 
  *         from Media to the SAI peripheral.
  * @param  None
  * @retval None
  */
static void Play_DMA_Init(void)  
{   
#if defined(PLAY_DMA_IT_TC_EN) || defined(PLAY_DMA_IT_HT_EN) || defined(PLAY_DMA_IT_TE_EN)
	NVIC_InitTypeDef NVIC_InitStructure;
#endif
	/* Enable the SAI  */
	SAI_Cmd(SAI_BLOCK1, DISABLE);
	
	/* Enable the DMA clock */
	RCC_AHB1PeriphClockCmd(PLAY_DMA_CLOCK, ENABLE);  // --- 不知道是否起作用
	
	/* DISABLE the SAI DMA request */
	SAI_DMACmd(SAI_BLOCK1, DISABLE);
	
	/* Configure the DMA Stream */
	DMA_Cmd(PLAY_DMA_STREAM, DISABLE);
	DMA_DeInit(PLAY_DMA_STREAM);
	/* Set the parameters to be configured */
	DMA_Init_Play.DMA_Channel = PLAY_DMA_CHANNEL;  
	DMA_Init_Play.DMA_PeripheralBaseAddr = SAI_BLOCK1_DR;
	DMA_Init_Play.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
	DMA_Init_Play.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_Init_Play.DMA_BufferSize = (uint32_t)0xFFFF;      /* This field will be configured in play function */
	DMA_Init_Play.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Init_Play.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Init_Play.DMA_PeripheralDataSize = PLAY_DMA_PERIPH_DATA_SIZE;
	DMA_Init_Play.DMA_MemoryDataSize = PLAY_DMA_MEM_DATA_SIZE; 
#ifdef AUDIO_MAL_MODE_NORMAL
	DMA_Init_Play.DMA_Mode = DMA_Mode_Normal;
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
	DMA_Init_Play.DMA_Mode = DMA_Mode_Circular;
#else
	#error "AUDIO_MAL_MODE_NORMAL or AUDIO_MAL_MODE_CIRCULAR should be selected !!"
#endif /* AUDIO_MAL_MODE_NORMAL */  
	DMA_Init_Play.DMA_Priority = DMA_Priority_High;
	DMA_Init_Play.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_Init_Play.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_Init_Play.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_Init_Play.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	DMA_Init(PLAY_DMA_STREAM, &DMA_Init_Play);  
	
/* Enable the selected DMA interrupts (selected in "stm32_eval_audio_codec.h" defines) */
#ifdef PLAY_DMA_IT_TC_EN
	DMA_ITConfig(PLAY_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif /* PLAY_DMA_IT_TC_EN */

#ifdef PLAY_DMA_IT_HT_EN
	DMA_ITConfig(PLAY_DMA_STREAM, DMA_IT_HT, ENABLE);
#endif /* PLAY_DMA_IT_HT_EN */
	
#ifdef PLAY_DMA_IT_TE_EN
	DMA_ITConfig(PLAY_DMA_STREAM, DMA_IT_TE | DMA_IT_FE | DMA_IT_DME, ENABLE);
#endif /* PLAY_DMA_IT_TE_EN */
	
	/* Enable the SAI DMA request */
	SAI_DMACmd(SAI_BLOCK1, ENABLE);
	
#if defined(PLAY_DMA_IT_TC_EN) || defined(PLAY_DMA_IT_HT_EN) || defined(PLAY_DMA_IT_TE_EN)
	/* SAI DMA IRQ Channel configuration */
	NVIC_InitStructure.NVIC_IRQChannel = PLAY_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif 
}


/**
  * @brief  Initializes and prepares the Media to perform audio data transfer 
  *         from Media to the SAI peripheral.
  * @param  None
  * @retval None
  */
static void Rec_DMA_Init(void)  
{   
#if defined(REC_DMA_IT_TC_EN) || defined(REC_DMA_IT_HT_EN) || defined(REC_DMA_IT_TE_EN)
	NVIC_InitTypeDef NVIC_InitStructure;
#endif
	/* Enable the SAI  */
	SAI_Cmd(SAI_BLOCK2, DISABLE);
	
	/* Enable the DMA clock */
	RCC_AHB1PeriphClockCmd(REC_DMA_CLOCK, ENABLE); 
	
	/* Configure the DMA Stream */
	DMA_Cmd(REC_DMA_STREAM, DISABLE);
	DMA_DeInit(REC_DMA_STREAM);
	/* Set the parameters to be configured */
	DMA_Init_Rec.DMA_Channel = REC_DMA_CHANNEL;  
	DMA_Init_Rec.DMA_PeripheralBaseAddr = SAI_BLOCK2_DR;
	DMA_Init_Rec.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
	DMA_Init_Rec.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_Init_Rec.DMA_BufferSize = (uint32_t)0xFFFF;      /* This field will be configured in play function */
	DMA_Init_Rec.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Init_Rec.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Init_Rec.DMA_PeripheralDataSize = REC_DMA_PERIPH_DATA_SIZE;
	DMA_Init_Rec.DMA_MemoryDataSize = REC_DMA_MEM_DATA_SIZE; 
#ifdef AUDIO_MAL_MODE_NORMAL
	DMA_Init_Rec.DMA_Mode = DMA_Mode_Normal;
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
	DMA_Init_Rec.DMA_Mode = DMA_Mode_Circular;
#else
	#error "AUDIO_MAL_MODE_NORMAL or AUDIO_MAL_MODE_CIRCULAR should be selected !!"
#endif /* AUDIO_MAL_MODE_NORMAL */  
	DMA_Init_Rec.DMA_Priority = DMA_Priority_High;
	DMA_Init_Rec.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_Init_Rec.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_Init_Rec.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_Init_Rec.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	DMA_Init(REC_DMA_STREAM, &DMA_Init_Rec);  
	
/* Enable the selected DMA interrupts (selected in "stm32_eval_audio_codec.h" defines) */
#ifdef REC_DMA_IT_TC_EN
	DMA_ITConfig(REC_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif /* REC_DMA_IT_TC_EN */

#ifdef REC_DMA_IT_HT_EN
	DMA_ITConfig(REC_DMA_STREAM, DMA_IT_HT, ENABLE);
#endif /* REC_DMA_IT_HT_EN */
	
#ifdef REC_DMA_IT_TE_EN
	DMA_ITConfig(REC_DMA_STREAM, DMA_IT_TE | DMA_IT_FE | DMA_IT_DME, ENABLE);
#endif /* REC_DMA_IT_TE_EN */
	
	/* Enable the SAI DMA request */
	SAI_DMACmd(SAI_BLOCK2, ENABLE);
	
#if defined(REC_DMA_IT_TC_EN) || defined(REC_DMA_IT_HT_EN) || defined(REC_DMA_IT_TE_EN)
	/* SAI DMA IRQ Channel configuration */
	NVIC_InitStructure.NVIC_IRQChannel = REC_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif 
}

/**
  * @brief  Restore default state of the used Media.
  * @param  None
  * @retval None
  */
static void Play_SAI_DMA_DeInit(void)  
{   
#if defined(PLAY_DMA_IT_TC_EN) || defined(PLAY_DMA_IT_HT_EN) || defined(PLAY_DMA_IT_TE_EN)
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	/* Deinitialize the NVIC interrupt for the SAI DMA Stream */
	NVIC_InitStructure.NVIC_IRQChannel = PLAY_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);  
#endif 
	
	/* Disable the DMA stream before the deinit */
	DMA_Cmd(PLAY_DMA_STREAM, DISABLE);
	
	/* Dinitialize the DMA Stream */
	DMA_DeInit(PLAY_DMA_STREAM);
	
	/* 
	 The DMA clock is not disabled, since it can be used by other streams 
	 */   		                                                                      
}

/**
  * @brief  Restore default state of the used Media.
  * @param  None
  * @retval None
  */
static void Rec_SAI_DMA_DeInit(void)  
{   
#if defined(REC_DMA_IT_TC_EN) || defined(REC_DMA_IT_HT_EN) || defined(REC_DMA_IT_TE_EN)
	NVIC_InitTypeDef NVIC_InitStructure;  
	
	/* Deinitialize the NVIC interrupt for the SAI DMA Stream */
	NVIC_InitStructure.NVIC_IRQChannel = PLAY_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);  
#endif 
	
	/* Disable the DMA stream before the deinit */
	DMA_Cmd(REC_DMA_STREAM, DISABLE);
	
	/* Dinitialize the DMA Stream */
	DMA_DeInit(REC_DMA_STREAM);
	
	/* 
	 The DMA clock is not disabled, since it can be used by other streams 
	*/ 
 	  		                                                                                                                                   
}

/**
  * @brief  This function handles main Media layer interrupt. 
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
void PLAY_DMA_IRQHandler(void)
{    
	#ifdef PLAY_DMA_IT_TC_EN
	/* Transfer complete interrupt --- */
		if (DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TC) != RESET)
		{     
		#ifdef AUDIO_MAL_MODE_NORMAL
			/* Check if the end of file has been reached */
			if (s_tPlay.AudioRemSize > DMA_MAX_SZE)
			{      
				/* Wait the DMA Stream to be effectively disabled */
				while (DMA_GetCmdStatus(PLAY_DMA_STREAM) != DISABLE)
				{}
				
				/* Clear the Interrupt flag */
				DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_ALL);  
				
				/* Re-Configure the buffer address and size */
				DMA_Init_Play.DMA_Memory0BaseAddr = (uint32_t) s_tPlay.CurrentPos;
				DMA_Init_Play.DMA_BufferSize = (uint32_t) (DMA_MAX(s_tPlay.AudioRemSize));
				
				/* Configure the DMA Stream with the new parameters */
				DMA_Init(PLAY_DMA_STREAM, &DMA_Init_Play);
				
				/* Enable the SAI DMA Stream*/
				DMA_Cmd(PLAY_DMA_STREAM, ENABLE);    
				
				/* Update the current pointer position */
				s_tPlay.CurrentPos += DMA_MAX(s_tPlay.AudioRemSize);        
				
				/* Update the remaining number of data to be played */
				s_tPlay.AudioRemSize -= DMA_MAX(s_tPlay.AudioRemSize);    
			}
			else
			{
				/* Disable the SAI DMA Stream*/
				DMA_Cmd(PLAY_DMA_STREAM, DISABLE);   
				
				/* Clear the Interrupt flag */
				DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TC);       
				
				/* 此处可以 bsp_PutMsg() 一个消息，通知主程序DMA传输结束 */
				//should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h)
				//EVAL_AUDIO_TransferComplete_CallBack((uint32_t)CurrentPos, 0);      
				/* 发送一个消息，DMA结束 */
				bsp_PutMsg(MSG_WM8978_DMA_END, 0);			
			}
		#elif defined(AUDIO_MAL_MODE_CIRCULAR)
			/* Manage the remaining file size and new address offset: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			//EVAL_AUDIO_TransferComplete_CallBack((uint32_t)CurrentPos, AudioRemSize); 
			/* 发送一个消息，DMA结束 */
			bsp_PutMsg(MSG_WM8978_DMA_END, 0);			
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TC);
		#endif /* AUDIO_MAL_MODE_NORMAL */  
		}
	#endif /* PLAY_DMA_IT_TC_EN */
	
	#ifdef PLAY_DMA_IT_HT_EN  
		/* Half Transfer complete interrupt */
		if (DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_HT) != RESET)
		{
			/* Manage the remaining file size and new address offset: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			//EVAL_AUDIO_HalfTransfer_CallBack((uint32_t)CurrentPos, AudioRemSize);    
			
			/* 发送一个消息，DMA结束 */
			bsp_PutMsg(MSG_WM8978_DMA_END, 0);
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_HT);    
		}
	#endif /* PLAY_DMA_IT_HT_EN */
	
	#ifdef PLAY_DMA_IT_TE_EN  
		/* FIFO Error interrupt */
		if ((DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TE) != RESET) || \
		(DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_FE) != RESET) || \
		(DMA_GetFlagStatus(PLAY_DMA_STREAM, PLAY_DMA_FLAG_DME) != RESET))
		{
			/* Manage the error generated on DMA FIFO: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			//EVAL_AUDIO_Error_CallBack((uint32_t*)&CurrentPos);    
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(PLAY_DMA_STREAM, PLAY_DMA_FLAG_TE | PLAY_DMA_FLAG_FE | \
			                        PLAY_DMA_FLAG_DME);
		}  
	#endif /* PLAY_DMA_IT_TE_EN */
}

/* 录音的DMA通道中断服务程序 */
void REC_DMA_IRQHandler(void)
{
	#ifdef REC_DMA_IT_TC_EN
	/* Transfer complete interrupt --- */
		if (DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_TC) != RESET)
		{     
		#ifdef AUDIO_MAL_MODE_NORMAL
			/* Check if the end of file has been reached */
			if (s_tRec.AudioRemSize > DMA_MAX_SZE)
			{      
				/* Wait the DMA Stream to be effectively disabled */
				while (DMA_GetCmdStatus(REC_DMA_STREAM) != DISABLE)
				{}
				
				/* Clear the Interrupt flag */
				DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_ALL);  
				
				/* Re-Configure the buffer address and size */
				DMA_Init_Rec.DMA_Memory0BaseAddr = (uint32_t) s_tRec.CurrentPos;
				DMA_Init_Rec.DMA_BufferSize = (uint32_t) (DMA_MAX(s_tRec.AudioRemSize));
				
				/* Configure the DMA Stream with the new parameters */
				DMA_Init(REC_DMA_STREAM, &DMA_Init_Rec);
				
				/* Enable the SAI DMA Stream*/
				DMA_Cmd(REC_DMA_STREAM, ENABLE);    
				
				/* Update the current pointer position */
				s_tRec.CurrentPos += DMA_MAX(s_tRec.AudioRemSize);        
				
				/* Update the remaining number of data to be record */
				s_tRec.AudioRemSize -= DMA_MAX(s_tRec.AudioRemSize);    
			}
			else
			{
				/* Disable the SAI DMA Stream*/
				DMA_Cmd(REC_DMA_STREAM, DISABLE);   
				
				/* Clear the Interrupt flag */
				DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_TC);       
				
				/* Manage the remaining file size and new address offset: This function 
				should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
				/* 发送一个消息，DMA结束 */
				bsp_PutMsg(MSG_WM8978_DMA_END, 0);
			}
		#elif defined(AUDIO_MAL_MODE_CIRCULAR)
			/* Manage the remaining file size and new address offset: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			//EVAL_AUDIO_TransferComplete_CallBack((uint32_t)s_tRec.CurrentPos, s_tRec.AudioRemSize);    
			/* 发送一个消息，DMA结束 */
			bsp_PutMsg(MSG_WM8978_DMA_END, 0);
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_TC);
		#endif /* AUDIO_MAL_MODE_NORMAL */  
		}
	#endif /* REC_DMA_IT_TC_EN */
	
	#ifdef REC_DMA_IT_HT_EN  
		/* Half Transfer complete interrupt */
		if (DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_HT) != RESET)
		{
			/* Manage the remaining file size and new address offset: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			EVAL_AUDIO_HalfTransfer_CallBack((uint32_t)s_tRec.CurrentPos, s_tRec.AudioRemSize);    
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_HT);    
		}
	#endif /* REC_DMA_IT_HT_EN */
	
	#ifdef REC_DMA_IT_TE_EN  
		/* FIFO Error interrupt */
		if ((DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_TE) != RESET) || \
		(DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_FE) != RESET) || \
		(DMA_GetFlagStatus(REC_DMA_STREAM, REC_DMA_FLAG_DME) != RESET))
		{
			/* Manage the error generated on DMA FIFO: This function 
			should be coded by user (its prototype is already declared in stm32_eval_audio_codec.h) */  
			EVAL_AUDIO_Error_CallBack((uint32_t*)&s_tRec.CurrentPos);    
			
			/* Clear the Interrupt flag */
			DMA_ClearFlag(REC_DMA_STREAM, REC_DMA_FLAG_TE | REC_DMA_FLAG_FE | \
			                        REC_DMA_FLAG_DME);
		}  
	#endif /* REC_DMA_IT_TE_EN */
}


/*
*********************************************************************************************************
*	函 数 名: SAI_GPIO_Config
*	功能说明: 配置GPIO引脚用于SAI codec应用
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void SAI_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*
		安富莱STM32-V6开发板---  SAI接口 I2S总线传输音频数据口线	
			PF9/SAI1_FS_B
			PF8/SAI1_SCK_B
			PD6/SAI1_SD_A
			PF6/SAI1_SD_B
			PF7/SAI1_MCLK_B	
	*/
	
	/* 使能SAI音频接口的GPIO时钟 */
	RCC_AHB1PeriphClockCmd(SAI_GPIO_RCC, ENABLE);
	
	/* 配置 FS, SCK, MCK, SD1， SD2 */
	GPIO_PinAFConfig(SAI_GPIO_FS, SAI_PINSRC_FS, SAI_GPIO_AF); 
	GPIO_PinAFConfig(SAI_GPIO_SCK, SAI_PINSRC_SCK, SAI_GPIO_AF);  
	GPIO_PinAFConfig(SAI_GPIO_MCK, SAI_PINSRC_MCK, SAI_GPIO_AF); 
	GPIO_PinAFConfig(SAI_GPIO_SD1, SAI_PINSRC_SD1, SAI_GPIO_AF); 
	GPIO_PinAFConfig(SAI_GPIO_SD2, SAI_PINSRC_SD2, SAI_GPIO_AF);

	/* Configure pins as AF pushpull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Pin = SAI_PIN_MCK;	
	GPIO_Init(SAI_GPIO_MCK, &GPIO_InitStructure); 
		
	
	/* Configure pins as AF pushpull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	
	
	GPIO_InitStructure.GPIO_Pin = SAI_PIN_FS;	
	GPIO_Init(SAI_GPIO_FS, &GPIO_InitStructure); 	

	GPIO_InitStructure.GPIO_Pin = SAI_PIN_SCK;	
	GPIO_Init(SAI_GPIO_SCK, &GPIO_InitStructure); 
	

	GPIO_InitStructure.GPIO_Pin = SAI_PIN_SD1;	
	GPIO_Init(SAI_GPIO_SD1, &GPIO_InitStructure); 	

	/* Configure pins as AF pushpull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = SAI_PIN_SD2;	
	GPIO_Init(SAI_GPIO_SD2, &GPIO_InitStructure); 	
}

/*
*********************************************************************************************************
*	函 数 名: SAI_Mode_Config
*	功能说明: 配置STM32的SAI音频接口的工作模式。 放音
*	形    参:  _mode : 1表示录音, 2表示放音, 3表示录音放音同时（保留未实现）
*			  _usStandard :  接口标准 I2S_Standard_Phillips,  I2S_Standard_MSB  或  I2S_Standard_LSB
*			  _uiWordLen : 样本字长，可选的参数: SAI_DataSize_8b,SAI_DataSize_10b 
*						SAI_DataSize_16b, SAI_DataSize_20b, SAI_DataSize_24b, SAI_DataSize_32b
*					目前仅调试支持16bit。
*				WM8978支持    16、20、24、32bit；
*				STM32F492支持 8、10、16、20、24、31bit
*							
*			  _usAudioFreq : 采样频率，I2S_AudioFreq_8K、I2S_AudioFreq_16K、I2S_AudioFreq_22K、
*							I2S_AudioFreq_44K、I2S_AudioFreq_48
*	返 回 值: 无
*********************************************************************************************************
*/
static void SAI_Mode_Config(uint8_t _mode, uint16_t _usStandard, uint32_t _uiWordLen, uint32_t _uiAudioFreq)
{
	uint32_t tmpdiv;
	uint32_t SAI_ClockSrcFreq;

	SAI_InitTypeDef       SAI_InitStructure;
	SAI_FrameInitTypeDef  SAI_FrameInitStructure;
	SAI_SlotInitTypeDef   SAI_SlotInitStructure;

	/* 
		配置MCK时钟输出的频率 
		
		MCLK_x = SAI_CK_x / (MCKDIV[3:0] * 2) with MCLK_x = 256 * FS
		
		FS = SAI_CK_x / (MCKDIV[3:0] * 2) * 256
		MCKDIV[3:0] = SAI_CK_x / FS * 512 	
	
		根据采样频率选择时钟源, 这样分频后可以得到准确的位时钟 
		#define SAI_ClockPLLSAI             ((uint32_t)11289600)
		#define SAI_ClockPLLI2S             ((uint32_t)49152000)

	*/
	if (_uiAudioFreq == SAI_AudioFreq_44_1k || _uiAudioFreq == SAI_AudioFreq_22_05k
		|| _uiAudioFreq == SAI_AudioFreq_11_025k)
	{
		SAI_ClockSrcFreq = SAI_ClockPLLSAI;
		
		/* 配置SAI_Block_A和 SAI_Block_B的时钟源 */
		RCC_SAIBlockACLKConfig(RCC_SAIACLKSource_PLLSAI);
		RCC_SAIBlockBCLKConfig(RCC_SAIACLKSource_PLLSAI);		
	}
	else	/*192k, 96k, 48k, 32k, 16k, 8k */
	{
		if (_uiAudioFreq ==	SAI_AudioFreq_8k)
		{
			RCC_PLLI2SConfig(344, 14, 4);	// 49152000 / 2;
			SAI_ClockSrcFreq = 49152000 / 2;
		}
		else
		{
			RCC_PLLI2SConfig(344, 7, 4);	// 49152000
			SAI_ClockSrcFreq = 49152000;
		}	

		/* 配置SAI_Block_A和 SAI_Block_B的时钟源 */
		RCC_SAIBlockACLKConfig(RCC_SAIBCLKSource_PLLI2S);
		RCC_SAIBlockBCLKConfig(RCC_SAIBCLKSource_PLLI2S);		
	}
	
	tmpdiv = SAI_ClockSrcFreq / (_uiAudioFreq * 256);
  
	SAI_InitStructure.SAI_NoDivider = SAI_MasterDivider_Enabled;
	SAI_InitStructure.SAI_MasterDivider = tmpdiv;

	/* 
		[内部同步]
		1. 音频模块可声明为与第二个音频模块同步。在这种情况下，将共用位时钟和帧同步信号，以
		   减少通信时占用外部引脚的数量。
		2. 声明为与另一个模块同步的音频模块将释放其 SCK_x、FS_x 和 MCLK_x 引脚以用作 GPIO。
		3. 声明为异步的模块将使用其 I/O 引脚 FS_x、 SCK_x 和 MCLK_x（如果该音频模块被视为主模块）。
			
		通常，音频模块同步模式可用于在全双工模式下配置 SAI。两个音频模块的其中一个可配置为主模块，另一个为从模块；
		也可将二个均配置为从模块；一个模块声明为异步 SAI_xCR1 中的相应位 SYNCEN[1:0] = 00），
		  另一个声明为同步（ SAI_xCR1 中的相应位SYNCEN[1:0] = 01）。
		  
		注意： 由于存在内部重新同步阶段， APB 频率 PCLK 必须大于或等于比特率时钟频率的二倍。
		
		必须在使能主模式前使能从模式。
	*/	
	if (_mode == 1)	/* 放音 */
	{
		/* 配置音频子模块1 --- 放音的 ， 两个子模块同步模式工作 */
		SAI_InitStructure.SAI_AudioMode = SAI_Mode_MasterTx;	/* 配置为主模式发送 */
		SAI_InitStructure.SAI_Protocol = SAI_Free_Protocol;		/* 协议 */
		SAI_InitStructure.SAI_DataSize = _uiWordLen;			/* 样本字长 */
		SAI_InitStructure.SAI_FirstBit = SAI_FirstBit_MSB;		/* bit次序，高bit先传 */
		SAI_InitStructure.SAI_ClockStrobing = SAI_ClockStrobing_RisingEdge;
		SAI_InitStructure.SAI_Synchro = SAI_Asynchronous;		/* 申明为异步，使用本模块的 FS, SCK,MCLK */
		SAI_InitStructure.SAI_OUTDRIV = SAI_OutputDrive_Enabled;
		SAI_InitStructure.SAI_FIFOThreshold = SAI_FIFOThreshold_1QuarterFull;
		SAI_Init(SAI_BLOCK1, &SAI_InitStructure);
	
	
		/* Configure SAI_Block_x Frame 
			Frame Length : 32
			Frame active Length: 16
			FS Definition : Start frame + Channel Side identification
			FS Polarity: FS active Low
			FS Offset: FS asserted one bit before the first bit of slot 0 */ 
		/*
			则帧长度应为 8 到 256 之间的一个等于 2
			的 n 次幂的数。这是为了确保音频帧的每个位时钟包含整数个 MCLK 脉冲，这样可确保解码器内的外部 DAC/ADC 正确工作。	
			
			
		*/
		SAI_FrameInitStructure.SAI_FrameLength = 32;		/* ST板子是64， V6是32 */
		SAI_FrameInitStructure.SAI_ActiveFrameLength = 16;  
		SAI_FrameInitStructure.SAI_FSDefinition = I2S_FS_ChannelIdentification;	 /* FS定义为左右声道 */
		SAI_FrameInitStructure.SAI_FSPolarity = SAI_FS_ActiveLow;  
		SAI_FrameInitStructure.SAI_FSOffset = SAI_FS_BeforeFirstBit;
		SAI_FrameInit(SAI_BLOCK1, &SAI_FrameInitStructure);
	
		/* 配置 SAI Block_x Slot 
		Slot First Bit Offset : 0
		Slot Size   : 16
		Slot Number : 2     <---- ST官方板子用的4个Slot，耳机和扬声器各2个左右声道。 安富莱V6板子WM8978就2个Slot表示左右声道。
		Slot Active : All slot actives 
		*/
		SAI_SlotInitStructure.SAI_FirstBitOffset = 0;
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_16b; 
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_DataSize;
		SAI_SlotInitStructure.SAI_SlotNumber = 2;		/* 2个声道, 每个声道一个 Slot */
		SAI_SlotInitStructure.SAI_SlotActive =  SAI_SlotActive_0;
		SAI_SlotInit(SAI_BLOCK1, &SAI_SlotInitStructure); 
	
		SAI_FlushFIFO(SAI_BLOCK1);
	}
	else if (_mode == 2)	/* 录音 */
	{
		/* 配置音频子模块1 --- 放音的 ， 两个子模块同步模式工作 */
		SAI_InitStructure.SAI_AudioMode = SAI_Mode_MasterRx;	/* 配置为主模式接收 */
		SAI_InitStructure.SAI_Protocol = SAI_Free_Protocol;		/* 协议 */
		SAI_InitStructure.SAI_DataSize = _uiWordLen;			/* 样本字长 */
		SAI_InitStructure.SAI_FirstBit = SAI_FirstBit_MSB;		/* bit次序，高bit先传 */
		SAI_InitStructure.SAI_ClockStrobing = SAI_ClockStrobing_RisingEdge;
		SAI_InitStructure.SAI_Synchro = SAI_Asynchronous;		/* 申明为异步，使用本模块的 FS, SCK,MCLK */
		SAI_InitStructure.SAI_OUTDRIV = SAI_OutputDrive_Enabled;
		SAI_InitStructure.SAI_FIFOThreshold = SAI_FIFOThreshold_1QuarterFull;
		SAI_Init(SAI_BLOCK2, &SAI_InitStructure);
	
	
		/* Configure SAI_Block_x Frame 
			Frame Length : 32
			Frame active Length: 16
			FS Definition : Start frame + Channel Side identification
			FS Polarity: FS active Low
			FS Offset: FS asserted one bit before the first bit of slot 0 */ 
		/*
			则帧长度应为 8 到 256 之间的一个等于 2
			的 n 次幂的数。这是为了确保音频帧的每个位时钟包含整数个 MCLK 脉冲，这样可确保解码器内的外部 DAC/ADC 正确工作。	
			
			
		*/
		SAI_FrameInitStructure.SAI_FrameLength = 32;		/* ST板子是64， V6是32 */
		SAI_FrameInitStructure.SAI_ActiveFrameLength = 16;  
		SAI_FrameInitStructure.SAI_FSDefinition = I2S_FS_ChannelIdentification;	 /* FS定义为左右声道 */
		SAI_FrameInitStructure.SAI_FSPolarity = SAI_FS_ActiveLow;  
		SAI_FrameInitStructure.SAI_FSOffset = SAI_FS_BeforeFirstBit;
		SAI_FrameInit(SAI_BLOCK2, &SAI_FrameInitStructure);
	
		/* 配置 SAI Block_x Slot 
		Slot First Bit Offset : 0
		Slot Size   : 16
		Slot Number : 2     <---- ST官方板子用的4个Slot，耳机和扬声器各2个左右声道。 安富莱V6板子WM8978就2个Slot表示左右声道。
		Slot Active : All slot actives 
		*/
		SAI_SlotInitStructure.SAI_FirstBitOffset = 0;
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_16b; 
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_DataSize;
		SAI_SlotInitStructure.SAI_SlotNumber = 2;		/* 2个声道, 每个声道一个 Slot */
		SAI_SlotInitStructure.SAI_SlotActive =  SAI_SlotActive_0;
		SAI_SlotInit(SAI_BLOCK2, &SAI_SlotInitStructure); 
	
		SAI_FlushFIFO(SAI_BLOCK2);		
	}
	else if (_mode == 3)	/* 录音和放音同时 */
	{
		/* 配置音频子模块2 --- 录音的 */
		SAI_InitStructure.SAI_AudioMode = SAI_Mode_SlaveRx;		/* 配置为从模式接收 */
		SAI_InitStructure.SAI_Protocol = SAI_Free_Protocol;		/* 协议 */
		SAI_InitStructure.SAI_DataSize = _uiWordLen;		/* 字长参数，注意 SAI_DataSize_16b 不是 16 */
		SAI_InitStructure.SAI_FirstBit = SAI_FirstBit_MSB;		/* 标准I2S格式，都是高bit先传输 */
		SAI_InitStructure.SAI_ClockStrobing = SAI_ClockStrobing_RisingEdge;
		SAI_InitStructure.SAI_Synchro = SAI_Synchronous;		/* 选同步模式, */
		SAI_InitStructure.SAI_OUTDRIV = SAI_OutputDrive_Disabled;
		SAI_InitStructure.SAI_FIFOThreshold = SAI_FIFOThreshold_1QuarterFull;
		SAI_Init(SAI_BLOCK2, &SAI_InitStructure);		
	
		/* 配置音频子模块1 --- 放音的 ， 两个子模块同步模式工作 */
		SAI_InitStructure.SAI_AudioMode = SAI_Mode_MasterTx;	/* 配置为主模式发送 */
		SAI_InitStructure.SAI_Protocol = SAI_Free_Protocol;		/* 协议 */
		SAI_InitStructure.SAI_DataSize = _uiWordLen;			/* 样本字长 */
		SAI_InitStructure.SAI_FirstBit = SAI_FirstBit_MSB;		/* bit次序，高bit先传 */
		SAI_InitStructure.SAI_ClockStrobing = SAI_ClockStrobing_RisingEdge;
		SAI_InitStructure.SAI_Synchro = SAI_Asynchronous;		/* 申明为异步，使用本模块的 FS, SCK,MCLK */
		SAI_InitStructure.SAI_OUTDRIV = SAI_OutputDrive_Enabled;
		SAI_InitStructure.SAI_FIFOThreshold = SAI_FIFOThreshold_1QuarterFull;
		SAI_Init(SAI_BLOCK1, &SAI_InitStructure);
	
	
		/* Configure SAI_Block_x Frame 
			Frame Length : 32
			Frame active Length: 16
			FS Definition : Start frame + Channel Side identification
			FS Polarity: FS active Low
			FS Offset: FS asserted one bit before the first bit of slot 0 */ 
		/*
			则帧长度应为 8 到 256 之间的一个等于 2
			的 n 次幂的数。这是为了确保音频帧的每个位时钟包含整数个 MCLK 脉冲，这样可确保解码器内的外部 DAC/ADC 正确工作。	
			
			
		*/
		SAI_FrameInitStructure.SAI_FrameLength = 32;		/* ST板子是64， V6是32 */
		SAI_FrameInitStructure.SAI_ActiveFrameLength = 16;  
		SAI_FrameInitStructure.SAI_FSDefinition = I2S_FS_ChannelIdentification;	 /* FS定义为左右声道 */
		SAI_FrameInitStructure.SAI_FSPolarity = SAI_FS_ActiveLow;  
		SAI_FrameInitStructure.SAI_FSOffset = SAI_FS_BeforeFirstBit;
		SAI_FrameInit(SAI_BLOCK1, &SAI_FrameInitStructure);
		
		SAI_FrameInit(SAI_BLOCK2, &SAI_FrameInitStructure);
	
		/* 配置 SAI Block_x Slot 
		Slot First Bit Offset : 0
		Slot Size   : 16
		Slot Number : 2     <---- ST官方板子用的4个Slot，耳机和扬声器各2个左右声道。 安富莱V6板子WM8978就2个Slot表示左右声道。
		Slot Active : All slot actives 
		*/
		SAI_SlotInitStructure.SAI_FirstBitOffset = 0;
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_16b; 
		SAI_SlotInitStructure.SAI_SlotSize = SAI_SlotSize_DataSize;
		SAI_SlotInitStructure.SAI_SlotNumber = 2;		/* 2个声道, 每个声道一个 Slot */
		SAI_SlotInitStructure.SAI_SlotActive =  SAI_SlotActive_0;
		SAI_SlotInit(SAI_BLOCK1, &SAI_SlotInitStructure); 
		
		SAI_SlotInit(SAI_BLOCK2, &SAI_SlotInitStructure); 
	
		SAI_FlushFIFO(SAI_BLOCK1);
		SAI_FlushFIFO(SAI_BLOCK2);
	}
}

/**
  * @brief  Configure PLLSAI. 启动两种时钟源，应对常用的音频采样频率.
  * @param  None
  * @retval None
  */

/*
*********************************************************************************************************
*	函 数 名: PLLSAI_Config
*	功能说明: 配置SAI时钟源。选择两个时钟源，以因对常见的音频采样频率.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PLLSAI_Config(void)
{ 
	/* Configure PLLSAI prescalers */
	/* PLLSAI_VCO : VCO_429M */
	/* SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 429/2 = 214.5 Mhz */
	RCC_PLLSAIConfig(429, 2, 4);

	/* SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 214.5/19 = 11.289 Mhz */  
	RCC_SAIPLLSAIClkDivConfig(19);

	/* Configure PLLI2S prescalers */
	/* PLLI2S_VCO : VCO_344M */
	/* SAI_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 344/7 = 49.142 Mhz */
	RCC_PLLI2SConfig(344, 7, 4);

	/* SAI_CLK_x = SAI_CLK(first level)/PLLI2SDIVQ = 49.142/1 = 49.142 Mhz */  
	RCC_SAIPLLI2SClkDivConfig(1);

	/* Configure Clock source for SAI Block A */
	RCC_SAIBlockACLKConfig(RCC_SAIACLKSource_PLLSAI);

	/* Configure Clock source for SAI Block B */
	RCC_SAIBlockBCLKConfig(RCC_SAIBCLKSource_PLLI2S);

	/* Enable PLLSAI Clock */
	RCC_PLLSAICmd(ENABLE);

	/* Wait till PLLSAI is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET) 
	{
	}

	/* Enable PLLI2S Clock */
	RCC_PLLI2SCmd(ENABLE);

	/* Wait till PLLI2S is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY) == RESET) 
	{
	}
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
