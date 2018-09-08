/*
*********************************************************************************************************
*
*	模块名称 : emWin的底层驱动文件
*	文件名称 : LCDConf_Lin_Template.c
*	版    本 : V1.0 
*	说    明 : LCD的部分驱动和emWin底层接口都在这个文件实现。
*              使用说明：
*                1. 此驱动自适应安富莱4.3寸，5寸和7寸屏。
*                2. 用户使用emWin前，请先使能STM32的CRC时钟，然后调用文件bsp_tft_429.c中的
*                   函数LCD_ConfigLTDC，之后调用GUI_Init()即可使用emWin。
*                3. 不同的显示屏对用的触摸驱动不同，用户自己使用的话，请周期性调用即可。
*                   电阻屏是：TOUCH_Scan()，1ms调用一次。
*                   电容屏是：GT811_OnePiontScan()或者FT5X06_OnePiontScan()，10ms调用一次。
*                4. 调试状态或者刚下载LCD的程序到F429里面，屏幕会抖动，这个是正常现象，详情请看
*                   http://bbs.armfly.com/read.php?tid=16892
*              配置说明：
*                1. F429的图层是由背景层，图层1和图层2组成。
*                2. 总共有12项配置选项，非常重要！！ 下面对每个配置选项都有详细说明。
*              移植说明：
*                  此文件可以直接使用，除了配置选项，其它无需做任何修改，用户要做的是提供两个函数：
*                1. 提供函数LCD_SetBackLight，实现背光的开关
*                2. 提供一个函数LCD_ConfigLTDC，具体实现参考文件bsp_tft_429.c中此函数的实现。
*                3. 提供了上面两个函数即可实现emWin的移植，当然，用户也可以直接修改此文件中的函数
*                  _LCD_InitController，实现F429/439的双图层配置即可。
*
*	修改记录 :
*		版本号    日期         作者      说明
*		V1.0    2016-01-05    Eric2013  1. 修正多缓冲模式的bug
*                                       2. 修正8位色_CM_L8，_CM_AL44无法正常显示bug
*                                       3. 修正驱动比较耗内存的bug。
*                                       4. 选择不使用函数_DMA_MixColors，此函数性价较低。详情请看：
*                                          http://bbs.armfly.com/read.php?tid=16919
*                                       5. 选择不使用DMA2D中断。因为此中断的作用就是做休眠唤醒用的。
*                                       6. 注释掉所有休眠指令，暂时用不上。
*
*		V1.1    2016-01-28    Eric2013  1. 取消掉变量定义uint16_t Width, Height, HSYNC_W, VSYNC_W, HBP, HFP, VBP, VFP
*                                          前面的__IO类型。
（
*       V1.2    2016-07-16    Eric2013  1. 使能函数_GetBitsPerPixel的使用，要不565格式的位图无法正常显示。
*                                       2. 重新整理配置选项的注释。
*
*
*	Copyright (C), 2016-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "GUI.h"
#include "GUI_Private.h"
#include "GUIDRV_Lin.h"



/*
**********************************************************************************************************
							调用外部文件的变量和函数(bsp_tft_429.c文件)
**********************************************************************************************************
*/
extern  uint16_t Width, Height, HSYNC_W, VSYNC_W, HBP, HFP, VBP, VFP;

/*
**********************************************************************************************************
									用户可以配置的选项
**********************************************************************************************************
*/
/* 0. 在官方代码的基础上再做优化，官方的部分函数效率低，耗内存, 0表示优化 */
#define emWin_Optimize   0

/* 
  1. 显示屏的物理分辨率，驱动已经做了显示屏自适应，支持4.3寸，5寸和7寸屏
     这里填写自适应显示屏中的最大分辨率。
*/
#define XSIZE_PHYS       800
#define YSIZE_PHYS       480

/* 2. 多缓冲 / 虚拟屏，多缓冲和虚拟屏不可同时使用，emWin不支持 */
#define NUM_BUFFERS      3 /* 定义多缓冲个数，仅可以设置1,2和3，也就是最大支持三缓冲 */
#define NUM_VSCREENS     1 /* 定义虚拟屏个数 */

/* 3. 没有图层激活时，背景色设置, 暂时未用到 */
#define BK_COLOR         GUI_DARKBLUE

/* 
   4. 重定义图层数，对于STM32F429/439，用户可以选择一个图层或者两个图层，不支持三图层 
      (1). 设置GUI_NUM_LAYERS = 1时，即仅使用图层1时，默认触摸值是发送给图层1的。
	  (2). 设置GUI_NUM_LAYERS = 2时，即图层1和图层2都已经使能，此时图层2是顶层，
	       用户需要根据自己的使用情况设置如下两个地方。
		   a. 在bsp_touch.c文件中的函数TOUCH_InitHard里面设置参数State.Layer = 1，1就表示
		      给图层2发送触摸值。
		   b. 调用GUI_Init函数后，调用函数GUI_SelectLayer(1), 设置当前操作的是图层2。
*/
#undef  GUI_NUM_LAYERS
#define GUI_NUM_LAYERS    1

/* 
   5. 设置图层1和图层2对应的显存地址
      (1) EXT_SDRAM_ADDR 是SDRAM的首地址。
      (2) LCD_LAYER0_FRAME_BUFFER 是图层1的显存地址。
	  (3) LCD_LAYER1_FRAME_BUFFER 是图层2的显存地址。
	  (4) 每个图层的显存大小比较考究，这里进行下简单的说明。
	      如果用户选择的颜色模式 = 32位色ARGB8888，显存的大小：
	      XSIZE_PHYS * YSIZE_PHYS * 4 * NUM_VSCREENS * NUM_BUFFERS
		  
	      颜色模式 = 24位色RGB888，显存的大小：
	      XSIZE_PHYS * YSIZE_PHYS * 3 * NUM_VSCREENS * NUM_BUFFERS
		  
	      颜色模式 = 16位色RGB566，ARGB1555, ARGB4444，AL88，那么显存的大小就是：
	      XSIZE_PHYS * YSIZE_PHYS * 2 * NUM_VSCREENS * NUM_BUFFERS

	      颜色模式 = 8位色L8，AL44，那么显存的大小就是：
	      XSIZE_PHYS * YSIZE_PHYS * 1 * NUM_VSCREENS * NUM_BUFFERS	
      
      这里为了方便起见，将开发板配套的16MB的SDRAM前8MB分配给LCD显存使用，后8MB用于emWin动态内存。
	  对于24位色，16位色，8位色，用户可以对其使能三缓冲，并且使能双图层。但是32位色也使能三缓冲和双
	  图层的话会超出8MB，所以用户根据自己的情况做显存和emWin动态内存的分配调整。
	    举一个例子，对于800*480分辨率的显示屏，使能32位色，三缓冲，那么最终一个图层需要的大小就是
      800 * 480 * 4 * 3  = 4.394MB的空间，如果是双图层，已经超出8MB的分配范围。

      (5)为了方便起见，图层2的宏定义LCD_LAYER1_FRAME_BUFFER中的参数4是按照32位色设置的，如果用户的图层1
         使用的是8位色，这里填数字1,如果是16位色，这里填2，如果是24位色，这里填3。
*/
#define LCD_LAYER0_FRAME_BUFFER  EXT_SDRAM_ADDR
#define LCD_LAYER1_FRAME_BUFFER  (LCD_LAYER0_FRAME_BUFFER + XSIZE_PHYS * YSIZE_PHYS * 4 * NUM_VSCREENS * NUM_BUFFERS)

/* 
   6. STM32F429/439支持的颜色模式，所有模式都支持，用户可任意配置。
      特别注意如下两个问题：
	  (1) 如果用户选择了ARGB8888或者RGB888模式，LCD闪烁比较厉害的话，
	      请降低LTDC的时钟大小，在文件bsp_tft_429.c的函数LCD_ConfigLTDC里面设置。
	      a. 一般800*480分辨率的显示屏，ARGB8888或者RGB888模式LTDC时钟选择10-20MHz即可。
	      b. 480*272分辨率的可以高些，取20MHz左右即可。
	  (2) 16位色或者8位色模式，LTDC的时钟频率一般可以比24位色或者32位色的高一倍。
*/
#define _CM_ARGB8888      1
#define _CM_RGB888        2
#define _CM_RGB565        3
#define _CM_ARGB1555      4
#define _CM_ARGB4444      5
#define _CM_L8            6
#define _CM_AL44          7
#define _CM_AL88          8

/* 7. 配置图层1的颜色模式和分辨率大小 */
#define COLOR_MODE_0      _CM_RGB565
#define XSIZE_0           XSIZE_PHYS
#define YSIZE_0           YSIZE_PHYS

/* 8. 配置图层2的的颜色模式和分辨率大小 */
#define COLOR_MODE_1      _CM_RGB565
#define XSIZE_1           XSIZE_PHYS
#define YSIZE_1           YSIZE_PHYS

/* 9. 单图层情况下，根据用户选择的颜色模式可自动选择图层1的emWin的驱动和颜色模式 */
#if   (COLOR_MODE_0 == _CM_ARGB8888)
  #define COLOR_CONVERSION_0 GUICC_M8888I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_32
#elif (COLOR_MODE_0 == _CM_RGB888)
  #define COLOR_CONVERSION_0 GUICC_M888
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_24
#elif (COLOR_MODE_0 == _CM_RGB565)
  #define COLOR_CONVERSION_0 GUICC_M565
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_16
#elif (COLOR_MODE_0 == _CM_ARGB1555)
  #define COLOR_CONVERSION_0 GUICC_M1555I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_16
#elif (COLOR_MODE_0 == _CM_ARGB4444)
  #define COLOR_CONVERSION_0 GUICC_M4444I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_16
#elif (COLOR_MODE_0 == _CM_L8)
  #define COLOR_CONVERSION_0 GUICC_8666
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_8
#elif (COLOR_MODE_0 == _CM_AL44)
  #define COLOR_CONVERSION_0 GUICC_1616I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_8
#elif (COLOR_MODE_0 == _CM_AL88)
  #define COLOR_CONVERSION_0 GUICC_88666I
  #define DISPLAY_DRIVER_0   GUIDRV_LIN_16
#else
  #error Illegal color mode 0!
#endif

/* 10. 双图层情况下，根据用户选择的颜色模式可自动选择图层2的emWin的驱动和颜色模式 */
#if (GUI_NUM_LAYERS > 1)

#if   (COLOR_MODE_1 == _CM_ARGB8888)
  #define COLOR_CONVERSION_1 GUICC_M8888I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_32
#elif (COLOR_MODE_1 == _CM_RGB888)
  #define COLOR_CONVERSION_1 GUICC_M888
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_24
#elif (COLOR_MODE_1 == _CM_RGB565)
  #define COLOR_CONVERSION_1 GUICC_M565
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_16
#elif (COLOR_MODE_1 == _CM_ARGB1555)
  #define COLOR_CONVERSION_1 GUICC_M1555I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_16
#elif (COLOR_MODE_1 == _CM_ARGB4444)
  #define COLOR_CONVERSION_1 GUICC_M4444I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_16
#elif (COLOR_MODE_1 == _CM_L8)
  #define COLOR_CONVERSION_1 GUICC_8666
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_8
#elif (COLOR_MODE_1 == _CM_AL44)
  #define COLOR_CONVERSION_1 GUICC_1616I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_8
#elif (COLOR_MODE_1 == _CM_AL88)
  #define COLOR_CONVERSION_1 GUICC_88666I
  #define DISPLAY_DRIVER_1   GUIDRV_LIN_16
#else
  #error Illegal color mode 1!
#endif

#else

#undef XSIZE_0
#undef YSIZE_0
#define XSIZE_0       XSIZE_PHYS
#define YSIZE_0       YSIZE_PHYS

#endif

/*11. 配置选项检测，防止配置错误或者某些选项没有配置 */
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   NUM_VSCREENS
  #define NUM_VSCREENS 1
#else
  #if (NUM_VSCREENS <= 0)
    #error At least one screeen needs to be defined!
  #endif
#endif
#if (NUM_VSCREENS > 1) && (NUM_BUFFERS > 1)
  #error Virtual screens and multiple buffers are not allowed!
#endif

/*
**********************************************************************************************************
									使用DMA2D重定向颜色的批量转换
**********************************************************************************************************
*/
#define DEFINE_DMA2D_COLORCONVERSION(PFIX, PIXELFORMAT)                                                        \
static void _Color2IndexBulk_##PFIX##_DMA2D(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex) { \
  _DMA_Color2IndexBulk(pColor, pIndex, NumItems, SizeOfIndex, PIXELFORMAT);                                    \
}                                                                                                              \
static void _Index2ColorBulk_##PFIX##_DMA2D(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex) { \
  _DMA_Index2ColorBulk(pColor, pIndex, NumItems, SizeOfIndex, PIXELFORMAT);                                    \
}

/* 函数声明 */
static void _DMA_Index2ColorBulk(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat);
static void _DMA_Color2IndexBulk(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat);

/* 颜色转换 */
DEFINE_DMA2D_COLORCONVERSION(M8888I, LTDC_Pixelformat_ARGB8888)
DEFINE_DMA2D_COLORCONVERSION(M888,   LTDC_Pixelformat_ARGB8888) /* Internal pixel format of emWin is 32 bit, because of that ARGB8888 */
DEFINE_DMA2D_COLORCONVERSION(M565,   LTDC_Pixelformat_RGB565)
DEFINE_DMA2D_COLORCONVERSION(M1555I, LTDC_Pixelformat_ARGB1555)
DEFINE_DMA2D_COLORCONVERSION(M4444I, LTDC_Pixelformat_ARGB4444)

/*
**********************************************************************************************************
									文件内使用的全局变量
**********************************************************************************************************
*/
static LTDC_Layer_TypeDef       * _apLayer[]  = { LTDC_Layer1, LTDC_Layer2 };
static const U32                  _aAddr[]    = { LCD_LAYER0_FRAME_BUFFER, LCD_LAYER1_FRAME_BUFFER};
static int                        _aPendingBuffer[GUI_NUM_LAYERS];
static int                        _aBufferIndex[GUI_NUM_LAYERS];
static int                        _axSize[GUI_NUM_LAYERS];
static int                        _aySize[GUI_NUM_LAYERS];
static int                        _aBytesPerPixels[GUI_NUM_LAYERS];
#if 0	 /* 官方此处设置有问题，暂时改为如下，不需要乘以sizeof(U32) */
  static U32 						  _aBuffer_DMA2D[XSIZE_PHYS * sizeof(U32)];
  static U32 						  _aBuffer_FG   [XSIZE_PHYS * sizeof(U32)];
  static U32 						  _aBuffer_BG   [XSIZE_PHYS * sizeof(U32)];
#else
  static U32 						  _aBuffer_DMA2D[XSIZE_PHYS];
  static U32 						  _aBuffer_FG   [XSIZE_PHYS];
  static U32 						  _aBuffer_BG   [XSIZE_PHYS];
#endif

static const LCD_API_COLOR_CONV * _apColorConvAPI[] = {
  COLOR_CONVERSION_0,
#if GUI_NUM_LAYERS > 1
  COLOR_CONVERSION_1,
#endif
};

/*
*********************************************************************************************************
*	函 数 名: _GetPixelformat
*	功能说明: 获取图层1或者图层2使用的颜色格式
*	形    参: LayerIndex  图层
*	返 回 值: 颜色格式
*********************************************************************************************************
*/
static U32 _GetPixelformat(int LayerIndex) {
  const LCD_API_COLOR_CONV * pColorConvAPI;

  if (LayerIndex >= GUI_COUNTOF(_apColorConvAPI)) {
    return 0;
  }
  pColorConvAPI = _apColorConvAPI[LayerIndex];
  if        (pColorConvAPI == GUICC_M8888I) {
    return LTDC_Pixelformat_ARGB8888;
  } else if (pColorConvAPI == GUICC_M888) {
    return LTDC_Pixelformat_RGB888;
  } else if (pColorConvAPI == GUICC_M565) {
    return LTDC_Pixelformat_RGB565;
  } else if (pColorConvAPI == GUICC_M1555I) {
    return LTDC_Pixelformat_ARGB1555;
  } else if (pColorConvAPI == GUICC_M4444I) {
    return LTDC_Pixelformat_ARGB4444;
  } else if (pColorConvAPI == GUICC_8666) {
    return LTDC_Pixelformat_L8;
  } else if (pColorConvAPI == GUICC_1616I) {
    return LTDC_Pixelformat_AL44;
  } else if (pColorConvAPI == GUICC_88666I) {
    return LTDC_Pixelformat_AL88;
  }
  while (1); // Error
}

/*
*********************************************************************************************************
*	函 数 名: _GetBytesPerLine
*	功能说明: 根据LayerIndex指定的图层和xSize指定的长度获取需要的字节数
*	形    参: LayerIndex  图层
*             xSize       像素个数
*	返 回 值: 字节数
*********************************************************************************************************
*/
static int _GetBytesPerLine(int LayerIndex, int xSize) {
  int BitsPerPixel, BytesPerLine;

  BitsPerPixel  = LCD_GetBitsPerPixelEx(LayerIndex);
  BytesPerLine = (BitsPerPixel * xSize + 7) / 8;
  return BytesPerLine;
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_LoadLUT
*	功能说明: 启动CLUT的自动加载
*	形    参: pColor      前景层图像的CLUT地址所使用的数据地址
*             NumItems    前景层图像所用的CLUT的大小 
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_LoadLUT(LCD_COLOR * pColor, U32 NumItems) {
  DMA2D->FGCMAR  = (U32)pColor;                     // Foreground CLUT Memory Address Register
  //
  // Foreground PFC Control Register
  //
  DMA2D->FGPFCCR  = LTDC_Pixelformat_RGB888         // Pixel format
                  | ((NumItems - 1) & 0xFF) << 8;   // Number of items to load
  DMA2D->FGPFCCR |= (1 << 5);                       // Start loading
  //
  // Waiting not required here...
  //
}

/*
*********************************************************************************************************
*	函 数 名: _InvertAlpha_SwapRB
*	功能说明: emWin的颜色格式跟DMA2D的颜色格式不同，DMA2D的颜色格式要转换为
*             emWin的颜色格式，需要以下两点：
*             1. 交换R和B的位置。
*             2. 翻转alpha通道。
*             此函数就完成这两个工作，反之亦然。
*	形    参: pColorSrc  DMA2D格式颜色地址，即原始颜色
*             pColorDst  emWin格式颜色地址，即转换后颜色
*             NumItems   要转换的颜色个数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _InvertAlpha_SwapRB(LCD_COLOR * pColorSrc, LCD_COLOR * pColorDst, U32 NumItems) {
  U32 Color;
  do {
    Color = *pColorSrc++;
    *pColorDst++ = ((Color & 0x000000FF) << 16)         // Swap red <-> blue
                 |  (Color & 0x0000FF00)                // Green
                 | ((Color & 0x00FF0000) >> 16)         // Swap red <-> blue
                 | ((Color & 0xFF000000) ^ 0xFF000000); // Invert alpha
  } while (--NumItems);
}

/*
*********************************************************************************************************
*	函 数 名: _InvertAlpha
*	功能说明: emWin的颜色格式跟DMA2D的颜色格式不同，DMA2D的颜色格式要转换为
*             emWin的颜色格式，需要以下两点：
*             1. 交换R和B的位置。
*             2. 翻转alpha通道。
*             此函数就完成第二工作，反之亦然。
*	形    参: pColorSrc  DMA2D格式颜色地址，即原始颜色
*             pColorDst  emWin格式颜色地址，即转换后颜色
*             NumItems   要转换的颜色个数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _InvertAlpha(LCD_COLOR * pColorSrc, LCD_COLOR * pColorDst, U32 NumItems) {
  U32 Color;

  do {
    Color = *pColorSrc++;
    *pColorDst++ = Color ^ 0xFF000000; // Invert alpha
  } while (--NumItems);
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_AlphaBlendingBulk
*	功能说明: 实现批量的颜色混合转换，获取前景色和背景色后执行PFC(像素格式转换器)，DMA2D方式选择存储器到存储器并执行混合。
*	形    参: pColorFG   前景色地址
*             pColorBG   背景色地址
*             pColorDst  转换后颜色存储
*             NumItems   要转换的颜色个数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_AlphaBlendingBulk(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U32 NumItems) {
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00020000UL | (1 << 9);         // Control Register (Memory to memory with blending of FG and BG and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)pColorFG;                   // Foreground Memory Address Register
  DMA2D->BGMAR   = (U32)pColorBG;                   // Background Memory Address Register
  DMA2D->OMAR    = (U32)pColorDst;                  // Output Memory Address Register (Destination address)
  //
  // Set up offsets
  //
  DMA2D->FGOR    = 0;                               // Foreground Offset Register
  DMA2D->BGOR    = 0;                               // Background Offset Register
  DMA2D->OOR     = 0;                               // Output Offset Register
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = LTDC_Pixelformat_ARGB8888;       // Foreground PFC Control Register (Defines the FG pixel format)
  DMA2D->BGPFCCR = LTDC_Pixelformat_ARGB8888;       // Background PFC Control Register (Defines the BG pixel format)
  DMA2D->OPFCCR  = LTDC_Pixelformat_ARGB8888;       // Output     PFC Control Register (Defines the output pixel format)
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(NumItems << 16) | 1;       // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //
  
  DMA2D->CR     |= 1;  
    
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                      // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_AlphaBlending
*	功能说明: 实现批量的颜色混合转换
*	形    参: pColorFG   前景色地址
*             pColorBG   背景色地址
*             pColorDst  转换后颜色存储
*             NumItems   要转换的颜色个数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_AlphaBlending(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U32 NumItems) {
  //
  // Invert alpha values
  //
  _InvertAlpha(pColorFG, _aBuffer_FG, NumItems);
  _InvertAlpha(pColorBG, _aBuffer_BG, NumItems);
  //
  // Use DMA2D for mixing
  //
  _DMA_AlphaBlendingBulk(_aBuffer_FG, _aBuffer_BG, _aBuffer_DMA2D, NumItems);
  //
  // Invert alpha values
  //
  _InvertAlpha(_aBuffer_DMA2D, pColorDst, NumItems);
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_MixColors
*	功能说明: 通过给定的alpha值，实现两种颜色的混合(感觉只执行一次两种颜色的混合也用DMA，有些臃肿)
*             如果背景色是透明的，直接返回前景色。
*	形    参: Color     前景色地址
*             BkColor   背景色地址
*             Intens    即alpha值
*	返 回 值: 无
*********************************************************************************************************
*/
#if emWin_Optimize 
static LCD_COLOR _DMA_MixColors(LCD_COLOR Color, LCD_COLOR BkColor, U8 Intens) {
  U32 ColorFG, ColorBG, ColorDst;

  if ((BkColor & 0xFF000000) == 0xFF000000) {
    return Color;
  }
  ColorFG = Color   ^ 0xFF000000;
  ColorBG = BkColor ^ 0xFF000000;
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00020000UL | (1 << 9);         // Control Register (Memory to memory with blending of FG and BG and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)&ColorFG;                   // Foreground Memory Address Register
  DMA2D->BGMAR   = (U32)&ColorBG;                   // Background Memory Address Register
  DMA2D->OMAR    = (U32)&ColorDst;                  // Output Memory Address Register (Destination address)
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = LTDC_Pixelformat_ARGB8888
                 | (1UL << 16)
                 | ((U32)Intens << 24);
  DMA2D->BGPFCCR = LTDC_Pixelformat_ARGB8888
                 | (0UL << 16)
                 | ((U32)(255 - Intens) << 24);
  DMA2D->OPFCCR  = LTDC_Pixelformat_ARGB8888;
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(1 << 16) | 1;              // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //

  //_DMA_ExecOperation();
  DMA2D->CR     |= 1;                               // Control Register (Start operation)
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                      // Sleep until next interrupt
  }

  return ColorDst ^ 0xFF000000;
}
#endif

/*
*********************************************************************************************************
*	函 数 名: _DMA_MixColorsBulk
*	功能说明: 通过给定的alpha值，实现两种颜色的批量混合
*	形    参: pColorFG   前景色地址
*             pColorBG   背景色地址
*             pColorDst  混合后颜色存储的地址
*             Intens     即alpha值
*             NumItems   转换的颜色数量
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_MixColorsBulk(LCD_COLOR * pColorFG, LCD_COLOR * pColorBG, LCD_COLOR * pColorDst, U8 Intens, U32 NumItems) {
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00020000UL | (1 << 9);         // Control Register (Memory to memory with blending of FG and BG and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)pColorFG;                   // Foreground Memory Address Register
  DMA2D->BGMAR   = (U32)pColorBG;                   // Background Memory Address Register
  DMA2D->OMAR    = (U32)pColorDst;                  // Output Memory Address Register (Destination address)
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = LTDC_Pixelformat_ARGB8888
                 | (1UL << 16)
                 | ((U32)Intens << 24);
  DMA2D->BGPFCCR = LTDC_Pixelformat_ARGB8888
                 | (0UL << 16)
                 | ((U32)(255 - Intens) << 24);
  DMA2D->OPFCCR  = LTDC_Pixelformat_ARGB8888;
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(NumItems << 16) | 1;              // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //
  DMA2D->CR     |= 1;  
    
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_MixColorsBulk
*	功能说明: 将一块显示区的前景色和背景色进行混合
*	形    参: pFG   前景色地址
*             pBG   背景色地址
*             pDst  混合后颜色存储的地址
*             OffFG    前景色偏移地址
*             OffBG    背景色偏移地址
*             OffDest  混合后偏移地址
*             xSize    显示区x轴大小
*             ySize    显示区y轴大小
*             Intens   即alpha值
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_MixColorsBulk(U32 * pFG, U32 * pBG, U32 * pDst, unsigned OffFG, unsigned OffBG, unsigned OffDest, unsigned xSize, unsigned ySize, U8 Intens) {
  int y;

  GUI_USE_PARA(OffFG);
  GUI_USE_PARA(OffDest);
  for (y = 0; y < ySize; y++) {
    //
    // Invert alpha values
    //
    _InvertAlpha(pFG, _aBuffer_FG, xSize);
    _InvertAlpha(pBG, _aBuffer_BG, xSize);
    //
    //
    //
    _DMA_MixColorsBulk(_aBuffer_FG, _aBuffer_BG, _aBuffer_DMA2D, Intens, xSize);
    //
    //
    //
    _InvertAlpha(_aBuffer_DMA2D, pDst, xSize);
    pFG  += xSize + OffFG;
    pBG  += xSize + OffBG;
    pDst += xSize + OffDest;
  }
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_ConvertColor
*	功能说明: 颜色转换，从一个颜色格式转换到另一种颜色格式
*	形    参: pSrc   源颜色地址
*             pDst   转换后颜色存储地址
*             PixelFormatSrc  源颜色格式
*             PixelFormatDst  转换后颜色格式
*             NumItems        要转换的颜色个数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_ConvertColor(void * pSrc, void * pDst,  U32 PixelFormatSrc, U32 PixelFormatDst, U32 NumItems) {
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00010000UL | (1 << 9);         // Control Register (Memory to memory with pixel format conversion and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)pSrc;                       // Foreground Memory Address Register (Source address)
  DMA2D->OMAR    = (U32)pDst;                       // Output Memory Address Register (Destination address)
  //
  // Set up offsets
  //
  DMA2D->FGOR    = 0;                               // Foreground Offset Register (Source line offset)
  DMA2D->OOR     = 0;                               // Output Offset Register (Destination line offset)
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = PixelFormatSrc;                  // Foreground PFC Control Register (Defines the input pixel format)
  DMA2D->OPFCCR  = PixelFormatDst;                  // Output PFC Control Register (Defines the output pixel format)
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(NumItems << 16) | 1;       // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //
  DMA2D->CR     |= 1;  
    
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Index2ColorBulk
*	功能说明: 通过DMA2D，将当前显示屏的颜色数据转换为emWin的32位ARGB颜色数据。
*	形    参: pIndex       显示屏颜色地址
*             pColor       转换成适用于emWin的颜色地址
*             NumItems     转换的颜色数量
*             SizeOfIndex  未使用
*             PixelFormat  显示屏当前使用的颜色格式
*	返 回 值: 无
*********************************************************************************************************
*/
/*
*********************************************************************************************************
* Purpose:
*   This routine is used by the emWin color conversion routines to use DMA2D for
*   color conversion. It converts the given index values to 32 bit colors.
*   Because emWin uses ABGR internally and 0x00 and 0xFF for opaque and fully
*   transparent the color array needs to be converted after DMA2D has been used.
*********************************************************************************************************
*/
static void _DMA_Index2ColorBulk(void * pIndex, LCD_COLOR * pColor, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat) {
  //
  // Use DMA2D for the conversion
  //
  _DMA_ConvertColor(pIndex, _aBuffer_DMA2D, PixelFormat, LTDC_Pixelformat_ARGB8888, NumItems);
  //
  // Convert colors from ARGB to ABGR and invert alpha values
  //
  _InvertAlpha_SwapRB(_aBuffer_DMA2D, pColor, NumItems);
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Color2IndexBulk
*	功能说明: 通过DMA2D，将emWin的32位ARGB颜色数据转换为适用于当前显示屏的颜色数据
*	形    参: pIndex       显示屏颜色地址
*             pColor       转换成适用于emWin的颜色地址
*             NumItems     转换的颜色数量
*             SizeOfIndex  未使用
*             PixelFormat  显示屏当前使用的颜色格式
*	返 回 值: 无
*********************************************************************************************************
*/
/*
*********************************************************************************************************
* Purpose:
*   This routine is used by the emWin color conversion routines to use DMA2D for
*   color conversion. It converts the given 32 bit color array to index values.
*   Because emWin uses ABGR internally and 0x00 and 0xFF for opaque and fully
*   transparent the given color array needs to be converted before DMA2D can be used.
*********************************************************************************************************
*/
#if 1 /* 将此函数加入，测试发现不加入此函数，BMP565格式的位图无法正常显示 */
static int _GetBitsPerPixel(int Pixelformat) 
{
	switch (Pixelformat) 
	{
		case LTDC_Pixelformat_ARGB8888:
			return 32;
		case LTDC_Pixelformat_RGB888:
			return 24;
		case LTDC_Pixelformat_RGB565:
			return 16;
		case LTDC_Pixelformat_ARGB1555:
			return 16;
		case LTDC_Pixelformat_ARGB4444:
			return 16;
		case LTDC_Pixelformat_L8:
			return 8;
		case LTDC_Pixelformat_AL44:
			return 8;
		case LTDC_Pixelformat_AL88:
			return 16;
	}
	return 0;
}
#endif 

static void _DMA_Color2IndexBulk(LCD_COLOR * pColor, void * pIndex, U32 NumItems, U8 SizeOfIndex, U32 PixelFormat) {
  //
  // Convert colors from ABGR to ARGB and invert alpha values
  //
  _InvertAlpha_SwapRB(pColor, _aBuffer_DMA2D, NumItems);
  //
  // Use DMA2D for the conversion
  //
  _DMA_ConvertColor(_aBuffer_DMA2D, pIndex, LTDC_Pixelformat_ARGB8888, PixelFormat, NumItems);

#if 1 /* 将此函数加入，测试发现不加入此函数，BMP565格式的位图无法正常显示  */
  {
    int BitsPerPixel;
    if (SizeOfIndex == 4) {
      BitsPerPixel = _GetBitsPerPixel(PixelFormat);
      GUI__ExpandPixelIndices(pIndex, NumItems, BitsPerPixel);
    }	
  }
#endif
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_GetpPalConvTable
*	功能说明: 转换颜色板以适应控制器设置的颜色格式。
*	形    参: pLogPal   源颜色板地址
*             pBitmap   位图地址
*             LayerIndex  源颜色格式
*	返 回 值: 转换后的颜色板地址
*********************************************************************************************************
*/
/*
*********************************************************************************************************
* Purpose:
*   The emWin function LCD_GetpPalConvTable() normally translates the given colors into
*   index values for the display controller. In case of index based bitmaps without
*   transparent pixels we load the palette only to the DMA2D LUT registers to be
*   translated (converted) during the process of drawing via DMA2D.
*********************************************************************************************************
*/
static LCD_PIXELINDEX * _LCD_GetpPalConvTable(const LCD_LOGPALETTE GUI_UNI_PTR * pLogPal, const GUI_BITMAP GUI_UNI_PTR * pBitmap, int LayerIndex) {
  void (* pFunc)(void);
  int DoDefault = 0;

  //
  // Check if we have a non transparent device independent bitmap
  //
  if (pBitmap->BitsPerPixel == 8) {
    pFunc = LCD_GetDevFunc(LayerIndex, LCD_DEVFUNC_DRAWBMP_8BPP);
    if (pFunc) {
      if (pBitmap->pPal) {
        if (pBitmap->pPal->HasTrans) {
          DoDefault = 1;
        }
      } else {
        DoDefault = 1;
      }
    } else {
      DoDefault = 1;
    }
  } else {
    DoDefault = 1;
  }
  //
  // Default palette management for other cases
  //
  if (DoDefault) {
    //
    // Return a pointer to the index values to be used by the controller
    //
    return LCD_GetpPalConvTable(pLogPal);
  }
  //
  // Convert palette colors from ARGB to ABGR
  //
  _InvertAlpha_SwapRB((U32 *)pLogPal->pPalEntries, _aBuffer_DMA2D, pLogPal->NumEntries);
  //
  // Load LUT using DMA2D
  //
  _DMA_LoadLUT(_aBuffer_DMA2D, pLogPal->NumEntries);
  //
  // Return something not NULL
  //
  return _aBuffer_DMA2D;
}

/*
*********************************************************************************************************
*	函 数 名: _LTDC_LayerEnableColorKeying
*	功能说明: 使能色键后，当前像素（格式转换后、混合前的像素）将与色键进行比较。如果当前像素与
*             编程的 RGB 值相匹配，则该像素的所有通道 (ARGB) 均设置为 0。
*	形    参: LTDC_Layer_TypeDef   结构体指针
*             NewState             DISABLE 禁止
*                                  ENABLE  使能
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LTDC_LayerEnableColorKeying(LTDC_Layer_TypeDef * LTDC_Layerx, int NewState) {
  if (NewState != DISABLE) {
    LTDC_Layerx->CR |= (U32)LTDC_LxCR_COLKEN;
  } else {
    LTDC_Layerx->CR &= ~(U32)LTDC_LxCR_COLKEN;
  }
  LTDC->SRCR = LTDC_SRCR_VBR; // Reload on next blanking period
}

/*
*********************************************************************************************************
*	函 数 名: _LTDC_LayerEnableLUT
*	功能说明: 使能LUT颜色查找表
*	形    参: LTDC_Layer_TypeDef   结构体指针
*             NewState             DISABLE 禁止
*                                  ENABLE  使能
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LTDC_LayerEnableLUT(LTDC_Layer_TypeDef * LTDC_Layerx, int NewState) {
  if (NewState != DISABLE) {
    LTDC_Layerx->CR |= (U32)LTDC_LxCR_CLUTEN;
  } else {
    LTDC_Layerx->CR &= ~(U32)LTDC_LxCR_CLUTEN;
  }
  LTDC->SRCR = LTDC_SRCR_VBR; // Reload on next blanking period
}

/*
*********************************************************************************************************
*	函 数 名: _LTDC_SetLayerPos
*	功能说明: 设置图层的位置
*	形    参: LayerIndex   结构体指针
*             xPos         X位置
*             yPos         Y位置
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LTDC_SetLayerPos(int LayerIndex, int xPos, int yPos) {
  int xSize, ySize;
  U32 HorizontalStart, HorizontalStop, VerticalStart, VerticalStop;

  xSize = LCD_GetXSizeEx(LayerIndex);
  ySize = LCD_GetYSizeEx(LayerIndex);
  HorizontalStart = xPos + HBP + 1;
  HorizontalStop  = xPos + HBP + xSize;
  VerticalStart   = yPos + VBP + 1;
  VerticalStop    = yPos + VBP + ySize;
  //
  // Horizontal start and stop position
  //
  _apLayer[LayerIndex]->WHPCR &= ~(LTDC_LxWHPCR_WHSTPOS | LTDC_LxWHPCR_WHSPPOS);
  _apLayer[LayerIndex]->WHPCR = (HorizontalStart | (HorizontalStop << 16));
  //
  // Vertical start and stop position
  //
  _apLayer[LayerIndex]->WVPCR &= ~(LTDC_LxWVPCR_WVSTPOS | LTDC_LxWVPCR_WVSPPOS);
  _apLayer[LayerIndex]->WVPCR  = (VerticalStart | (VerticalStop << 16));
  //
  // Reload configuration
  //
  LTDC_ReloadConfig(LTDC_SRCR_VBR); // Reload on next blanking period
}

/*
*********************************************************************************************************
*	函 数 名: _LTDC_SetLayerAlpha
*	功能说明: 设置图层的恒定Alpha
*	形    参: LayerIndex   图层
*             Alpha        alpha数值
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LTDC_SetLayerAlpha(int LayerIndex, int Alpha) {
  //
  // Set constant alpha value
  //
  _apLayer[LayerIndex]->CACR &= ~(LTDC_LxCACR_CONSTA);
  _apLayer[LayerIndex]->CACR  = 255 - Alpha;
  //
  // Reload configuration
  //
  LTDC_ReloadConfig(LTDC_SRCR_IMR/*LTDC_SRCR_VBR*/); // Reload on next blanking period/**/
}

/*
*********************************************************************************************************
*	函 数 名: _LTDC_SetLUTEntry
*	功能说明: 设置LUT地址和此地址对应的RGB值
*	形    参: LayerIndex   图层
*             Color        RGB值
*             Pos          RGB 值的 CLUT 地址（ CLUT 内的颜色位置）
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LTDC_SetLUTEntry(int LayerIndex, U32 Color, int Pos) {
  U32 r, g, b, a;

  r = ( Color        & 0xff) << 16;
  g = ((Color >>  8) & 0xff) <<  8;
  b = ((Color >> 16) & 0xff);
  a = Pos << 24;
  _apLayer[LayerIndex]->CLUTWR &= ~(LTDC_LxCLUTWR_BLUE | LTDC_LxCLUTWR_GREEN | LTDC_LxCLUTWR_RED | LTDC_LxCLUTWR_CLUTADD);
  _apLayer[LayerIndex]->CLUTWR  = r | g | b | a;
  //
  // Reload configuration
  //
  LTDC_ReloadConfig(LTDC_SRCR_IMR);
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Copy
*	功能说明: 通过DMA2D从前景层复制指定区域的颜色数据到目标区域
*	形    参: LayerIndex    图层
*             pSrc          颜色数据源地址
*             pDst          颜色数据目的地址
*             xSize         要复制区域的X轴大小，即每行像素数
*             ySize         要复制区域的Y轴大小，即行数
*             OffLineSrc    前景层图像的行偏移
*             OffLineDst    输出的行偏移
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_Copy(int LayerIndex, void * pSrc, void * pDst, int xSize, int ySize, int OffLineSrc, int OffLineDst) {
  U32 PixelFormat;

  PixelFormat = _GetPixelformat(LayerIndex);
  DMA2D->CR      = 0x00000000UL | (1 << 9);         // Control Register (Memory to memory and TCIE)
  DMA2D->FGMAR   = (U32)pSrc;                       // Foreground Memory Address Register (Source address)
  DMA2D->OMAR    = (U32)pDst;                       // Output Memory Address Register (Destination address)
  DMA2D->FGOR    = OffLineSrc;                      // Foreground Offset Register (Source line offset)
  DMA2D->OOR     = OffLineDst;                      // Output Offset Register (Destination line offset)
  DMA2D->FGPFCCR = PixelFormat;                     // Foreground PFC Control Register (Defines the input pixel format)
  DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize; // Number of Line Register (Size configuration of area to be transfered)
  DMA2D->CR     |= 1;                               // Start operation
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_Fill
*	功能说明: 通过DMA2D对于指定区域进行颜色填充
*	形    参: LayerIndex    图层
*             pDst          颜色数据目的地址
*             xSize         要复制区域的X轴大小，即每行像素数
*             ySize         要复制区域的Y轴大小，即行数
*             OffLine       前景层图像的行偏移
*             ColorIndex    要填充的颜色值
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_Fill(int LayerIndex, void * pDst, int xSize, int ySize, int OffLine, U32 ColorIndex) {
  U32 PixelFormat;

  PixelFormat = _GetPixelformat(LayerIndex);
  DMA2D->CR      = 0x00030000UL | (1 << 9);         // Register to memory and TCIE
  DMA2D->OCOLR   = ColorIndex;                      // Color to be used
  DMA2D->OMAR    = (U32)pDst;                       // Destination address
  DMA2D->OOR     = OffLine;                         // Destination line offset
  DMA2D->OPFCCR  = PixelFormat;                     // Defines the number of pixels to be transfered
  DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize; // Size configuration of area to be transfered
  DMA2D->CR     |= 1;                               // Start operation
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	函 数 名: _GetBufferSize
*	功能说明: 获取指定层显存大小
*	形    参: LayerIndex    图层
*	返 回 值: 显存大小
*********************************************************************************************************
*/
static U32 _GetBufferSize(int LayerIndex) {
  U32 BufferSize;

  BufferSize = _axSize[LayerIndex] * _aySize[LayerIndex] * _aBytesPerPixels[LayerIndex];
  return BufferSize;
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_CopyBuffer
*	功能说明: 此函数用于多缓冲，将一个缓冲中的所有数据复制到另一个缓冲。
*	形    参: LayerIndex    图层
*             IndexSrc      源缓冲序号
*             IndexDst      目标缓冲序号
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst) 
{
	U32 BufferSize, AddrSrc, AddrDst;

	BufferSize = _GetBufferSize(LayerIndex);
	AddrSrc = _aAddr[LayerIndex] + BufferSize * IndexSrc;
	AddrDst = _aAddr[LayerIndex] + BufferSize * IndexDst;

	_DMA_Copy(LayerIndex, (void *)AddrSrc, (void *)AddrDst, _axSize[LayerIndex], _aySize[LayerIndex], 0, 0);
	_aBufferIndex[LayerIndex] = IndexDst;  // After this function has been called all drawing operations are routed to Buffer[IndexDst]!
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_CopyRect
*	功能说明: 此函数用于多缓冲，将一个缓冲中指定区域数据复制到另一个缓冲。
*	形    参: LayerIndex    图层
*             x0            源缓冲x轴位置
*             y0            源缓冲y轴位置
*             x1            目标冲x轴位置
*             y1            目标冲y轴位置
*             xSize         要复制的x轴大小
*             ySize         要复制的y轴大小
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize) 
{
	U32 BufferSize, AddrSrc, AddrDst;
	int OffLine;

	BufferSize = _GetBufferSize(LayerIndex);
	AddrSrc = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y0 * _axSize[LayerIndex] + x0) * _aBytesPerPixels[LayerIndex];
	AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y1 * _axSize[LayerIndex] + x1) * _aBytesPerPixels[LayerIndex];
	OffLine = _axSize[LayerIndex] - xSize;
	_DMA_Copy(LayerIndex, (void *)AddrSrc, (void *)AddrDst, xSize, ySize, OffLine, OffLine);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_FillRect
*	功能说明: 对指定的区域进行颜色填充
*	形    参: LayerIndex    图层
*             x0            起始x轴位置
*             y0            起始y轴位置
*             x1            结束x轴位置
*             y1            结束y轴位置
*             PixelIndex    要填充的颜色值
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_FillRect(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex) {
  U32 BufferSize, AddrDst;
  int xSize, ySize;

  if (GUI_GetDrawMode() == GUI_DM_XOR) {
    LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, NULL);
    LCD_FillRect(x0, y0, x1, y1);
    LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, (void(*)(void))_LCD_FillRect);
  } else {
    xSize = x1 - x0 + 1;
    ySize = y1 - y0 + 1;
    BufferSize = _GetBufferSize(LayerIndex);
    AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y0 * _axSize[LayerIndex] + x0) * _aBytesPerPixels[LayerIndex];
    _DMA_Fill(LayerIndex, (void *)AddrDst, xSize, ySize, _axSize[LayerIndex] - xSize, PixelIndex);
  }
}

/*
*********************************************************************************************************
*	函 数 名: _DMA_DrawBitmapL8
*	功能说明: 对指定的区域进行颜色填充
*	形    参: LayerIndex    图层
*             x0            起始x轴位置
*             y0            起始y轴位置
*             x1            结束x轴位置
*             y1            结束y轴位置
*             PixelIndex    要填充的颜色值
*	返 回 值: 无
*********************************************************************************************************
*/
static void _DMA_DrawBitmapL8(void * pSrc, void * pDst,  U32 OffSrc, U32 OffDst, U32 PixelFormatDst, U32 xSize, U32 ySize) {
  //
  // Set up mode
  //
  DMA2D->CR      = 0x00010000UL | (1 << 9);         // Control Register (Memory to memory with pixel format conversion and TCIE)
  //
  // Set up pointers
  //
  DMA2D->FGMAR   = (U32)pSrc;                       // Foreground Memory Address Register (Source address)
  DMA2D->OMAR    = (U32)pDst;                       // Output Memory Address Register (Destination address)
  //
  // Set up offsets
  //
  DMA2D->FGOR    = OffSrc;                          // Foreground Offset Register (Source line offset)
  DMA2D->OOR     = OffDst;                          // Output Offset Register (Destination line offset)
  //
  // Set up pixel format
  //
  DMA2D->FGPFCCR = LTDC_Pixelformat_L8;             // Foreground PFC Control Register (Defines the input pixel format)
  DMA2D->OPFCCR  = PixelFormatDst;                  // Output PFC Control Register (Defines the output pixel format)
  //
  // Set up size
  //
  DMA2D->NLR     = (U32)(xSize << 16) | ySize;      // Number of Line Register (Size configuration of area to be transfered)
  //
  // Execute operation
  //
  DMA2D->CR     |= 1;                               // Start operation
  //
  // Wait until transfer is done
  //
  while (DMA2D->CR & DMA2D_CR_START) {
    //__WFI();                                        // Sleep until next interrupt
  }
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawBitmap8bpp
*	功能说明: 8bpp位图绘制
*	形    参: --
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DrawBitmap8bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine) {
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;
  U32 PixelFormat;

  BufferSize = _GetBufferSize(LayerIndex);
  AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
  OffLineSrc = BytesPerLine - xSize;
  OffLineDst = _axSize[LayerIndex] - xSize;
  PixelFormat = _GetPixelformat(LayerIndex);
  _DMA_DrawBitmapL8((void *)p, (void *)AddrDst, OffLineSrc, OffLineDst, PixelFormat, xSize, ySize);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawBitmap16bpp
*	功能说明: 16bpp位图绘制
*	形    参: --
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DrawBitmap16bpp(int LayerIndex, int x, int y, U16 const * p, int xSize, int ySize, int BytesPerLine) {
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;

  BufferSize = _GetBufferSize(LayerIndex);
  AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
  OffLineSrc = (BytesPerLine / 2) - xSize;
  OffLineDst = _axSize[LayerIndex] - xSize;
  _DMA_Copy(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DrawBitmap32bpp
*	功能说明: 32bpp位图绘制
*	形    参: --
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DrawBitmap32bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine)
{
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;

  BufferSize = _GetBufferSize(LayerIndex);
  AddrDst = _aAddr[LayerIndex] + BufferSize * _aBufferIndex[LayerIndex] + (y * _axSize[LayerIndex] + x) * _aBytesPerPixels[LayerIndex];
  OffLineSrc = (BytesPerLine / 4) - xSize;
  OffLineDst = _axSize[LayerIndex] - xSize;
  _DMA_Copy(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}


/*
*********************************************************************************************************
*	函 数 名: _LCD_DisplayOn
*	功能说明: 打开LCD
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DisplayOn(void) 
{
	// Enable LCD Backlight
	LCD_SetBackLight(255);
	
	// Display On
	LTDC_Cmd(ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_DisplayOff
*	功能说明: 关闭LCD
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_DisplayOff(void) 
{
	// Disable LCD Backlight
	LCD_SetBackLight(0);
	
	// Display Off
	LTDC_Cmd(DISABLE);
}

/*
*********************************************************************************************************
*	函 数 名: DMA2D_IRQHandler
*	功能说明: DMA2D传输完成中断服务程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DMA2D_IRQHandler(void) 
{
	DMA2D->IFCR = (U32)DMA2D_IFSR_CTCIF;
}

/*
*********************************************************************************************************
*	函 数 名: LTDC_IRQHandler
*	功能说明: LTDC帧中断，用于管理多缓冲
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LTDC_IRQHandler(void) 
{
	U32 Addr;
	int i;

	LTDC->ICR = (U32)LTDC_IER_LIE;
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		if (_aPendingBuffer[i] >= 0) 
		{
			//
			// Calculate address of buffer to be used as visible frame buffer
			//
			Addr = _aAddr[i] + _axSize[i] * _aySize[i] * _aPendingBuffer[i] * _aBytesPerPixels[i];
			
			//
			// Store address into SFR
			//
			_apLayer[i]->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);   
			_apLayer[i]->CFBAR = Addr;
			
			//
			// Reload configuration
			//
			LTDC_ReloadConfig(LTDC_SRCR_IMR);
			
			//
			// Tell emWin that buffer is used
			//
			GUI_MULTIBUF_ConfirmEx(i, _aPendingBuffer[i]);
			
			//
			// Clear pending buffer flag of layer
			//
			_aPendingBuffer[i] = -1;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: _LCD_InitController
*	功能说明: LCD初始化
*	形    参: LayerIndex  选择图层0或者1
*	返 回 值: 无
*********************************************************************************************************
*/
static void _LCD_InitController(int LayerIndex) 
{
	LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct = {0};
	int xSize, ySize, BytesPerLine, BitsPerPixel, i;
	U32 Pixelformat, Color;
	static int Done;

	if (LayerIndex >= GUI_COUNTOF(_apLayer)) 
	{
		return;
	}
	
	if (Done == 0) 
	{
		Done = 1;

		//
		// Enable line interrupt
		//
		LTDC_ITConfig(LTDC_IER_LIE, ENABLE);
		NVIC_SetPriority(LTDC_IRQn, 0);
		NVIC_EnableIRQ(LTDC_IRQn);
		
		#if emWin_Optimize 
			//
			// Enable DMA2D transfer complete interrupt
			//
			DMA2D_ITConfig(DMA2D_CR_TCIE, ENABLE);
			NVIC_SetPriority(DMA2D_IRQn, 0);
			NVIC_EnableIRQ(DMA2D_IRQn);
			//
			// Clear transfer complete interrupt flag
			//
			DMA2D->IFCR = (U32)DMA2D_IFSR_CTCIF;
		#endif
	}
	
	//
	// Layer configuration
	//
	xSize = LCD_GetXSizeEx(LayerIndex);
	ySize = LCD_GetYSizeEx(LayerIndex);

	// HorizontalStart = (Offset_X + Hsync + HBP);
    // HorizontalStop  = (Offset_X + Hsync + HBP + Window_Width - 1); 
    // VarticalStart   = (Offset_Y + Vsync + VBP);
    // VerticalStop    = (Offset_Y + Vsync + VBP + Window_Heigh - 1);
	
	LTDC_Layer_InitStruct.LTDC_HorizontalStart = HSYNC_W + HBP + 1;
	LTDC_Layer_InitStruct.LTDC_HorizontalStop = (Width + LTDC_Layer_InitStruct.LTDC_HorizontalStart - 1);
	LTDC_Layer_InitStruct.LTDC_VerticalStart = VSYNC_W + VBP + 1; 
	LTDC_Layer_InitStruct.LTDC_VerticalStop = (Height + LTDC_Layer_InitStruct.LTDC_VerticalStart - 1);

	//
	// Pixel Format configuration
	//
	Pixelformat = _GetPixelformat(LayerIndex);
	LTDC_Layer_InitStruct.LTDC_PixelFormat = Pixelformat;
	
	//
	// Alpha constant (255 totally opaque)
	//
	LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255;
	
	//
	// Default Color configuration (configure A, R, G, B component values)
	//
	LTDC_Layer_InitStruct.LTDC_DefaultColorBlue  = 0;
	LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;
	LTDC_Layer_InitStruct.LTDC_DefaultColorRed   = 0;
	LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;
	
	//
	// Configure blending factors
	//
	BytesPerLine = _GetBytesPerLine(LayerIndex, xSize);
	LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_PAxCA;
	LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_PAxCA;
	LTDC_Layer_InitStruct.LTDC_CFBLineLength    = BytesPerLine + 3;
	LTDC_Layer_InitStruct.LTDC_CFBPitch         = BytesPerLine;
	LTDC_Layer_InitStruct.LTDC_CFBLineNumber    = ySize;
	
	//
	// Input Address configuration
	//
	LTDC_Layer_InitStruct.LTDC_CFBStartAdress = _aAddr[LayerIndex];
	LTDC_LayerInit(_apLayer[LayerIndex], &LTDC_Layer_InitStruct);
	
	//
	// Enable LUT on demand
	//
	BitsPerPixel = LCD_GetBitsPerPixelEx(LayerIndex);
	if (BitsPerPixel <= 8) 
	{
		//
		// Enable usage of LUT for all modes with <= 8bpp
		//
		_LTDC_LayerEnableLUT(_apLayer[LayerIndex], ENABLE);
		
		//
		// Optional CLUT initialization for L8 mode (8bpp)
		//
		if (_apColorConvAPI[LayerIndex] == GUICC_1616I) 
		{
			for (i = 0; i < 16; i++) 
			{
				Color = LCD_API_ColorConv_1616I.pfIndex2Color(i);
				_LTDC_SetLUTEntry(LayerIndex, Color, i);
			}			
		}

		//
		// Optional CLUT initialization for AL44 mode (8bpp)
		//
		if (_apColorConvAPI[LayerIndex] == GUICC_8666) 
		{
			for (i = 0; i < 16; i++) 
			{
				Color = LCD_API_ColorConv_8666.pfIndex2Color(i);
				_LTDC_SetLUTEntry(LayerIndex, Color, i);
			}			
		}
	} 
	else 
	{
		//
		// Optional CLUT initialization for AL88 mode (16bpp)
		//
		if (_apColorConvAPI[LayerIndex] == GUICC_88666I) 
		{
			_LTDC_LayerEnableLUT(_apLayer[LayerIndex], ENABLE);
			for (i = 0; i < 256; i++) 
			{
				Color = LCD_API_ColorConv_8666.pfIndex2Color(i);
				_LTDC_SetLUTEntry(LayerIndex, Color, i);
			}
		}
	}
	
	//
	// Enable layer
	//
	LTDC_LayerCmd(_apLayer[LayerIndex], ENABLE);

	//
	// Reload configuration
	//
	LTDC_ReloadConfig(LTDC_SRCR_IMR);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_X_Config
*	功能说明: LCD配置
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_X_Config(void) 
{
	int i;

	//
	// At first initialize use of multiple buffers on demand
	//
	#if (NUM_BUFFERS > 1)
		for (i = 0; i < GUI_NUM_LAYERS; i++) 
		{
			GUI_MULTIBUF_ConfigEx(i, NUM_BUFFERS);
		}
	#endif
		
	//
	// Set display driver and color conversion for 1st layer
	//
	GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_0, COLOR_CONVERSION_0, 0, 0);
		
	//
	// Set size of 1st layer
	//
	LCD_SetSizeEx (0, g_LcdWidth, g_LcdHeight);
	LCD_SetVSizeEx(0, g_LcdWidth, g_LcdHeight * NUM_VSCREENS);
	#if (GUI_NUM_LAYERS > 1)
		//
		// Set display driver and color conversion for 2nd layer
		//
		GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_1, COLOR_CONVERSION_1, 0, 1);
		
		//
		// Set size of 2nd layer
		//
		LCD_SetSizeEx (1, g_LcdWidth, g_LcdHeight);
		LCD_SetVSizeEx(1, g_LcdWidth, g_LcdHeight * NUM_VSCREENS);
	#endif
	
	//
	// Setting up VRam address and custom functions for CopyBuffer-, CopyRect- and FillRect operations
	//
	for (i = 0; i < GUI_NUM_LAYERS; i++) 
	{
		_aPendingBuffer[i] = -1;
		
		//
		// Set VRAM address
		//
		LCD_SetVRAMAddrEx(i, (void *)(_aAddr[i]));
		
		//
		// Remember color depth for further operations
		//
		_aBytesPerPixels[i] = LCD_GetBitsPerPixelEx(i) >> 3;
		
		//
		// Set custom functions for several operations
		//
		LCD_SetDevFunc(i, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))_LCD_CopyBuffer);
		LCD_SetDevFunc(i, LCD_DEVFUNC_COPYRECT,   (void(*)(void))_LCD_CopyRect);
		
		//
		// Filling via DMA2D does only work with 16bpp or more
		//
		if (_GetPixelformat(i) <= LTDC_Pixelformat_ARGB4444) 
		{
			LCD_SetDevFunc(i, LCD_DEVFUNC_FILLRECT, (void(*)(void))_LCD_FillRect);
			LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_8BPP, (void(*)(void))_LCD_DrawBitmap8bpp); 
		}
		
		//
		// Set up drawing routine for 16bpp bitmap using DMA2D
		//
		if (_GetPixelformat(i) == LTDC_Pixelformat_RGB565) 
		{
			LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_16BPP, (void(*)(void))_LCD_DrawBitmap16bpp);     // Set up drawing routine for 16bpp bitmap using DMA2D. Makes only sense with RGB565
		}

		//
		// Set up drawing routine for 32bpp bitmap using DMA2D
		//
		if (_GetPixelformat(i) == LTDC_Pixelformat_ARGB8888) 
		{
			LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_32BPP, (void(*)(void))_LCD_DrawBitmap32bpp);     // Set up drawing routine for 16bpp bitmap using DMA2D. Makes only sense with RGB565
		}

		//
		// Set up custom color conversion using DMA2D, works only for direct color modes because of missing LUT for DMA2D destination
		//
		GUICC_M1555I_SetCustColorConv(_Color2IndexBulk_M1555I_DMA2D, _Index2ColorBulk_M1555I_DMA2D); // Set up custom bulk color conversion using DMA2D for ARGB1555
		GUICC_M565_SetCustColorConv  (_Color2IndexBulk_M565_DMA2D,   _Index2ColorBulk_M565_DMA2D);   // Set up custom bulk color conversion using DMA2D for RGB565
		GUICC_M4444I_SetCustColorConv(_Color2IndexBulk_M4444I_DMA2D, _Index2ColorBulk_M4444I_DMA2D); // Set up custom bulk color conversion using DMA2D for ARGB4444
		GUICC_M888_SetCustColorConv  (_Color2IndexBulk_M888_DMA2D,   _Index2ColorBulk_M888_DMA2D);   // Set up custom bulk color conversion using DMA2D for RGB888
		GUICC_M8888I_SetCustColorConv(_Color2IndexBulk_M8888I_DMA2D, _Index2ColorBulk_M8888I_DMA2D); // Set up custom bulk color conversion using DMA2D for ARGB8888

		//
		// Set up custom alpha blending function using DMA2D
		//
		GUI_SetFuncAlphaBlending(_DMA_AlphaBlending); 
		
		//
		// Set up custom function for translating a bitmap palette into index values.
		// Required to load a bitmap palette into DMA2D CLUT in case of a 8bpp indexed bitmap
		//
		GUI_SetFuncGetpPalConvTable(_LCD_GetpPalConvTable);
		
		//
		// Set up a custom function for mixing up single colors using DMA2D
		//
		#if emWin_Optimize 
			GUI_SetFuncMixColors(_DMA_MixColors);
		#endif
		
		//
		// Set up a custom function for mixing up arrays of colors using DMA2D
		//
		GUI_SetFuncMixColorsBulk(_LCD_MixColorsBulk);
	}
}

/*
*********************************************************************************************************
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*********************************************************************************************************
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) 
{
	int r = 0;

	switch (Cmd) 
	{
		case LCD_X_INITCONTROLLER: 
			{
				//
				// Called during the initialization process in order to set up the display controller and put it into operation.
				//
				_LCD_InitController(LayerIndex);
				break;
			}
			
		case LCD_X_SETORG: 
			{
				//
				// Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
				//
				LCD_X_SETORG_INFO * p;

				p = (LCD_X_SETORG_INFO *)pData;
				_apLayer[LayerIndex]->CFBAR = _aAddr[LayerIndex] + p->yPos * _axSize[LayerIndex] * _aBytesPerPixels[LayerIndex];
				LTDC_ReloadConfig(LTDC_SRCR_VBR); // Reload on next blanking period
				break;
			}
			
		case LCD_X_SHOWBUFFER: 
			{
				//
				// Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
				//
				LCD_X_SHOWBUFFER_INFO * p;

				p = (LCD_X_SHOWBUFFER_INFO *)pData;
				_aPendingBuffer[LayerIndex] = p->Index;
				break;
			}
			
		case LCD_X_SETLUTENTRY: 
			{
				//
				// Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
				//
				LCD_X_SETLUTENTRY_INFO * p;

				p = (LCD_X_SETLUTENTRY_INFO *)pData;
				_LTDC_SetLUTEntry(LayerIndex, p->Color, p->Pos);
				break;
			}
		case LCD_X_ON: 
			{
				//
				// Required if the display controller should support switching on and off
				//
				_LCD_DisplayOn();
				break;
			}
			
		case LCD_X_OFF:
			{
				//
				// Required if the display controller should support switching on and off
				//
				_LCD_DisplayOff();
				break;
			}
			
		case LCD_X_SETVIS:
			{
				//
				// Required for setting the layer visibility which is passed in the 'OnOff' element of pData
				//
				LCD_X_SETVIS_INFO * p;

				p = (LCD_X_SETVIS_INFO *)pData;
				LTDC_LayerCmd(_apLayer[LayerIndex], p->OnOff ? ENABLE : DISABLE);

				/* Reload shadow register */
				LTDC_ReloadConfig(LTDC_SRCR_IMR);
				break;
			}
			
		case LCD_X_SETPOS: 
			{
				//
				// Required for setting the layer position which is passed in the 'xPos' and 'yPos' element of pData
				//
				LCD_X_SETPOS_INFO * p;

				p = (LCD_X_SETPOS_INFO *)pData;
				_LTDC_SetLayerPos(LayerIndex, p->xPos, p->yPos);
				break;
			}
			
		case LCD_X_SETSIZE: 
			{
				//
				// Required for setting the layer position which is passed in the 'xPos' and 'yPos' element of pData
				//
				LCD_X_SETSIZE_INFO * p;
				int xPos, yPos;

				GUI_GetLayerPosEx(LayerIndex, &xPos, &yPos);
				p = (LCD_X_SETSIZE_INFO *)pData;
				_axSize[LayerIndex] = p->xSize;
				_aySize[LayerIndex] = p->ySize;
				_LTDC_SetLayerPos(LayerIndex, xPos, yPos);
				break;
			}
			
		case LCD_X_SETALPHA: 
			{
				//
				// Required for setting the alpha value which is passed in the 'Alpha' element of pData
				//
				LCD_X_SETALPHA_INFO * p;

				p = (LCD_X_SETALPHA_INFO *)pData;
				_LTDC_SetLayerAlpha(LayerIndex, p->Alpha);
				break;
			}
			
		case LCD_X_SETCHROMAMODE: 
			{
				//
				// Required for setting the chroma mode which is passed in the 'ChromaMode' element of pData
				//
				LCD_X_SETCHROMAMODE_INFO * p;

				p = (LCD_X_SETCHROMAMODE_INFO *)pData;
				_LTDC_LayerEnableColorKeying(_apLayer[LayerIndex], (p->ChromaMode != 0) ? ENABLE : DISABLE);
				break;
			}
			
		case LCD_X_SETCHROMA: 
			{
				//
				// Required for setting the chroma value which is passed in the 'ChromaMin' and 'ChromaMax' element of pData
				//
				LCD_X_SETCHROMA_INFO * p;
				U32 Color;

				p = (LCD_X_SETCHROMA_INFO *)pData;
				Color = ((p->ChromaMin & 0xFF0000) >> 16) | (p->ChromaMin & 0x00FF00) | ((p->ChromaMin & 0x0000FF) << 16);
				_apLayer[LayerIndex]->CKCR = Color;
				LTDC_ReloadConfig(LTDC_SRCR_VBR); // Reload on next blanking period
				break;
			}
		
		default:
			r = -1;
	}
	
	return r;
}

/*************************** End of file ****************************/
