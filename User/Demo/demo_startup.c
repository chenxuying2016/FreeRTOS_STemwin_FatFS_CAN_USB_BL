/**
  ******************************************************************************
  * @file    demo_startup.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2013
  * @brief   Demo startup
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "DIALOG.h"
#include "Res\startup_res.c"
#include "Res\tishi.c"
#include "Res\Logo-Cell.c"
#include "Res\Logo-Name.c"
#include "Res\project-name.c"

//extern uint8_t jiexiwan_flag;
//extern GUI_CONST_STORAGE GUI_FONT GUI_Fontyahei48;
//extern GUI_CONST_STORAGE GUI_FONT GUI_Fontsong24;

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

static uint32_t idx = 0;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Background callback
  * @param  pMsg: pointer to a data structure of type WM_MESSAGE
  * @retval None
  */
static void _cbBk(WM_MESSAGE * pMsg) {
  uint16_t   xPos, Step = 20, i;
  const GUI_BITMAP * pBm;
  
  switch (pMsg->MsgId) 
  {
		case WM_PAINT:	
//			GUI_SetBkColor(GUI_BLACK);
//			GUI_Clear();
//			GUI_SetColor(GUI_WHITE);
			
			GUI_DrawGradientV(0, 0, LCD_GetXSize()-1, LCD_GetYSize() - 1, GUI_BLACK, GUI_BLACK);
//			GUI_SetColor(GUI_WHITE);
//			GUI_SetFont(&GUI_Fontsong24);
//			GUI_DispStringAt("≈‰÷√≤Œ ˝", LCD_GetXSize() / 2 - 16*8, LCD_GetYSize() - 50);
			GUI_DrawBitmap(&bmLogoCell, (LCD_GetXSize() - bmLogoCell .XSize)/2 , LCD_GetYSize()/2 - bmLogoCell.YSize - 60);
			GUI_DrawBitmap(&bmprojectname, (LCD_GetXSize() - bmprojectname .XSize)/2 , (LCD_GetYSize() - bmprojectname.YSize)/2);
			GUI_DrawBitmap(&bmtishi, (LCD_GetXSize() - bmtishi .XSize)/2 , (LCD_GetYSize()/2 + bmprojectname.YSize + 10));
		
			GUI_DrawBitmap(&bmLogoName, (LCD_GetXSize() - bmLogoName .XSize)/2 , LCD_GetYSize() - bmLogoName.YSize - 25);
			GUI_SetFont(GUI_FONT_16_1);
			GUI_DispStringAt("@Freesense Image", LCD_GetXSize() / 2 - 8*8 + 4, LCD_GetYSize() - 20);
//			GUI_DrawBitmap(&bmlogo0, (LCD_GetXSize() - bmlogo0 .XSize)/2 , (LCD_GetYSize() - bmlogo0.YSize-10));
			
//			for (i = 0, xPos = LCD_GetXSize() / 2 - 2 * Step; i < 5; i++, xPos += Step) 
//			{
//				pBm = (idx == i) ? &_bmWhiteCircle_10x10 : &_bmWhiteCircle_6x6;
//				GUI_DrawBitmap(pBm, xPos - pBm->XSize / 2, 360 - pBm->YSize / 2);
//			}
			break; 
			
//	case WM_TIMER:
//			WriteFileTest();
//		break;
  }
}

/**
  * @brief  DEMO_Starup
  * @param  None
  * @retval None
  */
void DEMO_Starup(void)
{
	WM_HWIN hText;
  uint8_t loop = 15 ;
  GUI_RECT Rect = {360, 355, 440, 365}; 
  
  WM_SetCallback(WM_HBKWIN, _cbBk);
//	hText = TEXT_CreateEx(0, 10, 100, 26, WM_HBKWIN, WM_CF_SHOW, 0, GUI_ID_TEXT0, "The client window is");
//	TEXT_SetTextColor(hText, GUI_WHITE);
	
//	WM_CreateTimer(WM_HBKWIN, ID_Timer2, 10,0);	
	
	DemoFatFS();
  
  while (loop--)
  {
//    idx = (215- loop) % 5;
//    
//    WM_InvalidateArea(&Rect);
    
//		if(jiexiwan_flag)
//			break;
//		if(loop == 0)
//			loop = 255 ;
		
    GUI_Delay(200);
  }
}
/*************************** End of file ****************************/
