/*
*********************************************************************************************************
*	                                  
*	模块名称 : GUI界面主函数
*	文件名称 : MainTask.c
*	版    本 : V1.0
*	说    明 : GUI界面主函数
*		版本号   日期         作者            说明
*		v1.0    2015-08-05  Eric2013  	      首版
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __MainTask_H
#define __MainTask_H

#include "GUI.h"
#include "DIALOG.h"
#include "WM.h"
#include "BUTTON.h"
#include "CHECKBOX.h"
#include "DROPDOWN.h"
#include "EDIT.h"
#include "FRAMEWIN.h"
#include "LISTBOX.h"
#include "MULTIEDIT.h"
#include "RADIO.h"
#include "SLIDER.h"
#include "TEXT.h"
#include "PROGBAR.h"
#include "SCROLLBAR.h"
#include "LISTVIEW.h"
#include "GRAPH.h"
#include "MENU.h"
#include "MULTIPAGE.h"
#include "ICONVIEW.h"
#include "TREEVIEW.h"

#include "ff.h"
#include "can_network.h"
#include "demo_fatfs.h"

#define MSG_CreateInfo      (GUI_ID_USER + 0x0E)
#define MSG_DeleteInfo      (GUI_ID_USER + 0x0F)

/*
************************************************************************
*						  FatFs
************************************************************************
*/
extern FRESULT result;
extern FIL file;
extern DIR DirInf_usb;
extern UINT bw;
extern FATFS fs_nand;
extern FATFS fs_usb;

extern void _WriteByte2File(U8 Data, void * p); 
/*
************************************************************************
*						供外部文件调用
************************************************************************
*/
extern void MainTask(void);
extern void TOUCH_Calibration(void);
int getChData(char dataNum);
struct _CONFIG_CHANNEL_ getChannel(char chNum);
//void saveVCT(WM_MESSAGE * pMsg);

#endif

/*****************************(END OF FILE) *********************************/
