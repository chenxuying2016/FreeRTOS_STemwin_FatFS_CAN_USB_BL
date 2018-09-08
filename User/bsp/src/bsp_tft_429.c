/*
*********************************************************************************************************
*
*	模块名称 : STM32F429内部LCD驱动程序
*	文件名称 : bsp_tft_429.c
*	版    本 : V1.0
*	说    明 : STM32F429 内部LCD接口的硬件配置程序。
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2014-05-05 armfly 增加 STM32F429 内部LCD接口； 基于ST的例子更改，不要背景层和前景层定义，直接
*							      用 LTDC_Layer1 、 LTDC_Layer2, 这是2个结构体指针
*		V1.1	2015-11-19 armfly 
*						1. 绘图函数替换为DMA2D硬件驱动，提高绘图效率
*						2. 统一多种面板的配置函数，自动识别面板类型
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"
#include "fonts.h"

//typedef uint32_t LCD_COLOR;

#define LCD429_FRAME_BUFFER       EXT_SDRAM_ADDR

/* 偏移地址计算公式::
   Maximum width x Maximum Length x Maximum Pixel size (ARGB8888) in bytes
   => 640 x 480 x 4 =  1228800 or 0x12C000 */
#define BUFFER_OFFSET          (uint32_t)(g_LcdHeight * g_LcdWidth * 2)

uint32_t s_CurrentFrameBuffer;
uint8_t s_CurrentLayer;

static void LCD429_AF_GPIOConfig(void);
static void LCD429_ConfigLTDC(void);

void LCD429_LayerInit(void);

//static void LCD429_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth);
//static void LCD429_QuitWinMode(void);
void LCD429_SetPixelFormat(uint32_t PixelFormat);

static void _DMA_Copy(void * pSrc, void * pDst, int xSize, int ySize, int OffLineSrc, int OffLineDst);

/*
*********************************************************************************************************
*	函 数 名: LCD429_InitHard
*	功能说明: 初始化LCD
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_InitHard(void)
{
	LCD429_ConfigLTDC();			/* 配置429 CPU内部LTDC */

	LCD429_SetLayer(LCD_LAYER_1);	/* 切换到前景层 */
	LCD429_SetPixelFormat(LTDC_Pixelformat_RGB565);
	LCD429_ClrScr(CL_BLACK);		/* 清屏，显示全黑 */	
	LTDC_LayerCmd(LTDC_Layer1, ENABLE);
	

	LTDC_LayerCmd(LTDC_Layer2, ENABLE);	/* 仅用单层双缓冲区。 双层驱动以后再做 */
	LCD429_SetLayer(LCD_LAYER_2);	/* 切换到背景层 */
	LCD429_SetPixelFormat(LTDC_Pixelformat_RGB565);
	
	/* Enable The LCD */
	LTDC_Cmd(ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_ConfigLTDC
*	功能说明: 配置LTDC
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD429_ConfigLTDC(void)
{
	LTDC_InitTypeDef       LTDC_InitStruct;
	LTDC_Layer_TypeDef     LTDC_Layerx;
	uint16_t Width, Height, HSYNC_W, HBP, HFP, VSYNC_W, VBP, VFP;

	/* Enable the LTDC Clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_LTDC, ENABLE);

	/* Enable the DMA2D Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2D, ENABLE);

	/* Configure the LCD Control pins */
	LCD429_AF_GPIOConfig();

	/* Configure the FMC Parallel interface : SDRAM is used as Frame Buffer for LCD */
	//SDRAM_Init();
	bsp_InitExtSDRAM();

	/* 配置信号极性 */	
	LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;	/* HSYNC 低电平有效 */
	/* Initialize the vertical synchronization polarity as active low */
	LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;	/* VSYNC 低电平有效 */
	/* Initialize the data enable polarity as active low */
	LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;	/* DE 低电平有效 */
	/* Initialize the pixel clock polarity as input pixel clock */
	LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;
	//LTDC_InitStruct.LTDC_PCPolarity = LTDC_GCR_PCPOL;		// inverted input pixel clock.

	/* Configure R,G,B component values for LCD background color */
	LTDC_InitStruct.LTDC_BackgroundRedValue = 0;
	LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;
	LTDC_InitStruct.LTDC_BackgroundBlueValue = 0;
	
	/* 配置 PLLSAI 用于LCD */
	/* Enable Pixel Clock */

	/* 输入时钟 PLLSAI_VCO Input   = HSE_VALUE / PLL_M = 8M / 4 = 2 Mhz */
	/* 输出时钟 PLLSAI_VCO Output  = PLLSAI_VCO Input * PLLSAI_N =   2 * 429 = 858 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output / PLLSAI_R = 858 / 4 = 214.5 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 214.5 / 8 = 24 Mhz */

	/*

		RCC->PLLSAICFGR = (PLLSAIN << 6) | (PLLSAIQ << 24) | (PLLSAIR << 28);


		This register is used to configure the PLLSAI clock outputs according to the formulas:
		
		f(VCO clock) = f(PLLSAI clock input) × (PLLSAIN / PLLM)
		
		f(PLLSAI1 clock output) = f(VCO clock) / PLLSAIQ
		f(PLL LCD clock output) = f(VCO clock) / PLLSAIR				
	*/
	
	/* 支持6种面板 */
//	switch (g_LcdType)
	{
//		case LCD_35_480X320:	/* 3.5寸 480 * 320 */	
//			RCC_PLLSAIConfig(429, 2,  4);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);
//		
//			Width = 480;
//			Height = 272;
//			HSYNC_W = 10;
//			HBP = 20;
//			HFP = 20;
//			VSYNC_W = 20;
//			VBP = 20;
//			VFP = 20;
//			break;
//		
//		case LCD_43_480X272:		/* 4.3寸 480 * 272 */
//			RCC_PLLSAIConfig(429, 2,  6);		/* 频率高了后会抖屏 */
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);		

//			Width = 480;
//			Height = 272;

//			HSYNC_W = 40;
//			HBP = 2;
//			HFP = 2;
//			VSYNC_W = 9;
//			VBP = 2;
//			VFP = 2;
//			break;
//		
//		case LCD_50_480X272:		/* 5.0寸 480 * 272 */
//			RCC_PLLSAIConfig(429, 2,  4);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);
//		
//			Width = 480;
//			Height = 272;
//		
//			HSYNC_W = 40;
//			HBP = 2;
//			HFP = 2;
//			VSYNC_W = 9;
//			VBP = 2;
//			VFP = 2;			
//			break;
//		
//		case LCD_50_800X480:		/* 5.0寸 800 * 480 */
			RCC_PLLSAIConfig(429, 2,  6);
			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div4);

			Width = 800;
			Height = 480;

			HSYNC_W = 96;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
			HBP = 10;
			HFP = 10;
			VSYNC_W = 2;
			VBP = 10;
			VFP = 10;			
//			break;
		
//		case LCD_70_800X480:		/* 7.0寸 800 * 480 */
//			RCC_PLLSAIConfig(429, 2,  6);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div4);
//			
//			Width = 800;
//			Height = 480;

//			HSYNC_W = 90;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
//			HBP = 10;
//			HFP = 10;
//		
//			VSYNC_W = 10;
//			VBP = 10;
//			VFP = 10;				   
//			break;
//		
//		case LCD_70_1024X600:		/* 7.0寸 1024 * 600 */
//			LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;	/* HSYNC 低电平有效 */
//			/* Initialize the vertical synchronization polarity as active low */
//			LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;	/* VSYNC 低电平有效 */
//			/* Initialize the data enable polarity as active low */
//			LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;	/* DE 低电平有效 */
//			/* Initialize the pixel clock polarity as input pixel clock */
//			LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IIPC;
//		
//			/* IPS 7寸 1024*600，  像素时钟频率范围 : 57 -- 65 --- 70.5MHz 
//		
//				PLLSAI_VCO Input   = HSE_VALUE / PLL_M = 8M / 4 = 2 Mhz
//				PLLSAI_VCO Output  = PLLSAI_VCO Input * PLLSAI_N =   2 * 429 = 858 Mhz
//				PLLLCDCLK = PLLSAI_VCO Output / PLLSAI_R = 858 / 4 = 214.5 Mhz
//				LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 214.5 / 4 = 53.625 Mhz 	

//				(429, 2, 4); RCC_PLLSAIDivR_Div4 实测像素时钟 = 53.7M
//			*/
//			RCC_PLLSAIConfig(429, 2, 6);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div4);
//		
//			Width = 1024;
//			Height = 600;

//			HSYNC_W = 2;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
//			HBP = 157;
//			HFP = 160;
//		
//			VSYNC_W = 2;
//			VBP = 20;
//			VFP = 12;			
//			break;
//		
//		default:
//			RCC_PLLSAIConfig(429, 2,  4);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);

//			Width = 800;
//			Height = 480;

//			HSYNC_W = 80;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
//			HBP = 10;
//			HFP = 10;
//			VSYNC_W = 10;
//			VBP = 10;
//			VFP = 10;
//			break;
	}
	
	/* Initialize the LCD pixel width and pixel height */
	g_LcdWidth  = Width;		/* 显示屏分辨率-宽度 */
	g_LcdHeight = Height;		/* 显示屏分辨率-高度 */
	
	/* Enable PLLSAI Clock */
	RCC_PLLSAICmd(ENABLE);
	
	/* Wait for PLLSAI activation */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET);

	/* Timing configuration */
	/* Configure horizontal synchronization width */
	LTDC_InitStruct.LTDC_HorizontalSync = HSYNC_W;
	/* Configure vertical synchronization height */
	LTDC_InitStruct.LTDC_VerticalSync = VSYNC_W;
	/* Configure accumulated horizontal back porch */
	LTDC_InitStruct.LTDC_AccumulatedHBP = LTDC_InitStruct.LTDC_HorizontalSync + HBP;
	/* Configure accumulated vertical back porch */
	LTDC_InitStruct.LTDC_AccumulatedVBP = LTDC_InitStruct.LTDC_VerticalSync + VBP;
	/* Configure accumulated active width */
	LTDC_InitStruct.LTDC_AccumulatedActiveW = Width + LTDC_InitStruct.LTDC_AccumulatedHBP;
	/* Configure accumulated active height */
	LTDC_InitStruct.LTDC_AccumulatedActiveH = Height + LTDC_InitStruct.LTDC_AccumulatedVBP;
	/* Configure total width */
	LTDC_InitStruct.LTDC_TotalWidth = LTDC_InitStruct.LTDC_AccumulatedActiveW + HFP;
	/* Configure total height */
	LTDC_InitStruct.LTDC_TotalHeigh = LTDC_InitStruct.LTDC_AccumulatedActiveH + VFP;

	LTDC_Init(&LTDC_InitStruct);

	//LCD429_LayerInit();  展开此函数
	{
		LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct;

		LTDC_Layer_InitStruct.LTDC_HorizontalStart = HSYNC_W + HBP + 1;
		LTDC_Layer_InitStruct.LTDC_HorizontalStop = (Width + LTDC_Layer_InitStruct.LTDC_HorizontalStart - 1);
		LTDC_Layer_InitStruct.LTDC_VerticalStart = VSYNC_W + VBP + 1; 
		LTDC_Layer_InitStruct.LTDC_VerticalStop = (Height + LTDC_Layer_InitStruct.LTDC_VerticalStart - 1);

		/* Pixel Format configuration*/
		LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;
		/* Alpha constant (255 totally opaque) */
		LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255;
		/* Default Color configuration (configure A,R,G,B component values) */
		LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;
		LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;
		LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;
		LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;
		/* Configure blending factors */
		LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;
		LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;

		/* the length of one line of pixels in bytes + 3 then :
		Line Lenth = Active high width x number of bytes per pixel + 3
		Active high width         = LCD429_PIXEL_WIDTH
		number of bytes per pixel = 2    (pixel_format : RGB565)
		*/
		LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((Width * 2) + 3);
		/* the pitch is the increment from the start of one line of pixels to the
		start of the next line in bytes, then :
		Pitch = Active high width x number of bytes per pixel
		*/
		LTDC_Layer_InitStruct.LTDC_CFBPitch = (Height * 2);

		/* Configure the number of lines */
		LTDC_Layer_InitStruct.LTDC_CFBLineNumber = 	Width;	/*　此处需要填写宽度值?  */
		
		/* Start Address configuration : the LCD Frame buffer is defined on SDRAM */
		LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD429_FRAME_BUFFER;

		LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);

		/* Configure Layer2 */
		/* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
		LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD429_FRAME_BUFFER + BUFFER_OFFSET;

		/* Configure blending factors */
		LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_PAxCA;
		LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_PAxCA;

		LTDC_LayerInit(LTDC_Layer2, &LTDC_Layer_InitStruct);

		LTDC_ReloadConfig(LTDC_IMReload);

		/* Enable foreground & background Layers */
		LTDC_LayerCmd(LTDC_Layer1, ENABLE);
		LTDC_LayerCmd(LTDC_Layer2, ENABLE);
		LTDC_ReloadConfig(LTDC_IMReload);
		
		//LCD429_SetFont(&LCD429_DEFAULT_FONT);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_AF_GPIOConfig
*	功能说明: 配置GPIO用于 LTDC.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD429_AF_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* Enable GPIOI, GPIOJ, GPIOK AHB Clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOJ | \
	             RCC_AHB1Periph_GPIOK,  ENABLE);

	/* GPIOs Configuration */
	/*
	+------------------------+-----------------------+----------------------------+
	+                       LCD pins assignment                                   +
	+------------------------+-----------------------+----------------------------+
	|  LCD429_TFT R0 <-> PI.15  |  LCD429_TFT G0 <-> PJ.07 |  LCD429_TFT B0 <-> PJ.12      |
	|  LCD429_TFT R1 <-> PJ.00  |  LCD429_TFT G1 <-> PJ.08 |  LCD429_TFT B1 <-> PJ.13      |
	|  LCD429_TFT R2 <-> PJ.01  |  LCD429_TFT G2 <-> PJ.09 |  LCD429_TFT B2 <-> PJ.14      |
	|  LCD429_TFT R3 <-> PJ.02  |  LCD429_TFT G3 <-> PJ.10 |  LCD429_TFT B3 <-> PJ.15      |
	|  LCD429_TFT R4 <-> PJ.03  |  LCD429_TFT G4 <-> PJ.11 |  LCD429_TFT B4 <-> PK.03      |
	|  LCD429_TFT R5 <-> PJ.04  |  LCD429_TFT G5 <-> PK.00 |  LCD429_TFT B5 <-> PK.04      |
	|  LCD429_TFT R6 <-> PJ.05  |  LCD429_TFT G6 <-> PK.01 |  LCD429_TFT B6 <-> PK.05      |
	|  LCD429_TFT R7 <-> PJ.06  |  LCD429_TFT G7 <-> PK.02 |  LCD429_TFT B7 <-> PK.06      |
	-------------------------------------------------------------------------------
	|  LCD429_TFT HSYNC <-> PI.12  | LCDTFT VSYNC <->  PI.13 |
	|  LCD429_TFT CLK   <-> PI.14  | LCD429_TFT DE   <->  PK.07 |
	-----------------------------------------------------
	*/

	/* GPIOI configuration */
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource12, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource13, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource14, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOI, GPIO_PinSource15, GPIO_AF_LTDC);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	//GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOI, &GPIO_InitStruct);
	
	/* GPIOJ configuration */
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource0, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource1, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource2, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource3, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource4, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource5, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource6, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource7, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource8, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource9, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource10, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource11, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource12, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource13, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource14, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOJ, GPIO_PinSource15, GPIO_AF_LTDC);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | \
	                 GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | \
	                 GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | \
	                 GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOJ, &GPIO_InitStruct);

	/* GPIOI configuration */
	GPIO_PinAFConfig(GPIOK, GPIO_PinSource0, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOK, GPIO_PinSource1, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOK, GPIO_PinSource2, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOK, GPIO_PinSource3, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOK, GPIO_PinSource4, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOK, GPIO_PinSource5, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOK, GPIO_PinSource6, GPIO_AF_LTDC);
	GPIO_PinAFConfig(GPIOK, GPIO_PinSource7, GPIO_AF_LTDC);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | \
	                 GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;

	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOK, &GPIO_InitStruct);
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_GetChipDescribe
*	功能说明: 读取LCD驱动芯片的描述符号，用于显示
*	形    参: char *_str : 描述符字符串填入此缓冲区
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_GetChipDescribe(char *_str)
{
	strcpy(_str, "STM32F429");
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_SetLayer
*	功能说明: 切换层。只是更改程序变量，以便于后面的代码更改相关寄存器。硬件支持2层。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetLayer(uint8_t _ucLayer)
{
	if (_ucLayer == LCD_LAYER_1)
	{
		s_CurrentFrameBuffer = LCD429_FRAME_BUFFER;
		s_CurrentLayer = LCD_LAYER_1;
	}
	else if (_ucLayer == LCD_LAYER_2)
	{
		s_CurrentFrameBuffer = LCD429_FRAME_BUFFER + BUFFER_OFFSET;
		s_CurrentLayer = LCD_LAYER_2;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_SetTransparency
*	功能说明: 配置当前层的透明属性
*	形    参: 透明度， 值域： 0x00 - 0xFF
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetTransparency(uint8_t transparency)
{
	if (s_CurrentLayer == LCD_LAYER_1)
	{
		LTDC_LayerAlpha(LTDC_Layer1, transparency);
	}
	else
	{
		LTDC_LayerAlpha(LTDC_Layer2, transparency);
	}
	LTDC_ReloadConfig(LTDC_IMReload);	/* 立即刷新 */
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_SetPixelFormat
*	功能说明: 配置当前层的像素格式
*	形    参: 像素格式:
*                      LTDC_Pixelformat_ARGB8888
*                      LTDC_Pixelformat_RGB888
*                      LTDC_Pixelformat_RGB565
*                      LTDC_Pixelformat_ARGB1555
*                      LTDC_Pixelformat_ARGB4444
*                      LTDC_Pixelformat_L8
*                      LTDC_Pixelformat_AL44
*                      LTDC_Pixelformat_AL88
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetPixelFormat(uint32_t PixelFormat)
{
	if (s_CurrentLayer == LCD_LAYER_1)
	{
		LTDC_LayerPixelFormat(LTDC_Layer1, PixelFormat);
	}
	else
	{
		LTDC_LayerPixelFormat(LTDC_Layer2, PixelFormat);
	}
	LTDC_ReloadConfig(LTDC_IMReload);
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_SetDispWin
*	功能说明: 设置显示窗口，进入窗口显示模式。
*	形    参:  
*		_usX : 水平坐标
*		_usY : 垂直坐标
*		_usHeight: 窗口高度
*		_usWidth : 窗口宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth)
{
	if (s_CurrentLayer == LCD_LAYER_1)
	{ 
		/* reconfigure the layer1 position */
		LTDC_LayerPosition(LTDC_Layer1, _usX, _usY);
		LTDC_ReloadConfig(LTDC_IMReload);
	
		/* reconfigure the layer1 size */
		LTDC_LayerSize(LTDC_Layer1, _usWidth, _usHeight);
		LTDC_ReloadConfig(LTDC_IMReload);
	}
	else
	{   
		/* reconfigure the layer2 position */
		LTDC_LayerPosition(LTDC_Layer2, _usX, _usY);
		LTDC_ReloadConfig(LTDC_IMReload); 
		
		/* reconfigure the layer2 size */
		LTDC_LayerSize(LTDC_Layer2, _usWidth, _usHeight);
		LTDC_ReloadConfig(LTDC_IMReload);
	}	
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_QuitWinMode
*	功能说明: 退出窗口显示模式，变为全屏显示模式
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_QuitWinMode(void)
{
	LCD429_SetDispWin(0, 0, g_LcdHeight, g_LcdWidth);
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_DispOn
*	功能说明: 打开显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DispOn(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_DispOff
*	功能说明: 关闭显示
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DispOff(void)
{
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_ClrScr
*	功能说明: 根据输入的颜色值清屏
*	形    参: _usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_ClrScr(uint16_t _usColor)
{
#if 1
	LCD429_FillRect(0, 0, g_LcdHeight, g_LcdWidth, _usColor);
#else
	uint16_t *index ;
	uint32_t i;

	index = (uint16_t *)s_CurrentFrameBuffer;

	for (i = 0; i < g_LcdHeight * g_LcdWidth; i++)
	{
		*index++ = _usColor;
	}
#endif	
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_PutPixel
*	功能说明: 画1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色 ( RGB = 565 格式)
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	uint16_t *p;
	uint32_t index = 0;
	
	p = (uint16_t *)s_CurrentFrameBuffer;
		
	if (g_LcdDirection == 0)		/* 横屏 */
	{
		index = (uint32_t)_usY * g_LcdWidth + _usX;
	}
	else if (g_LcdDirection == 1)	/* 横屏180°*/
	{
		index = (uint32_t)(g_LcdHeight - _usY - 1) * g_LcdWidth + (g_LcdWidth - _usX - 1);
	}
	else if (g_LcdDirection == 2)	/* 竖屏 */
	{
		index = (uint32_t)(g_LcdWidth - _usX - 1) * g_LcdHeight + _usY;
	}
	else if (g_LcdDirection == 3)	/* 竖屏180° */
	{
		index = (uint32_t)_usX * g_LcdHeight + g_LcdHeight - _usY - 1;
	}	
	
	if (index < g_LcdHeight * g_LcdWidth)
	{
		p[index] = _usColor;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_GetPixel
*	功能说明: 读取1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_usColor  : 像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
uint16_t LCD429_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;
	uint16_t *p;
	uint32_t index = 0;
	
	p = (uint16_t *)s_CurrentFrameBuffer;

	if (g_LcdDirection == 0)		/* 横屏 */
	{
		index = (uint32_t)_usY * g_LcdWidth + _usX;
	}
	else if (g_LcdDirection == 1)	/* 横屏180°*/
	{
		index = (uint32_t)(g_LcdHeight - _usY - 1) * g_LcdWidth + (g_LcdWidth - _usX - 1);
	}
	else if (g_LcdDirection == 2)	/* 竖屏 */
	{
		index = (uint32_t)(g_LcdWidth - _usX - 1) * g_LcdHeight + _usY;
	}
	else if (g_LcdDirection == 3)	/* 竖屏180° */
	{
		index = (uint32_t)_usX * g_LcdHeight + g_LcdHeight - _usY - 1;
	}
	
	usRGB = p[index];

	return usRGB;
}


/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawLine
*	功能说明: 采用 Bresenham 算法，在2点间画一条直线。
*	形    参:
*			_usX1, _usY1 : 起始点坐标
*			_usX2, _usY2 : 终止点Y坐标
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* 采用 Bresenham 算法，在2点间画一条直线 */

	LCD429_PutPixel(_usX1 , _usY1 , _usColor);

	/* 如果两点重合，结束后面的动作。*/
	if ( _usX1 == _usX2 && _usY1 == _usY2 )
	{
		return;
	}

	iTag = 0 ;
	/* dx = abs ( _usX2 - _usX1 ); */
	if (_usX2 >= _usX1)
	{
		dx = _usX2 - _usX1;
	}
	else
	{
		dx = _usX1 - _usX2;
	}

	/* dy = abs ( _usY2 - _usY1 ); */
	if (_usY2 >= _usY1)
	{
		dy = _usY2 - _usY1;
	}
	else
	{
		dy = _usY1 - _usY2;
	}

	if ( dx < dy )   /*如果dy为计长方向，则交换纵横坐标。*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* 确定是增1还是减1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* 循环画点 */
	{
		if ( d < 0 )
		{
			d += inc1 ;
		}
		else
		{
			y += ty ;
			d += inc2 ;
		}
		if ( iTag )
		{
			LCD429_PutPixel ( y , x , _usColor) ;
		}
		else
		{
			LCD429_PutPixel ( x , y , _usColor) ;
		}
		x += tx ;
	}	
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawHLine
*	功能说明: 绘制一条水平线. 使用STM32F429 DMA2D硬件绘制.
*	形    参:
*			_usX1, _usY1 : 起始点坐标
*			_usLen       : 线的长度
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawHLine(uint16_t _usX, uint16_t _usY, uint16_t _usLen , uint16_t _usColor)
{
#if 0
	LCD429_FillRect(_usX, _usY, 1, _usLen, _usColor);
#else	
	uint16_t i;
	
	for (i = 0; i < _usLen; i++)
	{	
		LCD429_PutPixel(_usX + i , _usY , _usColor);
	}
#endif	
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawVLine
*	功能说明: 绘制一条垂直线。 使用STM32F429 DMA2D硬件绘制.
*	形    参:
*			_usX, _usY : 起始点坐标
*			_usLen       : 线的长度
*			_usColor     : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawVLine(uint16_t _usX , uint16_t _usY , uint16_t _usLen , uint16_t _usColor)
{
#if 0
	LCD429_FillRect(_usX, _usY, _usLen, 1, _usColor);
#else	
	uint16_t i;
	
	for (i = 0; i < _usLen; i++)
	{	
		LCD429_PutPixel(_usX, _usY + i, _usColor);
	}
#endif	
}
/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawPoints
*	功能说明: 采用 Bresenham 算法，绘制一组点，并将这些点连接起来。可用于波形显示。
*	形    参:
*			x, y     : 坐标数组
*			_usColor : 颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint16_t _usColor)
{
	uint16_t i;

	for (i = 0 ; i < _usSize - 1; i++)
	{
		LCD429_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _usColor);
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawRect
*	功能说明: 绘制水平放置的矩形。
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	/*
	 ---------------->---
	|(_usX，_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
	*/
	LCD429_DrawHLine(_usX, _usY, _usWidth, _usColor);
	LCD429_DrawVLine(_usX +_usWidth - 1, _usY, _usHeight, _usColor);
	LCD429_DrawHLine(_usX, _usY + _usHeight - 1, _usWidth, _usColor);
	LCD429_DrawVLine(_usX, _usY, _usHeight, _usColor);
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_FillRect
*	功能说明: 用一个颜色值填充一个矩形。使用STM32F429内部DMA2D硬件绘制。
*	形    参:
*			_usX,_usY: 矩形左上角的坐标
*			_usHeight : 矩形的高度
*			_usWidth  : 矩形的宽度
*			_usColor  : 颜色代码
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_FillRect(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
#if 1	/* 将库函数展开，提高执行效率 */
	uint32_t  Xaddress = 0;
	uint16_t  OutputOffset = 0;
	uint16_t  NumberOfLine = 0;
	uint16_t  PixelPerLine = 0;	

	/* 根据显示方向设置不同的参数 */
	if (g_LcdDirection == 0)		/* 横屏 */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * _usY + _usX);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 1)	/* 横屏180°*/
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * (g_LcdHeight - _usHeight - _usY) + g_LcdWidth - _usX - _usWidth);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 2)	/* 竖屏 */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * (g_LcdWidth - _usX -  _usWidth) + _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;
	}
	else if (g_LcdDirection == 3)	/* 竖屏180° */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * _usX + g_LcdHeight - _usHeight - _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;		
	}	

	/* 软件复位 DMA2D */
	RCC->AHB1RSTR |= RCC_AHB1Periph_DMA2D;
	RCC->AHB1RSTR &= ~RCC_AHB1Periph_DMA2D; 
	
	/* 配置DMA2D操作模式为 R2M (寄存器到存储器） */
	DMA2D->CR &= (uint32_t)0xFFFCE0FC;
	DMA2D->CR |= DMA2D_R2M;

	/* 配置输出的颜色模式为 RGB565 */
	DMA2D->OPFCCR &= ~(uint32_t)DMA2D_OPFCCR_CM;
	DMA2D->OPFCCR |= (DMA2D_RGB565);

	/* 设置颜色值 */   
	DMA2D->OCOLR |= _usColor;

	/* Configures the output memory address */
	DMA2D->OMAR = Xaddress;

	/* Configure  the line Offset */
	DMA2D->OOR &= ~(uint32_t)DMA2D_OOR_LO;
	DMA2D->OOR |= OutputOffset;

	/* Configure the number of line and pixel per line */
	DMA2D->NLR &= ~(DMA2D_NLR_NL | DMA2D_NLR_PL);
	DMA2D->NLR |= (NumberOfLine | (PixelPerLine << 16));
  
	/* Start Transfer */ 
    DMA2D->CR |= (uint32_t)DMA2D_CR_START;

	/* Wait for CTC Flag activation */
	while ((DMA2D->ISR & DMA2D_FLAG_TC)  == 0);
#endif

#if 0	/* 使用库函数 -- 容易理解 */
	/* 使用DMA2D硬件填充矩形 */
	DMA2D_InitTypeDef      DMA2D_InitStruct;
	uint32_t  Xaddress = 0;
	uint16_t  OutputOffset;
	uint16_t  NumberOfLine;
	uint16_t  PixelPerLine;	
		
	if (g_LcdDirection == 0)		/* 横屏 */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * _usY + _usX);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 1)	/* 横屏180°*/
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdWidth * (g_LcdHeight - _usHeight - _usY) + g_LcdWidth - _usX - _usWidth);	
		OutputOffset = g_LcdWidth - _usWidth;
		NumberOfLine = _usHeight;
		PixelPerLine = _usWidth;
	}
	else if (g_LcdDirection == 2)	/* 竖屏 */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * (g_LcdWidth - _usX -  _usWidth) + _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;
	}
	else if (g_LcdDirection == 3)	/* 竖屏180° */
	{
		Xaddress = s_CurrentFrameBuffer + 2 * (g_LcdHeight * _usX + g_LcdHeight - _usHeight - _usY);	
		OutputOffset = g_LcdHeight - _usHeight;
		NumberOfLine = _usWidth;
		PixelPerLine = _usHeight;		
	}	

	/* 配置 DMA2D */
	DMA2D_DeInit();		/* 复位 DMA2D */
	DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;       /* 传输模式: 寄存器到存储器 */
	DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;   /* 颜色模式， RGB565 */   
	
	DMA2D_InitStruct.DMA2D_OutputRed = RGB565_R2(_usColor);		/* 红色分量，5bit，高位为0 */           
	DMA2D_InitStruct.DMA2D_OutputGreen = RGB565_G2(_usColor);	/* 绿色分量，6bit，高位为0 */
	DMA2D_InitStruct.DMA2D_OutputBlue = RGB565_B2(_usColor); 	/* 蓝色分量，5bit，高位为0 */    
	
	DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;    				/* 透明参数， 对于565格式，无意义 */              
	DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;      	/* 目标内存地址 */          
	DMA2D_InitStruct.DMA2D_OutputOffset = OutputOffset; 	/* 每行间的显存像素地址差值  */               
	DMA2D_InitStruct.DMA2D_NumberOfLine = NumberOfLine;     /* 一共有几行 */        
	DMA2D_InitStruct.DMA2D_PixelPerLine = PixelPerLine;		/* 每行几个像素 */
	DMA2D_Init(&DMA2D_InitStruct); 

	/* Start Transfer */ 
	DMA2D_StartTransfer();

	/* Wait for CTC Flag activation */
	while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
	{
	}
#endif	
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素
*	形    参:
*			_usX,_usY  : 圆心的坐标
*			_usRadius  : 圆的半径
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* 当前 X 值 */
	uint32_t  CurY;		/* 当前 Y 值 */

	D = 3 - (_usRadius << 1);
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{
		LCD429_PutPixel(_usX + CurX, _usY + CurY, _usColor);
		LCD429_PutPixel(_usX + CurX, _usY - CurY, _usColor);
		LCD429_PutPixel(_usX - CurX, _usY + CurY, _usColor);
		LCD429_PutPixel(_usX - CurX, _usY - CurY, _usColor);
		LCD429_PutPixel(_usX + CurY, _usY + CurX, _usColor);
		LCD429_PutPixel(_usX + CurY, _usY - CurX, _usColor);
		LCD429_PutPixel(_usX - CurY, _usY + CurX, _usColor);
		LCD429_PutPixel(_usX - CurY, _usY - CurX, _usColor);

		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
*	形    参:  
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	uint16_t i, k, y;
	const uint16_t *p;

	p = _ptr;
	y = _usY;
	for (i = 0; i < _usHeight; i++)
	{
		for (k = 0; k < _usWidth; k++)
		{
			LCD429_PutPixel(_usX + k, y, *p++);
		}
		
		y++;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_SetDirection
*	功能说明: 设置显示屏显示方向（横屏 竖屏）
*	形    参: 显示方向代码 0 横屏正常, 1=横屏180度翻转, 2=竖屏, 3=竖屏180度翻转
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_SetDirection(uint8_t _dir)
{
	uint16_t temp;
	
	if (_dir == 0 || _dir == 1)		/* 横屏， 横屏180度 */
	{
		if (g_LcdWidth < g_LcdHeight)
		{
			temp = g_LcdWidth;
			g_LcdWidth = g_LcdHeight;
			g_LcdHeight = temp;			
		}
	}
	else if (_dir == 2 || _dir == 3)	/* 竖屏, 竖屏180°*/
	{
		if (g_LcdWidth > g_LcdHeight)
		{
			temp = g_LcdWidth;
			g_LcdWidth = g_LcdHeight;
			g_LcdHeight = temp;			
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_BeginDraw
*	功能说明: 双缓冲区工作模式。开始绘图。将当前显示缓冲区的数据完整复制到另外一个缓冲区。
*			 必须和 LCD429_EndDraw函数成对使用。 实际效果并不好。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_BeginDraw(void)
{
	uint16_t *src;
	uint16_t *dst;
		
	if (s_CurrentFrameBuffer == LCD429_FRAME_BUFFER)
	{
		src = (uint16_t *)LCD429_FRAME_BUFFER;
		dst =  (uint16_t *)(LCD429_FRAME_BUFFER + BUFFER_OFFSET);
		
		s_CurrentFrameBuffer = LCD429_FRAME_BUFFER + BUFFER_OFFSET;
	}
	else
	{
		src = (uint16_t *)(LCD429_FRAME_BUFFER + BUFFER_OFFSET);
		dst = (uint16_t *)LCD429_FRAME_BUFFER;
		
		s_CurrentFrameBuffer = LCD429_FRAME_BUFFER;
	}
	
	_DMA_Copy(src, dst, g_LcdHeight, g_LcdWidth, 0, 0);
}

/*
*********************************************************************************************************
*	函 数 名: LCD429_EndDraw
*	功能说明: APP结束了缓冲区绘图工作，切换硬件显示。
*			 必须和 LCD429_BeginDraw函数成对使用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD429_EndDraw(void)
{
	LTDC_LayerAddress(LTDC_Layer1, s_CurrentFrameBuffer);
}

/*
*********************************************************************************************************
*	函 数 名: _GetBufferSize
*	功能说明: 无
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
uint32_t _GetBufferSize(uint8_t LayerIndex)
{
	return g_LcdWidth * g_LcdHeight;
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Copy
*	功能说明: 复制图块
*	形    参: 
*			 pSrc : 源内存地址
*			 pDst : 目标内存地址
*			 xSize : 矩形x尺寸
*			 ySize : 矩形y尺寸
*			 OffLineSrc : 源图块行偏移
*			 OffLineDst : 目标图块行偏移
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_Copy(void * pSrc, void * pDst, int xSize, int ySize, int OffLineSrc, int OffLineDst) 
{
	DMA2D->CR      = 0x00000000UL | (1 << 9);  	// Control Register (Memory to memory and TCIE)
	DMA2D->FGMAR   = (uint32_t)pSrc;            // Foreground Memory Address Register (Source address)
	DMA2D->OMAR    = (uint32_t)pDst;        	// Output Memory Address Register (Destination address)
	DMA2D->FGOR    = OffLineSrc;           		// Foreground Offset Register (Source line offset)
	DMA2D->OOR     = OffLineDst;            	// Output Offset Register (Destination line offset)
	DMA2D->FGPFCCR = LTDC_Pixelformat_RGB565;           	// Foreground PFC Control Register (Defines the input pixel format)
	DMA2D->NLR     = (uint32_t)(xSize << 16) | (uint16_t)ySize; // Number of Line Register (Size configuration of area to be transfered)
	DMA2D->CR     |= 1;                               // Start operation
	//
	// Wait until transfer is done
	//
	while (DMA2D->CR & DMA2D_CR_START)
	{
		//__WFI();                                        // Sleep until next interrupt
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
*********************************************************************************************************
*	                                 下面的函数被emWin所调用
*********************************************************************************************************
*/
/*
*********************************************************************************************************
*	函 数 名: LCD_ConfigLTDC
*	功能说明: 配置LTDC
*	形    参: 无
*	返 回 值: 无
*   笔    记:
*       LCD_TFT 同步时序配置（整理自官方做的一个截图，言简意赅）：
*       ----------------------------------------------------------------------------
*    
*                                                 Total Width
*                             <--------------------------------------------------->
*                       Hsync width HBP             Active Width                HFP
*                             <---><--><--------------------------------------><-->
*                         ____    ____|_______________________________________|____ 
*                             |___|   |                                       |    |
*                                     |                                       |    |
*                         __|         |                                       |    |
*            /|\    /|\  |            |                                       |    |
*             | VSYNC|   |            |                                       |    |
*             |Width\|/  |__          |                                       |    |
*             |     /|\     |         |                                       |    |
*             |  VBP |      |         |                                       |    |
*             |     \|/_____|_________|_______________________________________|    |
*             |     /|\     |         | / / / / / / / / / / / / / / / / / / / |    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*    Total    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*    Heigh    |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |Active|      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |Heigh |      |         |/ / / / / / Active Display Area / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |      |      |         |/ / / / / / / / / / / / / / / / / / / /|    |
*             |     \|/_____|_________|_______________________________________|    |
*             |     /|\     |                                                      |
*             |  VFP |      |                                                      |
*            \|/    \|/_____|______________________________________________________|
*            
*     
*     每个LCD设备都有自己的同步时序值：
*     Horizontal Synchronization (Hsync) 
*     Horizontal Back Porch (HBP)       
*     Active Width                      
*     Horizontal Front Porch (HFP)     
*   
*     Vertical Synchronization (Vsync)  
*     Vertical Back Porch (VBP)         
*     Active Heigh                       
*     Vertical Front Porch (VFP)         
*     
*     LCD_TFT 窗口水平和垂直的起始以及结束位置 :
*     ----------------------------------------------------------------
*   
*     HorizontalStart = (Offset_X + Hsync + HBP);
*     HorizontalStop  = (Offset_X + Hsync + HBP + Window_Width - 1); 
*     VarticalStart   = (Offset_Y + Vsync + VBP);
*     VerticalStop    = (Offset_Y + Vsync + VBP + Window_Heigh - 1);
*
*********************************************************************************************************
*/
__IO uint16_t Width, Height, HSYNC_W, VSYNC_W, HBP, HFP, VBP, VFP;
void LCD_ConfigLTDC(void)
{
	LTDC_InitTypeDef       LTDC_InitStruct;
	LTDC_Layer_TypeDef     LTDC_Layerx;

	/* 使能LTDC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_LTDC, ENABLE);

	/* 使能DMA2D */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2D, ENABLE);

	/* 配置LCD引脚 */
	LCD429_AF_GPIOConfig();

	/* 配置信号极性 */	
	LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;	/* HSYNC 低电平有效 */
	LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;	/* VSYNC 低电平有效 */
	LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;	/* DE 低电平有效 */
	LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;

	/* 背景色 */
	LTDC_InitStruct.LTDC_BackgroundRedValue = 0xff;
	LTDC_InitStruct.LTDC_BackgroundGreenValue = 0;
	LTDC_InitStruct.LTDC_BackgroundBlueValue = 0;
	
	/* 
	   LTDC时钟配置说明：
	     函数RCC_PLLSAIConfig的第一个参数是PLLSAI_N，第三个参数数PLLSAI_R。
	     函数RCC_LTDCCLKDivConfig的参数是RCC_PLLSAIDivR。
	   
	   下面举一个例子：PLLSAI_N = 400， PLLSAI_R = 4  RCC_PLLSAIDivR = 2:
	     首先，输入时钟 PLLSAI_VCO Input = HSE_VALUE / PLL_M = 8M / 8 = 1MHz 
	       输出时钟 PLLSAI_VCO Output  = PLLSAI_VCO Input * PLLSAI_N = 1 * 400 = 400 1MHz 
	       PLLLCDCLK = PLLSAI_VCO Output / PLLSAI_R = 400 / 4 = 100 1MHz 
	     最好，LTDC 时钟 = PLLLCDCLK / RCC_PLLSAIDivR = 100 / 2 = 50 1MHz 
	 */
				
	/* 支持6种面板 */
//	switch (g_LcdType)
	{
//		case LCD_35_480X320:	/* 3.5寸 480 * 320 */	
//			RCC_PLLSAIConfig(429, 2,  4);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);
//		
//			Width = 480;
//			Height = 272;
//			HSYNC_W = 10;
//			HBP = 20;
//			HFP = 20;
//			VSYNC_W = 20;
//			VBP = 20;
//			VFP = 20;
//			break;
		
//		case LCD_43_480X272:		/* 4.3寸 480 * 272  选择LTDC输出20MHz，所有颜色深度都可以选择这个时钟频率 */
//			RCC_PLLSAIConfig(280, 2,  7);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div2);

//			Width = 480;
//			Height = 272;

//			HSYNC_W = 40;
//			HBP = 2;
//			HFP = 2;
//			VSYNC_W = 9;
//			VBP = 2;
//			VFP = 2;
//			break;
//		
//		case LCD_50_480X272:		/* 5.0寸 480 * 272 */
//			RCC_PLLSAIConfig(429, 2,  4);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);
//		
//			Width = 480;
//			Height = 272;
//		
//			HSYNC_W = 40;
//			HBP = 2;
//			HFP = 2;
//			VSYNC_W = 9;
//			VBP = 2;
//			VFP = 2;			
//			break;
		
//		case LCD_50_800X480:		/* 5.0寸 800 * 480，24位或者32位色选择LTDC输出15MHz，16位或者8位30MHz */
			RCC_PLLSAIConfig(420, 2,  7);
			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div2);

			Width = 800;
			Height = 480;

			HSYNC_W = 96;	
			HBP = 10;
			HFP = 10;
			VSYNC_W = 2;
			VBP = 10;
			VFP = 10;			
//			break;
		
//		case LCD_70_800X480:		/* 7.0寸 800 * 480，24位或者32位色选择LTDC输出15MHz，16位或者8位30MHz*/
//			RCC_PLLSAIConfig(420, 2,  7);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div2);
//			
//			#if 0
//				RCC_PLLSAIConfig(400, 2,  2);
//				RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div4); 
//			#endif
//			Width = 800;
//			Height = 480;

//			HSYNC_W = 90;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
//			HBP = 10;
//			HFP = 10;
//		
//			VSYNC_W = 10;
//			VBP = 10;
//			VFP = 10;	
//			break;
		
//		case LCD_70_1024X600:		/* 7.0寸 1024 * 600 */
//			LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;	/* HSYNC 低电平有效 */
//			LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;	/* VSYNC 低电平有效 */
//			LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;	/* DE 低电平有效 */
//			LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IIPC;
//		
//			/* IPS 7寸 1024*600，  像素时钟频率范围 : 57 -- 65 --- 70.5MHz 
//		
//				PLLSAI_VCO Input   = HSE_VALUE / PLL_M = 8M / 4 = 2 Mhz
//				PLLSAI_VCO Output  = PLLSAI_VCO Input * PLLSAI_N =   2 * 429 = 858 Mhz
//				PLLLCDCLK = PLLSAI_VCO Output / PLLSAI_R = 858 / 4 = 214.5 Mhz
//				LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = 214.5 / 4 = 53.625 Mhz 	

//				(429, 2, 4); RCC_PLLSAIDivR_Div4 实测像素时钟 = 53.7M
//			*/
//			RCC_PLLSAIConfig(429, 2, 6);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div4);
//		
//			Width = 1024;
//			Height = 600;

//			HSYNC_W = 2;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
//			HBP = 157;
//			HFP = 160;
//		
//			VSYNC_W = 2;
//			VBP = 20;
//			VFP = 12;			
//			break;
//		
//		default:
//			RCC_PLLSAIConfig(429, 2,  4);
//			RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div8);

//			Width = 800;
//			Height = 480;

//			HSYNC_W = 80;	/* =10时，显示错位，20时部分屏可以的,80时全部OK */
//			HBP = 10;
//			HFP = 10;
//			VSYNC_W = 10;
//			VBP = 10;
//			VFP = 10;			
//			break;
	}
	
	g_LcdWidth  = Width;		/* 显示屏分辨率-宽度 */
	g_LcdHeight = Height;		/* 显示屏分辨率-高度 */
	
	/* 使能 PLLSAI */
	RCC_PLLSAICmd(ENABLE);
	/* 等待完成 */
	while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET);
	
	/* 配置LTDC的同步时序 */
	LTDC_InitStruct.LTDC_HorizontalSync = HSYNC_W;
	LTDC_InitStruct.LTDC_VerticalSync = VSYNC_W;
	LTDC_InitStruct.LTDC_AccumulatedHBP = LTDC_InitStruct.LTDC_HorizontalSync + HBP;
	LTDC_InitStruct.LTDC_AccumulatedVBP = LTDC_InitStruct.LTDC_VerticalSync + VBP;
	LTDC_InitStruct.LTDC_AccumulatedActiveW = Width + LTDC_InitStruct.LTDC_AccumulatedHBP;
	LTDC_InitStruct.LTDC_AccumulatedActiveH = Height + LTDC_InitStruct.LTDC_AccumulatedVBP;
	LTDC_InitStruct.LTDC_TotalWidth = LTDC_InitStruct.LTDC_AccumulatedActiveW + HFP;
	LTDC_InitStruct.LTDC_TotalHeigh = LTDC_InitStruct.LTDC_AccumulatedActiveH + VFP;

	LTDC_Init(&LTDC_InitStruct);
	
//	LCD_SetBackLight(BRIGHT_DEFAULT);	 /* 打开背光，设置为缺省亮度 */
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
