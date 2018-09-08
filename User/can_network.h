/*
*********************************************************************************************************
*	                                  
*	模块名称 : CAN网络演示程序。
*	文件名称 : can_network.h
*	版    本 : V1.3
*	说    明 : 头文件
*	修改记录 :
*		版本号  日期       作者      说明
*		v1.3    2015-01-29 Eric2013  首发
*
*	Copyright (C), 2015-2016, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _CAN_NETWORK_H
#define _CAN_NETWORK_H

/* 供外部调用的函数声明 */
void can1_Init(void);
void can1_NVIC_Config(void);
void can2_Init(void);
void can2_NVIC_Config(void);
void can_demo(void);
void CAN_SendMessage(unsigned int ID,unsigned char *array);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
