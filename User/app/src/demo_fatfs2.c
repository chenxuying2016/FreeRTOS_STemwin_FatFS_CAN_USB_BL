/*
*********************************************************************************************************
*
*	模块名称 : U盘Fat文件系统演示模块。
*	文件名称 : demo_fatfs.c
*	版    本 : V1.0
*	说    明 : 该例程移植FatFS文件系统（版本 R0.11a），演示如何创建文件、读取文件、创建目录和删除文件
*			并测试了文件读写速度。
*           使用说明：
*           1. 多个盘符操作的时候，务必时刻保证加上盘符号，要不容易无法正确识别。只有一个盘符的时候可以
*              不加盘符号。
*           2. 支持热插拔。
*
*	修改记录 :
*		版本号   日期        作者       说明
*		V1.0    2016-02-27 	 Eric2013  正式发布
*
*	Copyright (C), 2016-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "includes.h"
#include "ff.h"			/* FatFS文件系统模块*/
#include "usbh_bsp_msc.h"
#include "demo_fatfs.h"


#define name_to_str(name_struct)  (#name_struct)

//extern char usb_flag;
uint8_t jiexiwan_flag=0;

BL_FPGA1 BL_fpga1 ;
FPGA1_STV fpga1_STV1;   //1到5套时序参数
FPGA1_STV fpga1_STV2;   //第六到第10套时序参数
FPGA1_STV fpga1_STV3;   //第11套到第15套时序参数
FPGA1_RGB_N fpga1_rgb_n;
extern FIL MyFiles;

/* 仅允许本文件内调用的函数声明 */
static void DispMenu(void);
//static void CreateNewFile(uint8_t s_ucTestSn,char *_fileName,void *buf,uint16_t len);
static void CreateNewFile(uint8_t s_ucTestSn);
static void DeleteDirFile(char *_ucVolume);
static void MSC_App3();

/* FatFs API的返回值 */
static const char * FR_Table[]= 
{
	"FR_OK：成功",				                             /* (0) Succeeded */
	"FR_DISK_ERR：底层硬件错误",			                 /* (1) A hard error occurred in the low level disk I/O layer */
	"FR_INT_ERR：断言失败",				                     /* (2) Assertion failed */
	"FR_NOT_READY：物理驱动没有工作",			             /* (3) The physical drive cannot work */
	"FR_NO_FILE：文件不存在",				                 /* (4) Could not find the file */
	"FR_NO_PATH：路径不存在",				                 /* (5) Could not find the path */
	"FR_INVALID_NAME：无效文件名",		                     /* (6) The path name format is invalid */
//	"FR_DENIED：由于禁止访问或者目录已满访问被拒绝",         /* (7) Access denied due to prohibited access or directory full */
//	"FR_EXIST：文件已经存在",			                     /* (8) Access denied due to prohibited access */
//	"FR_INVALID_OBJECT：文件或者目录对象无效",		         /* (9) The file/directory object is invalid */
//	"FR_WRITE_PROTECTED：物理驱动被写保护",		             /* (10) The physical drive is write protected */
//	"FR_INVALID_DRIVE：逻辑驱动号无效",		                 /* (11) The logical drive number is invalid */
//	"FR_NOT_ENABLED：卷中无工作区",			                 /* (12) The volume has no work area */
//	"FR_NO_FILESYSTEM：没有有效的FAT卷",		             /* (13) There is no valid FAT volume */
//	"FR_MKFS_ABORTED：由于参数错误f_mkfs()被终止",	         /* (14) The f_mkfs() aborted due to any parameter error */
//	"FR_TIMEOUT：在规定的时间内无法获得访问卷的许可",		 /* (15) Could not get a grant to access the volume within defined period */
//	"FR_LOCKED：由于文件共享策略操作被拒绝",				 /* (16) The operation is rejected according to the file sharing policy */
//	"FR_NOT_ENOUGH_CORE：无法分配长文件名工作区",		     /* (17) LFN working buffer could not be allocated */
//	"FR_TOO_MANY_OPEN_FILES：当前打开的文件数大于_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
//	"FR_INVALID_PARAMETER：参数无效"	                     /* (19) Given parameter is invalid */
};

/*
*********************************************************************************************************
*	函 数 名: DemoFatFS
*	功能说明: FatFS文件系统演示主程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoFatFS(void)
{
	uint8_t cmd;
	uint32_t i=0;
	FRESULT result;
	FATFS fs;
//	FIL file;
	DIR DirInf;
	char path[20]={0};
	/* 打印命令列表，用户可以通过串口操作指令 */
//	DispMenu();
	
	/* 为了防止上电前U盘已经插入，造成挂载失败，加上调度锁，使用FreeRTOS发现没有这个问题，
	   所以此处没加 。
	*/

	while (1)
	{
//			result = f_mount(&fs, FS_VOLUME_USB, 0);			/* Mount a logical drive */
//			if (result != FR_OK)
//			{
//				printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
//			}

//			sprintf(path, "%s/", FS_VOLUME_USB);
//			result = f_opendir(&DirInf, path); /* 如果不带参数，则从当前目录开始 */
//			if (result != FR_OK)
//			{
//				printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
//				return;
//			}

			sprintf(path, "%s/Cell.TXT", FS_VOLUME_USB);
			result = f_open(&MyFiles, path, FA_OPEN_EXISTING | FA_READ);
			if (result !=  FR_OK)
			{
				printf("Don't Find File : %s\r\n",path);
				return;
			}

			vTaskPrioritySet(NULL, 7);
			MSC_App3();
			vTaskPrioritySet(NULL, 1);

			/* 关闭文件*/
			f_close(&MyFiles);
				
//			/* 卸载文件系统 */
//			f_mount(NULL, "2:", 0);

			break;
		}
	
		vTaskDelay(10);
}

/*
*********************************************************************************************************
*	函 数 名: DispMenu
*	功能说明: 显示操作提示菜单
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispMenu(void)
{
	printf("\r\n------------------------------------------------\r\n");
	printf("请选择操作命令:\r\n");
	printf("1 - 显示根目录下的文件列表\r\n");
	printf("2 - 创建一个新文件armfly.txt\r\n");
	printf("3 - 读armfly.txt文件的内容\r\n");
	printf("4 - 创建目录\r\n");
	printf("5 - 删除文件和目录\r\n");
	printf("6 - 读写文件速度测试\r\n");
	printf("7 - 挂载U盘\r\n");
	printf("8 - 卸载U盘\r\n");
}

/*
*********************************************************************************************************
*	函 数 名: CreateNewFile
*	功能说明: 在SD卡创建一个新文件，文件内容填写“www.armfly.com”
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CreateNewFile(uint8_t s_ucTestSn)
{
	/* 本函数使用的局部变量占用较多，请修改任务栈大小，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char ProjectFileName[30];

 	/* 挂载文件系统 */
	result = f_mount(&fs, "0:", 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "0:/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
		return;
	}

	/* 打开文件 */
//	sprintf(ProjectFileName, "0:/Project%02d%s.txt", s_ucTestSn, _fileName);
	sprintf(ProjectFileName, "0:/Project%02d%s.txt", s_ucTestSn, name_to_str(BL_fpga1));
	result = f_open(&file, ProjectFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
//	result = f_write(&file, buf, len, &bw);
	result = f_write(&file, &BL_fpga1, sizeof(BL_fpga1), &bw);
	if (result == FR_OK)
	{
		printf("%s 文件写入成功\r\n",ProjectFileName);
	}
	else
	{
		printf("%s 文件写入失败  (%s)\r\n", ProjectFileName, FR_Table[result]);
	}
	f_close(&file);
	
	sprintf(ProjectFileName, "0:/Project%02d%s.txt", s_ucTestSn, name_to_str(fpga1_STV1));
	result = f_open(&file, ProjectFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
	result = f_write(&file, &fpga1_STV1, sizeof(fpga1_STV1), &bw);
	if (result == FR_OK)
	{
		printf("%s 文件写入成功\r\n",ProjectFileName);
	}
	else
	{
		printf("%s 文件写入失败  (%s)\r\n", ProjectFileName, FR_Table[result]);
	}
	f_close(&file);
	
	sprintf(ProjectFileName, "0:/Project%02d%s.txt", s_ucTestSn, name_to_str(fpga1_STV2));
	result = f_open(&file, ProjectFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
	result = f_write(&file, &fpga1_STV2, sizeof(fpga1_STV2), &bw);
	if (result == FR_OK)
	{
		printf("%s 文件写入成功\r\n",ProjectFileName);
	}
	else
	{
		printf("%s 文件写入失败  (%s)\r\n", ProjectFileName, FR_Table[result]);
	}
	f_close(&file);
	
	sprintf(ProjectFileName, "0:/Project%02d%s.txt", s_ucTestSn, name_to_str(fpga1_STV3));
	result = f_open(&file, ProjectFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
	result = f_write(&file, &fpga1_STV3, sizeof(fpga1_STV3), &bw);
	if (result == FR_OK)
	{
		printf("%s 文件写入成功\r\n",ProjectFileName);
	}
	else
	{
		printf("%s 文件写入失败  (%s)\r\n", ProjectFileName, FR_Table[result]);
	}
	f_close(&file);
	
	sprintf(ProjectFileName, "0:/Project%02d%s.txt", s_ucTestSn, name_to_str(fpga1_rgb_n));
	result = f_open(&file, ProjectFileName, FA_CREATE_ALWAYS | FA_WRITE);

	/* 写一串数据 */
	result = f_write(&file, &fpga1_rgb_n, sizeof(fpga1_rgb_n), &bw);
	if (result == FR_OK)
	{
		printf("%s 文件写入成功\r\n",ProjectFileName);
	}
	else
	{
		printf("%s 文件写入失败  (%s)\r\n", ProjectFileName, FR_Table[result]);
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	 f_mount(NULL, "0:", 0);
}

int strCompare( char *dest)                                                                                                                                                                                                                                                                             
{                                                                                                                                                                                                                                                                                                                 
	int i = 0;                                                                                                                                                                                                                                                                                                   
	int flag = 0;                                                                                                                                                                                                                                                                                                   
	char *strCmd[3468];                                                                                                                                                                                                                                                                                             
   
	{	
	strCmd[0]="Project_Name";		//name user define//*************BL BOARD CONFIG*************             
	strCmd[1]="CFG_Part1";			//??,????                                                          
	strCmd[2]="VBL";   				//??????(V)                                                           
	strCmd[3]="IBL";   				//??????(mA)                                                          
	strCmd[4]="IBL_LIMIT";   	//??????(mA) //*************FPGA1 BOARD CONFIG*************           
	strCmd[5]="CFG_Part2"; //??,????                                                               
	strCmd[6]="VGL1";       //(V)                                                                         
	strCmd[7]="VGH1";       //(V)                                                                         
	strCmd[8]="VGG1";       //(V)                                                                         
	strCmd[9]="VSW1"; 			//(V)                                                                         
	strCmd[10]="VCOM1_H";      //(V)                                                                        
	strCmd[11]="VBW1";       //(V)                                                                        
	strCmd[12]="VFW1"; 			//(V)                                                                         
	strCmd[13]="VTIMING_H1"; //(V)                                                                        
	strCmd[14]="VTIMING_L1"; //(V) //*************STV1 TIMING CONFIG*************  

	strCmd[15]="FPIC1_NAMEE";  //picture name user define                                                 
	strCmd[16]="FPIC1_TIMEE";  //????(s)                                                              
	strCmd[17]="FPIC1_R1_VH"; //R1????                                                               
	strCmd[18]="FPIC1_R1_VL"; //R1????                                                               
	strCmd[19]="FPIC1_G1_VH"; //G1????                                                               
	strCmd[20]="FPIC1_G1_VL"; //G1????                                                               
	strCmd[21]="FPIC1_B1_VH"; //B1????                                                               
	strCmd[22]="FPIC1_B1_VL"; //B1????                                                               
	strCmd[23]="FPIC1_R2_VH"; //R2????                                                               
	strCmd[24]="FPIC1_R2_VL"; //R2????                                                               
	strCmd[25]="FPIC1_G2_VH"; //G2????                                                               
	strCmd[26]="FPIC1_G2_VL"; //G2????                                                               
	strCmd[27]="FPIC1_B2_VH"; //B2????                                                               
	strCmd[28]="FPIC1_B2_VL"; //B2????    
	strCmd[29]="FPIC1_R3_VH"; //R1????                                                               
	strCmd[30]="FPIC1_R3_VL"; //R1????                                                               
	strCmd[31]="FPIC1_G3_VH"; //G1????                                                               
	strCmd[32]="FPIC1_G3_VL"; //G1????                                                               
	strCmd[33]="FPIC1_B3_VH"; //B1????                                                               
	strCmd[34]="FPIC1_B3_VL"; //B1????                                                               
	strCmd[35]="FPIC1_R4_VH"; //R2????                                                               
	strCmd[36]="FPIC1_R4_VL"; //R2????                                                               
	strCmd[37]="FPIC1_G4_VH"; //G2????                                                               
	strCmd[38]="FPIC1_G4_VL"; //G2????                                                               
	strCmd[39]="FPIC1_B4_VH"; //B2????                                                               
	strCmd[40]="FPIC1_B4_VL"; //B2????    

	strCmd[41]="FPIC2_NAMEE";  //picture name user define                                                 
	strCmd[42]="FPIC2_TIMEE";  //????(s)                                                              
	strCmd[43]="FPIC2_R1_VH"; //R1????                                                               
	strCmd[44]="FPIC2_R1_VL"; //R1????                                                               
	strCmd[45]="FPIC2_G1_VH"; //G1????                                                               
	strCmd[46]="FPIC2_G1_VL"; //G1????                                                               
	strCmd[47]="FPIC2_B1_VH"; //B1????                                                               
	strCmd[48]="FPIC2_B1_VL"; //B1????                                                               
	strCmd[49]="FPIC2_R2_VH"; //R2????                                                               
	strCmd[50]="FPIC2_R2_VL"; //R2????                                                               
	strCmd[51]="FPIC2_G2_VH"; //G2????                                                               
	strCmd[52]="FPIC2_G2_VL"; //G2????                                                               
	strCmd[53]="FPIC2_B2_VH"; //B2????                                                               
	strCmd[54]="FPIC2_B2_VL"; //B2????    
	strCmd[55]="FPIC2_R3_VH"; //R1????                                                               
	strCmd[56]="FPIC2_R3_VL"; //R1????                                                               
	strCmd[57]="FPIC2_G3_VH"; //G1????                                                               
	strCmd[58]="FPIC2_G3_VL"; //G1????                                                               
	strCmd[59]="FPIC2_B3_VH"; //B1????                                                               
	strCmd[60]="FPIC2_B3_VL"; //B1????                                                               
	strCmd[61]="FPIC2_R4_VH"; //R2????                                                               
	strCmd[62]="FPIC2_R4_VL"; //R2????                                                               
	strCmd[63]="FPIC2_G4_VH"; //G2????                                                               
	strCmd[64]="FPIC2_G4_VL"; //G2????                                                               
	strCmd[65]="FPIC2_B4_VH"; //B2????                                                               
	strCmd[66]="FPIC2_B4_VL"; //B2????      

	strCmd[67]="FPIC3_NAMEE";  //picture name user define                                                 
	strCmd[68]="FPIC3_TIMEE";  //????(s)                                                              
	strCmd[69]="FPIC3_R1_VH"; //R1????                                                               
	strCmd[70]="FPIC3_R1_VL"; //R1????                                                               
	strCmd[71]="FPIC3_G1_VH"; //G1????                                                               
	strCmd[72]="FPIC3_G1_VL"; //G1????                                                               
	strCmd[73]="FPIC3_B1_VH"; //B1????                                                               
	strCmd[74]="FPIC3_B1_VL"; //B1????                                                               
	strCmd[75]="FPIC3_R2_VH"; //R2????                                                               
	strCmd[76]="FPIC3_R2_VL"; //R2????                                                               
	strCmd[77]="FPIC3_G2_VH"; //G2????                                                               
	strCmd[78]="FPIC3_G2_VL"; //G2????                                                               
	strCmd[79]="FPIC3_B2_VH"; //B2????                                                               
	strCmd[80]="FPIC3_B2_VL"; //B2????    
	strCmd[81]="FPIC3_R3_VH"; //R1????                                                               
	strCmd[82]="FPIC3_R3_VL"; //R1????                                                               
	strCmd[83]="FPIC3_G3_VH"; //G1????                                                               
	strCmd[84]="FPIC3_G3_VL"; //G1????                                                               
	strCmd[85]="FPIC3_B3_VH"; //B1????                                                               
	strCmd[86]="FPIC3_B3_VL"; //B1????                                                               
	strCmd[87]="FPIC3_R4_VH"; //R2????                                                               
	strCmd[88]="FPIC3_R4_VL"; //R2????                                                               
	strCmd[89]="FPIC3_G4_VH"; //G2????                                                               
	strCmd[90]="FPIC3_G4_VL"; //G2????                                                               
	strCmd[91]="FPIC3_B4_VH"; //B2????                                                               
	strCmd[92]="FPIC3_B4_VL"; //B2????    

	strCmd[93]="FPIC4_NAMEE";  //picture name user define                                                 
	strCmd[94]="FPIC4_TIMEE";  //????(s)                                                                  
	strCmd[95]="FPIC4_R1_VH"; //R1????                                                                    
	strCmd[96]="FPIC4_R1_VL"; //R1????                                                                    
	strCmd[97]="FPIC4_G1_VH"; //G1????                                                                    
	strCmd[98]="FPIC4_G1_VL"; //G1????                                                                    
	strCmd[99]="FPIC4_B1_VH"; //B1????                                                                    
	strCmd[100]="FPIC4_B1_VL"; //B1????                                                                    
	strCmd[101]="FPIC4_R2_VH"; //R2????                                                                    
	strCmd[102]="FPIC4_R2_VL"; //R2????                                                                    
	strCmd[103]="FPIC4_G2_VH"; //G2????                                                                    
	strCmd[104]="FPIC4_G2_VL"; //G2????                                                                    
	strCmd[105]="FPIC4_B2_VH"; //B2????                                                                    
	strCmd[106]="FPIC4_B2_VL"; //B2????                                                                    
	strCmd[107]="FPIC4_R3_VH"; //R1????                                                                    
	strCmd[108]="FPIC4_R3_VL"; //R1????                                                                    
	strCmd[109]="FPIC4_G3_VH"; //G1????                                                                    
	strCmd[110]="FPIC4_G3_VL"; //G1????                                                                    
	strCmd[111]="FPIC4_B3_VH"; //B1????                                                                    
	strCmd[112]="FPIC4_B3_VL"; //B1????                                                                    
	strCmd[113]="FPIC4_R4_VH"; //R2????                                                                    
	strCmd[114]="FPIC4_R4_VL"; //R2????                                                                    
	strCmd[115]="FPIC4_G4_VH"; //G2????                                                                    
	strCmd[116]="FPIC4_G4_VL"; //G2????                                                                    
	strCmd[117]="FPIC4_B4_VH"; //B2????                                                                    
	strCmd[118]="FPIC4_B4_VL"; //B2????     

	strCmd[119]="FPIC5_NAMEE";  //picture name user define                                                 
	strCmd[120]="FPIC5_TIMEE";  //????(s)                                                                                                                                 
	strCmd[121]="FPIC5_R1_VH"; //R1????                                                                    
	strCmd[122]="FPIC5_R1_VL"; //R1????                                                                    
	strCmd[123]="FPIC5_G1_VH"; //G1????                                                                    
	strCmd[124]="FPIC5_G1_VL"; //G1????                                                                    
	strCmd[125]="FPIC5_B1_VH"; //B1????                                                                    
	strCmd[126]="FPIC5_B1_VL"; //B1????                                                                    
	strCmd[127]="FPIC5_R2_VH"; //R2????                                                                    
	strCmd[128]="FPIC5_R2_VL"; //R2????                                                                    
	strCmd[129]="FPIC5_G2_VH"; //G2????                                                                    
	strCmd[130]="FPIC5_G2_VL"; //G2????                                                                    
	strCmd[131]="FPIC5_B2_VH"; //B2????                                                                    
	strCmd[132]="FPIC5_B2_VL"; //B2????                                                                    
	strCmd[133]="FPIC5_R3_VH"; //R1????                                                                    
	strCmd[134]="FPIC5_R3_VL"; //R1????                                                                    
	strCmd[135]="FPIC5_G3_VH"; //G1????                                                                    
	strCmd[136]="FPIC5_G3_VL"; //G1????                                                                    
	strCmd[137]="FPIC5_B3_VH"; //B1????                                                                    
	strCmd[138]="FPIC5_B3_VL"; //B1????                                                                    
	strCmd[139]="FPIC5_R4_VH"; //R2????                                                                    
	strCmd[140]="FPIC5_R4_VL"; //R2????                                                                    
	strCmd[141]="FPIC5_G4_VH"; //G2????                                                                    
	strCmd[142]="FPIC5_G4_VL"; //G2????                                                                    
	strCmd[143]="FPIC5_B4_VH"; //B2????                                                                    
	strCmd[144]="FPIC5_B4_VL"; //B2????   
	 
	strCmd[145]="FPIC6_NAMEE";  //picture name user define                                                                                                                
	strCmd[146]="FPIC6_TIMEE";  //????(s)                                                                  
	strCmd[147]="FPIC6_R1_VH"; //R1????                                                                    
	strCmd[148]="FPIC6_R1_VL"; //R1????                                                                    
	strCmd[149]="FPIC6_G1_VH"; //G1????                                                                    
	strCmd[150]="FPIC6_G1_VL"; //G1????                                                                    
	strCmd[151]="FPIC6_B1_VH"; //B1????                                                                    
	strCmd[152]="FPIC6_B1_VL"; //B1????                                                                    
	strCmd[153]="FPIC6_R2_VH"; //R2????                                                                    
	strCmd[154]="FPIC6_R2_VL"; //R2????                                                                    
	strCmd[155]="FPIC6_G2_VH"; //G2????                                                                    
	strCmd[156]="FPIC6_G2_VL"; //G2????                                                                    
	strCmd[157]="FPIC6_B2_VH"; //B2????                                                                    
	strCmd[158]="FPIC6_B2_VL"; //B2????                                                                    
	strCmd[159]="FPIC6_R3_VH"; //R1????                                                                    
	strCmd[160]="FPIC6_R3_VL"; //R1????                                                                    
	strCmd[161]="FPIC6_G3_VH"; //G1????                                                                    
	strCmd[162]="FPIC6_G3_VL"; //G1????                                                                    
	strCmd[163]="FPIC6_B3_VH"; //B1????                                                                    
	strCmd[164]="FPIC6_B3_VL"; //B1????                                                                    
	strCmd[165]="FPIC6_R4_VH"; //R2????                                                                    
	strCmd[166]="FPIC6_R4_VL"; //R2????                                                                    
	strCmd[167]="FPIC6_G4_VH"; //G2????                                                                    
	strCmd[168]="FPIC6_G4_VL"; //G2????                                                                    
	strCmd[169]="FPIC6_B4_VH"; //B2????                                                                    
	strCmd[170]="FPIC6_B4_VL"; //B2????    
	 
	strCmd[171]="FPIC7_NAMEE";  //picture name user define                                                  
	strCmd[172]="FPIC7_TIMEE";  //????(s)                                                                                                                                  
	strCmd[173]="FPIC7_R1_VH"; //R1????                                                                    
	strCmd[174]="FPIC7_R1_VL"; //R1????                                                                    
	strCmd[175]="FPIC7_G1_VH"; //G1????                                                                    
	strCmd[176]="FPIC7_G1_VL"; //G1????                                                                    
	strCmd[177]="FPIC7_B1_VH"; //B1????                                                                    
	strCmd[178]="FPIC7_B1_VL"; //B1????                                                                    
	strCmd[179]="FPIC7_R2_VH"; //R2????                                                                    
	strCmd[180]="FPIC7_R2_VL"; //R2????                                                                    
	strCmd[181]="FPIC7_G2_VH"; //G2????                                                                    
	strCmd[182]="FPIC7_G2_VL"; //G2????                                                                    
	strCmd[183]="FPIC7_B2_VH"; //B2????                                                                    
	strCmd[184]="FPIC7_B2_VL"; //B2????                                                                    
	strCmd[185]="FPIC7_R3_VH"; //R1????                                                                    
	strCmd[186]="FPIC7_R3_VL"; //R1????                                                                    
	strCmd[187]="FPIC7_G3_VH"; //G1????                                                                    
	strCmd[188]="FPIC7_G3_VL"; //G1????                                                                    
	strCmd[189]="FPIC7_B3_VH"; //B1????                                                                    
	strCmd[190]="FPIC7_B3_VL"; //B1????                                                                    
	strCmd[191]="FPIC7_R4_VH"; //R2????                                                                    
	strCmd[192]="FPIC7_R4_VL"; //R2????                                                                    
	strCmd[193]="FPIC7_G4_VH"; //G2????                                                                    
	strCmd[194]="FPIC7_G4_VL"; //G2????                                                                    
	strCmd[195]="FPIC7_B4_VH"; //B2????                                                                    
	strCmd[196]="FPIC7_B4_VL"; //B2????       

	strCmd[197]="FPIC8_NAMEE";  //picture name user define                                                                                                              
	strCmd[198]="FPIC8_TIMEE";  //????(s)                                                                  
	strCmd[199]="FPIC8_R1_VH"; //R1????                                                                    
	strCmd[200]="FPIC8_R1_VL"; //R1????                                                                    
	strCmd[201]="FPIC8_G1_VH"; //G1????                                                                    
	strCmd[202]="FPIC8_G1_VL"; //G1????                                                                    
	strCmd[203]="FPIC8_B1_VH"; //B1????                                                                    
	strCmd[204]="FPIC8_B1_VL"; //B1????                                                                    
	strCmd[205]="FPIC8_R2_VH"; //R2????                                                                    
	strCmd[206]="FPIC8_R2_VL"; //R2????                                                                    
	strCmd[207]="FPIC8_G2_VH"; //G2????                                                                    
	strCmd[208]="FPIC8_G2_VL"; //G2????                                                                    
	strCmd[209]="FPIC8_B2_VH"; //B2????                                                                    
	strCmd[210]="FPIC8_B2_VL"; //B2????                                                                    
	strCmd[211]="FPIC8_R3_VH"; //R1????                                                                    
	strCmd[212]="FPIC8_R3_VL"; //R1????                                                                    
	strCmd[213]="FPIC8_G3_VH"; //G1????                                                                    
	strCmd[214]="FPIC8_G3_VL"; //G1????                                                                    
	strCmd[215]="FPIC8_B3_VH"; //B1????                                                                    
	strCmd[216]="FPIC8_B3_VL"; //B1????                                                                    
	strCmd[217]="FPIC8_R4_VH"; //R2????                                                                    
	strCmd[218]="FPIC8_R4_VL"; //R2????                                                                    
	strCmd[219]="FPIC8_G4_VH"; //G2????                                                                    
	strCmd[220]="FPIC8_G4_VL"; //G2????                                                                    
	strCmd[221]="FPIC8_B4_VH"; //B2????                                                                    
	strCmd[222]="FPIC8_B4_VL"; //B2????      

	strCmd[223]="FPIC9_NAMEE";  //picture name user define                                                                                                               
	strCmd[224]="FPIC9_TIMEE";  //????(s)                                                                  
	strCmd[225]="FPIC9_R1_VH"; //R1????                                                                    
	strCmd[226]="FPIC9_R1_VL"; //R1????                                                                    
	strCmd[227]="FPIC9_G1_VH"; //G1????                                                                    
	strCmd[228]="FPIC9_G1_VL"; //G1????                                                                    
	strCmd[229]="FPIC9_B1_VH"; //B1????                                                                    
	strCmd[230]="FPIC9_B1_VL"; //B1????                                                                    
	strCmd[231]="FPIC9_R2_VH"; //R2????                                                                    
	strCmd[232]="FPIC9_R2_VL"; //R2????                                                                    
	strCmd[233]="FPIC9_G2_VH"; //G2????                                                                    
	strCmd[234]="FPIC9_G2_VL"; //G2????                                                                    
	strCmd[235]="FPIC9_B2_VH"; //B2????                                                                    
	strCmd[236]="FPIC9_B2_VL"; //B2????                                                                    
	strCmd[237]="FPIC9_R3_VH"; //R1????                                                                    
	strCmd[238]="FPIC9_R3_VL"; //R1????                                                                    
	strCmd[239]="FPIC9_G3_VH"; //G1????                                                                    
	strCmd[240]="FPIC9_G3_VL"; //G1????                                                                    
	strCmd[241]="FPIC9_B3_VH"; //B1????                                                                    
	strCmd[242]="FPIC9_B3_VL"; //B1????                                                                    
	strCmd[243]="FPIC9_R4_VH"; //R2????                                                                    
	strCmd[244]="FPIC9_R4_VL"; //R2????                                                                    
	strCmd[245]="FPIC9_G4_VH"; //G2????                                                                    
	strCmd[246]="FPIC9_G4_VL"; //G2????                                                                    
	strCmd[247]="FPIC9_B4_VH"; //B2????                                                                    
	strCmd[248]="FPIC9_B4_VL"; //B2????      
		
	strCmd[249]="FPIC10_NAMEE";  //picture name user define                                                                                                             
	strCmd[250]="FPIC10_TIMEE";  //????(s)                                                                  
	strCmd[251]="FPIC10_R1_VH"; //R1????                                                                    
	strCmd[252]="FPIC10_R1_VL"; //R1????                                                                    
	strCmd[253]="FPIC10_G1_VH"; //G1????                                                                    
	strCmd[254]="FPIC10_G1_VL"; //G1????                                                                    
	strCmd[255]="FPIC10_B1_VH"; //B1????                                                                    
	strCmd[256]="FPIC10_B1_VL"; //B1????                                                                    
	strCmd[257]="FPIC10_R2_VH"; //R2????                                                                    
	strCmd[258]="FPIC10_R2_VL"; //R2????                                                                    
	strCmd[259]="FPIC10_G2_VH"; //G2????                                                                    
	strCmd[260]="FPIC10_G2_VL"; //G2????                                                                    
	strCmd[261]="FPIC10_B2_VH"; //B2????                                                                    
	strCmd[262]="FPIC10_B2_VL"; //B2????                                                                    
	strCmd[263]="FPIC10_R3_VH"; //R1????                                                                    
	strCmd[264]="FPIC10_R3_VL"; //R1????                                                                    
	strCmd[265]="FPIC10_G3_VH"; //G1????                                                                    
	strCmd[266]="FPIC10_G3_VL"; //G1????                                                                    
	strCmd[267]="FPIC10_B3_VH"; //B1????                                                                    
	strCmd[268]="FPIC10_B3_VL"; //B1????                                                                    
	strCmd[269]="FPIC10_R4_VH"; //R2????                                                                    
	strCmd[270]="FPIC10_R4_VL"; //R2????                                                                    
	strCmd[271]="FPIC10_G4_VH"; //G2????                                                                    
	strCmd[272]="FPIC10_G4_VL"; //G2????                                                                    
	strCmd[273]="FPIC10_B4_VH"; //B2????                                                                    
	strCmd[274]="FPIC10_B4_VL"; //B2????      

	strCmd[275]="FPIC11_NAMEE";  //picture name user define                                                                                                               
	strCmd[276]="FPIC11_TIMEE";  //????(s)                                                                  
	strCmd[277]="FPIC11_R1_VH"; //R1????                                                                    
	strCmd[278]="FPIC11_R1_VL"; //R1????                                                                    
	strCmd[279]="FPIC11_G1_VH"; //G1????                                                                    
	strCmd[280]="FPIC11_G1_VL"; //G1????                                                                    
	strCmd[281]="FPIC11_B1_VH"; //B1????                                                                    
	strCmd[282]="FPIC11_B1_VL"; //B1????                                                                    
	strCmd[283]="FPIC11_R2_VH"; //R2????                                                                    
	strCmd[284]="FPIC11_R2_VL"; //R2????                                                                    
	strCmd[285]="FPIC11_G2_VH"; //G2????                                                                    
	strCmd[286]="FPIC11_G2_VL"; //G2????                                                                    
	strCmd[287]="FPIC11_B2_VH"; //B2????                                                                    
	strCmd[288]="FPIC11_B2_VL"; //B2????                                                                    
	strCmd[289]="FPIC11_R3_VH"; //R1????                                                                    
	strCmd[290]="FPIC11_R3_VL"; //R1????                                                                    
	strCmd[291]="FPIC11_G3_VH"; //G1????                                                                    
	strCmd[292]="FPIC11_G3_VL"; //G1????                                                                    
	strCmd[293]="FPIC11_B3_VH"; //B1????                                                                    
	strCmd[294]="FPIC11_B3_VL"; //B1????                                                                    
	strCmd[295]="FPIC11_R4_VH"; //R2????                                                                    
	strCmd[296]="FPIC11_R4_VL"; //R2????                                                                    
	strCmd[297]="FPIC11_G4_VH"; //G2????                                                                    
	strCmd[298]="FPIC11_G4_VL"; //G2????                                                                    
	strCmd[299]="FPIC11_B4_VH"; //B2????                                                                    
	strCmd[300]="FPIC11_B4_VL"; //B2????     

	strCmd[301]="FPIC12_NAMEE";  //picture name user define                                                                                                                
	strCmd[302]="FPIC12_TIMEE";  //????(s)                                                                  
	strCmd[303]="FPIC12_R1_VH"; //R1????                                                                    
	strCmd[304]="FPIC12_R1_VL"; //R1????                                                                    
	strCmd[305]="FPIC12_G1_VH"; //G1????                                                                    
	strCmd[306]="FPIC12_G1_VL"; //G1????                                                                    
	strCmd[307]="FPIC12_B1_VH"; //B1????                                                                    
	strCmd[308]="FPIC12_B1_VL"; //B1????                                                                    
	strCmd[309]="FPIC12_R2_VH"; //R2????                                                                    
	strCmd[310]="FPIC12_R2_VL"; //R2????                                                                    
	strCmd[311]="FPIC12_G2_VH"; //G2????                                                                    
	strCmd[312]="FPIC12_G2_VL"; //G2????                                                                    
	strCmd[313]="FPIC12_B2_VH"; //B2????                                                                    
	strCmd[314]="FPIC12_B2_VL"; //B2????                                                                    
	strCmd[315]="FPIC12_R3_VH"; //R1????                                                                    
	strCmd[316]="FPIC12_R3_VL"; //R1????                                                                    
	strCmd[317]="FPIC12_G3_VH"; //G1????                                                                    
	strCmd[318]="FPIC12_G3_VL"; //G1????                                                                    
	strCmd[319]="FPIC12_B3_VH"; //B1????                                                                    
	strCmd[320]="FPIC12_B3_VL"; //B1????                                                                    
	strCmd[321]="FPIC12_R4_VH"; //R2????                                                                    
	strCmd[322]="FPIC12_R4_VL"; //R2????                                                                    
	strCmd[323]="FPIC12_G4_VH"; //G2????                                                                    
	strCmd[324]="FPIC12_G4_VL"; //G2????                                                                    
	strCmd[325]="FPIC12_B4_VH"; //B2????                                                                    
	strCmd[326]="FPIC12_B4_VL"; //B2????     
		
	strCmd[327]="FPIC13_NAMEE";  //picture name user define                                                                                                              
	strCmd[328]="FPIC13_TIMEE";  //????(s)                                                                  
	strCmd[329]="FPIC13_R1_VH"; //R1????                                                                    
	strCmd[330]="FPIC13_R1_VL"; //R1????                                                                    
	strCmd[331]="FPIC13_G1_VH"; //G1????                                                                    
	strCmd[332]="FPIC13_G1_VL"; //G1????                                                                    
	strCmd[333]="FPIC13_B1_VH"; //B1????                                                                    
	strCmd[334]="FPIC13_B1_VL"; //B1????                                                                    
	strCmd[335]="FPIC13_R2_VH"; //R2????                                                                    
	strCmd[336]="FPIC13_R2_VL"; //R2????                                                                    
	strCmd[337]="FPIC13_G2_VH"; //G2????                                                                    
	strCmd[338]="FPIC13_G2_VL"; //G2????                                                                    
	strCmd[339]="FPIC13_B2_VH"; //B2????                                                                    
	strCmd[340]="FPIC13_B2_VL"; //B2????                                                                    
	strCmd[341]="FPIC13_R3_VH"; //R1????                                                                    
	strCmd[342]="FPIC13_R3_VL"; //R1????                                                                    
	strCmd[343]="FPIC13_G3_VH"; //G1????                                                                    
	strCmd[344]="FPIC13_G3_VL"; //G1????                                                                    
	strCmd[345]="FPIC13_B3_VH"; //B1????                                                                    
	strCmd[346]="FPIC13_B3_VL"; //B1????                                                                    
	strCmd[347]="FPIC13_R4_VH"; //R2????                                                                    
	strCmd[348]="FPIC13_R4_VL"; //R2????                                                                    
	strCmd[349]="FPIC13_G4_VH"; //G2????                                                                    
	strCmd[350]="FPIC13_G4_VL"; //G2????                                                                    
	strCmd[351]="FPIC13_B4_VH"; //B2????                                                                    
	strCmd[352]="FPIC13_B4_VL"; //B2????     

	strCmd[353]="FPIC14_NAMEE";  //picture name user define                                                                                                                
	strCmd[354]="FPIC14_TIMEE";  //????(s)                                                                  
	strCmd[355]="FPIC14_R1_VH"; //R1????                                                                    
	strCmd[356]="FPIC14_R1_VL"; //R1????                                                                    
	strCmd[357]="FPIC14_G1_VH"; //G1????                                                                    
	strCmd[358]="FPIC14_G1_VL"; //G1????                                                                    
	strCmd[359]="FPIC14_B1_VH"; //B1????                                                                    
	strCmd[360]="FPIC14_B1_VL"; //B1????                                                                    
	strCmd[361]="FPIC14_R2_VH"; //R2????                                                                    
	strCmd[362]="FPIC14_R2_VL"; //R2????                                                                    
	strCmd[363]="FPIC14_G2_VH"; //G2????                                                                    
	strCmd[364]="FPIC14_G2_VL"; //G2????                                                                    
	strCmd[365]="FPIC14_B2_VH"; //B2????                                                                    
	strCmd[366]="FPIC14_B2_VL"; //B2????                                                                    
	strCmd[367]="FPIC14_R3_VH"; //R1????                                                                    
	strCmd[368]="FPIC14_R3_VL"; //R1????                                                                    
	strCmd[369]="FPIC14_G3_VH"; //G1????                                                                    
	strCmd[370]="FPIC14_G3_VL"; //G1????                                                                    
	strCmd[371]="FPIC14_B3_VH"; //B1????                                                                    
	strCmd[372]="FPIC14_B3_VL"; //B1????                                                                    
	strCmd[373]="FPIC14_R4_VH"; //R2????                                                                    
	strCmd[374]="FPIC14_R4_VL"; //R2????                                                                    
	strCmd[375]="FPIC14_G4_VH"; //G2????                                                                    
	strCmd[376]="FPIC14_G4_VL"; //G2????                                                                    
	strCmd[377]="FPIC14_B4_VH"; //B2????                                                                    
	strCmd[378]="FPIC14_B4_VL"; //B2????      

	strCmd[379]="FPIC15_NAMEE";  //picture name user define                                                                                                               
	strCmd[380]="FPIC15_TIMEE";  //????(s)                                                                  
	strCmd[381]="FPIC15_R1_VH"; //R1????                                                                    
	strCmd[382]="FPIC15_R1_VL"; //R1????                                                                    
	strCmd[383]="FPIC15_G1_VH"; //G1????                                                                    
	strCmd[384]="FPIC15_G1_VL"; //G1????                                                                    
	strCmd[385]="FPIC15_B1_VH"; //B1????                                                                    
	strCmd[386]="FPIC15_B1_VL"; //B1????                                                                    
	strCmd[387]="FPIC15_R2_VH"; //R2????                                                                    
	strCmd[388]="FPIC15_R2_VL"; //R2????                                                                    
	strCmd[389]="FPIC15_G2_VH"; //G2????                                                                    
	strCmd[390]="FPIC15_G2_VL"; //G2????                                                                    
	strCmd[391]="FPIC15_B2_VH"; //B2????                                                                    
	strCmd[392]="FPIC15_B2_VL"; //B2????                                                                    
	strCmd[393]="FPIC15_R3_VH"; //R1????                                                                    
	strCmd[394]="FPIC15_R3_VL"; //R1????                                                                    
	strCmd[395]="FPIC15_G3_VH"; //G1????                                                                    
	strCmd[396]="FPIC15_G3_VL"; //G1????                                                                    
	strCmd[397]="FPIC15_B3_VH"; //B1????                                                                    
	strCmd[398]="FPIC15_B3_VL"; //B1????                                                                    
	strCmd[399]="FPIC15_R4_VH"; //R2????                                                                    
	strCmd[400]="FPIC15_R4_VL"; //R2????                                                                    
	strCmd[401]="FPIC15_G4_VH"; //G2????                                                                    
	strCmd[402]="FPIC15_G4_VL"; //G2????                                                                    
	strCmd[403]="FPIC15_B4_VH"; //B2????                                                                    
	strCmd[404]="FPIC15_B4_VL"; //B2????         
	 
	strCmd[405]="FPIC16_NAMEE";  //picture name user define                                                                                                           
	strCmd[406]="FPIC16_TIMEE";  //????(s)                                                                  
	strCmd[407]="FPIC16_R1_VH"; //R1????                                                                    
	strCmd[408]="FPIC16_R1_VL"; //R1????                                                                    
	strCmd[409]="FPIC16_G1_VH"; //G1????                                                                    
	strCmd[410]="FPIC16_G1_VL"; //G1????                                                                    
	strCmd[411]="FPIC16_B1_VH"; //B1????                                                                    
	strCmd[412]="FPIC16_B1_VL"; //B1????                                                                    
	strCmd[413]="FPIC16_R2_VH"; //R2????                                                                    
	strCmd[414]="FPIC16_R2_VL"; //R2????                                                                    
	strCmd[415]="FPIC16_G2_VH"; //G2????                                                                    
	strCmd[416]="FPIC16_G2_VL"; //G2????                                                                    
	strCmd[417]="FPIC16_B2_VH"; //B2????                                                                    
	strCmd[418]="FPIC16_B2_VL"; //B2????                                                                    
	strCmd[419]="FPIC16_R3_VH"; //R1????                                                                    
	strCmd[420]="FPIC16_R3_VL"; //R1????                                                                    
	strCmd[421]="FPIC16_G3_VH"; //G1????                                                                    
	strCmd[422]="FPIC16_G3_VL"; //G1????                                                                    
	strCmd[423]="FPIC16_B3_VH"; //B1????                                                                    
	strCmd[424]="FPIC16_B3_VL"; //B1????                                                                    
	strCmd[425]="FPIC16_R4_VH"; //R2????                                                                    
	strCmd[426]="FPIC16_R4_VL"; //R2????                                                                    
	strCmd[427]="FPIC16_G4_VH"; //G2????                                                                    
	strCmd[428]="FPIC16_G4_VL"; //G2????                                                                    
	strCmd[429]="FPIC16_B4_VH"; //B2????                                                                    
	strCmd[430]="FPIC16_B4_VL"; //B2????    

	strCmd[431]="FPIC17_NAMEE";  //picture name user define                                                                                                                 
	strCmd[432]="FPIC17_TIMEE";  //????(s)                                                                  
	strCmd[433]="FPIC17_R1_VH"; //R1????                                                                    
	strCmd[434]="FPIC17_R1_VL"; //R1????                                                                    
	strCmd[435]="FPIC17_G1_VH"; //G1????                                                                    
	strCmd[436]="FPIC17_G1_VL"; //G1????                                                                    
	strCmd[437]="FPIC17_B1_VH"; //B1????                                                                    
	strCmd[438]="FPIC17_B1_VL"; //B1????                                                                    
	strCmd[439]="FPIC17_R2_VH"; //R2????                                                                    
	strCmd[440]="FPIC17_R2_VL"; //R2????                                                                    
	strCmd[441]="FPIC17_G2_VH"; //G2????                                                                    
	strCmd[442]="FPIC17_G2_VL"; //G2????                                                                    
	strCmd[443]="FPIC17_B2_VH"; //B2????                                                                    
	strCmd[444]="FPIC17_B2_VL"; //B2????                                                                    
	strCmd[445]="FPIC17_R3_VH"; //R1????                                                                    
	strCmd[446]="FPIC17_R3_VL"; //R1????                                                                    
	strCmd[447]="FPIC17_G3_VH"; //G1????                                                                    
	strCmd[448]="FPIC17_G3_VL"; //G1????                                                                    
	strCmd[449]="FPIC17_B3_VH"; //B1????                                                                    
	strCmd[450]="FPIC17_B3_VL"; //B1????                                                                    
	strCmd[451]="FPIC17_R4_VH"; //R2????                                                                    
	strCmd[452]="FPIC17_R4_VL"; //R2????                                                                    
	strCmd[453]="FPIC17_G4_VH"; //G2????                                                                    
	strCmd[454]="FPIC17_G4_VL"; //G2????                                                                    
	strCmd[455]="FPIC17_B4_VH"; //B2????                                                                    
	strCmd[456]="FPIC17_B4_VL"; //B2????           


	 
	 
	strCmd[457]="S1TV1_TTTTT1";   //(us)                                                                        
	strCmd[458]="S1TV1_TTTTT2";   //(us)                                                                        
	strCmd[459]="S1TV1_TTTTT3";   //(us)                                                                        
	strCmd[460]="S1TV1_TTTTT4";   //(us)                                                                        
	strCmd[461]="S1TV1_TTTTT5";		//(us)                                                                        
	strCmd[462]="S1TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[463]="S1TV1_CK1_T1";	//(us)                                                                      
	strCmd[464]="S1TV1_CK1_T2";	//(us)                                                                      
	strCmd[465]="S1TV1_CK1_T3";	//(us)                                                                      
	strCmd[466]="S1TV1_CK1_T4";	//(us)                                                                      
	strCmd[467]="S1TV1_CK1_T5";	//(us)                                                                      
	strCmd[468]="S1TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[469]="S1TV1_CK2_T1"; //(us)                                                                      
	strCmd[470]="S1TV1_CK2_T2"; //(us)                                                                      
	strCmd[471]="S1TV1_CK2_T3"; //(us)                                                                      
	strCmd[472]="S1TV1_CK2_T4"; //(us)                                                                      
	strCmd[473]="S1TV1_CK2_T5";	//(us)                                                                      
	strCmd[474]="S1TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[475]="S1TV1_RST_T1"; //(us)                                                                      
	strCmd[476]="S1TV1_RST_T2"; //(us)                                                                      
	strCmd[477]="S1TV1_RST_T3"; //(us)                                                                      
	strCmd[478]="S1TV1_RST_T4"; //(us)                                                                      
	strCmd[479]="S1TV1_RST_T5";	//(us)                                                                      
	strCmd[480]="S1TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[481]="S1TV2_TTTTT1";   //(us)                                                                        
	strCmd[482]="S1TV2_TTTTT2";   //(us)                                                                        
	strCmd[483]="S1TV2_TTTTT3";   //(us)                                                                        
	strCmd[484]="S1TV2_TTTTT4";   //(us)                                                                        
	strCmd[485]="S1TV2_TTTTT5";		//(us)                                                                        
	strCmd[486]="S1TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[487]="S1TV2_CK1_T1";	//(us)                                                                      
	strCmd[488]="S1TV2_CK1_T2";	//(us)                                                                      
	strCmd[489]="S1TV2_CK1_T3";	//(us)                                                                      
	strCmd[490]="S1TV2_CK1_T4";	//(us)                                                                      
	strCmd[491]="S1TV2_CK1_T5";	//(us)                                                                      
	strCmd[492]="S1TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[493]="S1TV2_CK2_T1"; //(us)                                                                      
	strCmd[494]="S1TV2_CK2_T2"; //(us)                                                                      
	strCmd[495]="S1TV2_CK2_T3"; //(us)                                                                      
	strCmd[496]="S1TV2_CK2_T4"; //(us)                                                                      
	strCmd[497]="S1TV2_CK2_T5";	//(us)                                                                      
	strCmd[498]="S1TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[499]="S1TV2_RST_T1"; //(us)                                                                      
	strCmd[500]="S1TV2_RST_T2"; //(us)                                                                      
	strCmd[501]="S1TV2_RST_T3"; //(us)                                                                      
	strCmd[502]="S1TV2_RST_T4"; //(us)                                                                      
	strCmd[503]="S1TV2_RST_T5";	//(us)                                                                      
	strCmd[504]="S1TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[505]="S1TV3_TTTTT1";   //(us)                                                                        
	strCmd[506]="S1TV3_TTTTT2";   //(us)                                                                        
	strCmd[507]="S1TV3_TTTTT3";   //(us)                                                                        
	strCmd[508]="S1TV3_TTTTT4";   //(us)                                                                        
	strCmd[509]="S1TV3_TTTTT5";		//(us)                                                                        
	strCmd[510]="S1TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[511]="S1TV3_CK1_T1";	//(us)                                                                      
	strCmd[512]="S1TV3_CK1_T2";	//(us)                                                                      
	strCmd[513]="S1TV3_CK1_T3";	//(us)                                                                      
	strCmd[514]="S1TV3_CK1_T4";	//(us)                                                                      
	strCmd[515]="S1TV3_CK1_T5";	//(us)                                                                      
	strCmd[516]="S1TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[517]="S1TV3_CK2_T1"; //(us)                                                                      
	strCmd[518]="S1TV3_CK2_T2"; //(us)                                                                      
	strCmd[519]="S1TV3_CK2_T3"; //(us)                                                                      
	strCmd[520]="S1TV3_CK2_T4"; //(us)                                                                      
	strCmd[521]="S1TV3_CK2_T5";	//(us)                                                                      
	strCmd[522]="S1TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[523]="S1TV3_RST_T1"; //(us)                                                                      
	strCmd[524]="S1TV3_RST_T2"; //(us)                                                                      
	strCmd[525]="S1TV3_RST_T3"; //(us)                                                                      
	strCmd[526]="S1TV3_RST_T4"; //(us)                                                                      
	strCmd[527]="S1TV3_RST_T5";	//(us)                                                                      
	strCmd[528]="S1TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[529]="S1TV4_TTTTT1";   //(us)                                                                        
	strCmd[530]="S1TV4_TTTTT2";   //(us)                                                                        
	strCmd[531]="S1TV4_TTTTT3";   //(us)                                                                        
	strCmd[532]="S1TV4_TTTTT4";   //(us)                                                                        
	strCmd[533]="S1TV4_TTTTT5";		//(us)                                                                        
	strCmd[534]="S1TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[535]="S1TV4_CK1_T1";	//(us)                                                                      
	strCmd[536]="S1TV4_CK1_T2";	//(us)                                                                      
	strCmd[537]="S1TV4_CK1_T3";	//(us)                                                                      
	strCmd[538]="S1TV4_CK1_T4";	//(us)                                                                      
	strCmd[539]="S1TV4_CK1_T5";	//(us)                                                                      
	strCmd[540]="S1TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[541]="S1TV4_CK2_T1"; //(us)                                                                      
	strCmd[542]="S1TV4_CK2_T2"; //(us)                                                                     
	strCmd[543]="S1TV4_CK2_T3"; //(us)                                                                     
	strCmd[544]="S1TV4_CK2_T4"; //(us)                                                                     
	strCmd[545]="S1TV4_CK2_T5";	//(us)                                                                    
	strCmd[546]="S1TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[547]="S1TV4_RST_T1"; //(us)                                                                     
	strCmd[548]="S1TV4_RST_T2"; //(us)                                                                     
	strCmd[549]="S1TV4_RST_T3"; //(us)                                                                     
	strCmd[550]="S1TV4_RST_T4"; //(us)                                                                     
	strCmd[551]="S1TV4_RST_T5";	//(us)                                                                    
	strCmd[552]="S1TV4_RST_NN";      	//      

	strCmd[553]="S2TV1_TTTTT1";   //(us)                                                                        
	strCmd[554]="S2TV1_TTTTT2";   //(us)                                                                        
	strCmd[555]="S2TV1_TTTTT3";   //(us)                                                                        
	strCmd[556]="S2TV1_TTTTT4";   //(us)                                                                        
	strCmd[557]="S2TV1_TTTTT5";		//(us)                                                                        
	strCmd[558]="S2TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[559]="S2TV1_CK1_T1";	//(us)                                                                      
	strCmd[560]="S2TV1_CK1_T2";	//(us)                                                                      
	strCmd[561]="S2TV1_CK1_T3";	//(us)                                                                      
	strCmd[562]="S2TV1_CK1_T4";	//(us)                                                                      
	strCmd[563]="S2TV1_CK1_T5";	//(us)                                                                      
	strCmd[564]="S2TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[565]="S2TV1_CK2_T1"; //(us)                                                                      
	strCmd[566]="S2TV1_CK2_T2"; //(us)                                                                      
	strCmd[567]="S2TV1_CK2_T3"; //(us)                                                                      
	strCmd[568]="S2TV1_CK2_T4"; //(us)                                                                      
	strCmd[569]="S2TV1_CK2_T5";	//(us)                                                                      
	strCmd[570]="S2TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[571]="S2TV1_RST_T1"; //(us)                                                                      
	strCmd[572]="S2TV1_RST_T2"; //(us)                                                                      
	strCmd[573]="S2TV1_RST_T3"; //(us)                                                                      
	strCmd[574]="S2TV1_RST_T4"; //(us)                                                                      
	strCmd[575]="S2TV1_RST_T5";	//(us)                                                                      
	strCmd[576]="S2TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[577]="S2TV2_TTTTT1";   //(us)                                                                        
	strCmd[578]="S2TV2_TTTTT2";   //(us)                                                                        
	strCmd[579]="S2TV2_TTTTT3";   //(us)                                                                        
	strCmd[580]="S2TV2_TTTTT4";   //(us)                                                                        
	strCmd[581]="S2TV2_TTTTT5";		//(us)                                                                        
	strCmd[582]="S2TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[583]="S2TV2_CK1_T1";	//(us)                                                                      
	strCmd[584]="S2TV2_CK1_T2";	//(us)                                                                      
	strCmd[585]="S2TV2_CK1_T3";	//(us)                                                                      
	strCmd[586]="S2TV2_CK1_T4";	//(us)                                                                      
	strCmd[587]="S2TV2_CK1_T5";	//(us)                                                                      
	strCmd[588]="S2TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[589]="S2TV2_CK2_T1"; //(us)                                                                      
	strCmd[590]="S2TV2_CK2_T2"; //(us)                                                                      
	strCmd[591]="S2TV2_CK2_T3"; //(us)                                                                      
	strCmd[592]="S2TV2_CK2_T4"; //(us)                                                                      
	strCmd[593]="S2TV2_CK2_T5";	//(us)                                                                      
	strCmd[594]="S2TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[595]="S2TV2_RST_T1"; //(us)                                                                      
	strCmd[596]="S2TV2_RST_T2"; //(us)                                                                      
	strCmd[597]="S2TV2_RST_T3"; //(us)                                                                      
	strCmd[598]="S2TV2_RST_T4"; //(us)                                                                      
	strCmd[599]="S2TV2_RST_T5";	//(us)                                                                      
	strCmd[600]="S2TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[601]="S2TV3_TTTTT1";   //(us)                                                                        
	strCmd[602]="S2TV3_TTTTT2";   //(us)                                                                        
	strCmd[603]="S2TV3_TTTTT3";   //(us)                                                                        
	strCmd[604]="S2TV3_TTTTT4";   //(us)                                                                        
	strCmd[605]="S2TV3_TTTTT5";		//(us)                                                                        
	strCmd[606]="S2TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[607]="S2TV3_CK1_T1";	//(us)                                                                      
	strCmd[608]="S2TV3_CK1_T2";	//(us)                                                                      
	strCmd[609]="S2TV3_CK1_T3";	//(us)                                                                      
	strCmd[610]="S2TV3_CK1_T4";	//(us)                                                                      
	strCmd[611]="S2TV3_CK1_T5";	//(us)                                                                      
	strCmd[612]="S2TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[613]="S2TV3_CK2_T1"; //(us)                                                                      
	strCmd[614]="S2TV3_CK2_T2"; //(us)                                                                      
	strCmd[615]="S2TV3_CK2_T3"; //(us)                                                                      
	strCmd[616]="S2TV3_CK2_T4"; //(us)                                                                      
	strCmd[617]="S2TV3_CK2_T5";	//(us)                                                                      
	strCmd[618]="S2TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[619]="S2TV3_RST_T1"; //(us)                                                                      
	strCmd[620]="S2TV3_RST_T2"; //(us)                                                                      
	strCmd[621]="S2TV3_RST_T3"; //(us)                                                                      
	strCmd[622]="S2TV3_RST_T4"; //(us)                                                                      
	strCmd[623]="S2TV3_RST_T5";	//(us)                                                                      
	strCmd[624]="S2TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[625]="S2TV4_TTTTT1";   //(us)                                                                        
	strCmd[626]="S2TV4_TTTTT2";   //(us)                                                                        
	strCmd[627]="S2TV4_TTTTT3";   //(us)                                                                        
	strCmd[628]="S2TV4_TTTTT4";   //(us)                                                                        
	strCmd[629]="S2TV4_TTTTT5";		//(us)                                                                        
	strCmd[630]="S2TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[631]="S2TV4_CK1_T1";	//(us)                                                                      
	strCmd[632]="S2TV4_CK1_T2";	//(us)                                                                      
	strCmd[633]="S2TV4_CK1_T3";	//(us)                                                                      
	strCmd[634]="S2TV4_CK1_T4";	//(us)                                                                      
	strCmd[635]="S2TV4_CK1_T5";	//(us)                                                                      
	strCmd[636]="S2TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[637]="S2TV4_CK2_T1"; //(us)                                                                      
	strCmd[638]="S2TV4_CK2_T2"; //(us)                                                                     
	strCmd[639]="S2TV4_CK2_T3"; //(us)                                                                     
	strCmd[640]="S2TV4_CK2_T4"; //(us)                                                                     
	strCmd[641]="S2TV4_CK2_T5";	//(us)                                                                    
	strCmd[642]="S2TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[643]="S2TV4_RST_T1"; //(us)                                                                     
	strCmd[644]="S2TV4_RST_T2"; //(us)                                                                     
	strCmd[645]="S2TV4_RST_T3"; //(us)                                                                     
	strCmd[646]="S2TV4_RST_T4"; //(us)                                                                     
	strCmd[647]="S2TV4_RST_T5";	//(us)                                                                    
	strCmd[648]="S2TV4_RST_NN";       

	strCmd[649]="S3TV1_TTTTT1";   //(us)                                                                        
	strCmd[650]="S3TV1_TTTTT2";   //(us)                                                                        
	strCmd[651]="S3TV1_TTTTT3";   //(us)                                                                        
	strCmd[652]="S3TV1_TTTTT4";   //(us)                                                                        
	strCmd[653]="S3TV1_TTTTT5";		//(us)                                                                        
	strCmd[654]="S3TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[655]="S3TV1_CK1_T1";	//(us)                                                                      
	strCmd[656]="S3TV1_CK1_T2";	//(us)                                                                      
	strCmd[657]="S3TV1_CK1_T3";	//(us)                                                                      
	strCmd[658]="S3TV1_CK1_T4";	//(us)                                                                      
	strCmd[659]="S3TV1_CK1_T5";	//(us)                                                                      
	strCmd[660]="S3TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[661]="S3TV1_CK2_T1"; //(us)                                                                      
	strCmd[662]="S3TV1_CK2_T2"; //(us)                                                                      
	strCmd[663]="S3TV1_CK2_T3"; //(us)                                                                      
	strCmd[664]="S3TV1_CK2_T4"; //(us)                                                                      
	strCmd[665]="S3TV1_CK2_T5";	//(us)                                                                      
	strCmd[666]="S3TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[667]="S3TV1_RST_T1"; //(us)                                                                      
	strCmd[668]="S3TV1_RST_T2"; //(us)                                                                      
	strCmd[669]="S3TV1_RST_T3"; //(us)                                                                      
	strCmd[670]="S3TV1_RST_T4"; //(us)                                                                      
	strCmd[671]="S3TV1_RST_T5";	//(us)                                                                      
	strCmd[672]="S3TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[673]="S3TV2_TTTTT1";   //(us)                                                                        
	strCmd[674]="S3TV2_TTTTT2";   //(us)                                                                        
	strCmd[675]="S3TV2_TTTTT3";   //(us)                                                                        
	strCmd[676]="S3TV2_TTTTT4";   //(us)                                                                        
	strCmd[677]="S3TV2_TTTTT5";		//(us)                                                                        
	strCmd[678]="S3TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[679]="S3TV2_CK1_T1";	//(us)                                                                      
	strCmd[680]="S3TV2_CK1_T2";	//(us)                                                                      
	strCmd[681]="S3TV2_CK1_T3";	//(us)                                                                      
	strCmd[682]="S3TV2_CK1_T4";	//(us)                                                                      
	strCmd[683]="S3TV2_CK1_T5";	//(us)                                                                      
	strCmd[684]="S3TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[685]="S3TV2_CK2_T1"; //(us)                                                                      
	strCmd[686]="S3TV2_CK2_T2"; //(us)                                                                      
	strCmd[687]="S3TV2_CK2_T3"; //(us)                                                                      
	strCmd[688]="S3TV2_CK2_T4"; //(us)                                                                      
	strCmd[689]="S3TV2_CK2_T5";	//(us)                                                                      
	strCmd[690]="S3TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[691]="S3TV2_RST_T1"; //(us)                                                                      
	strCmd[692]="S3TV2_RST_T2"; //(us)                                                                      
	strCmd[693]="S3TV2_RST_T3"; //(us)                                                                      
	strCmd[694]="S3TV2_RST_T4"; //(us)                                                                      
	strCmd[695]="S3TV2_RST_T5";	//(us)                                                                      
	strCmd[696]="S3TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[697]="S3TV3_TTTTT1";   //(us)                                                                        
	strCmd[698]="S3TV3_TTTTT2";   //(us)                                                                        
	strCmd[699]="S3TV3_TTTTT3";   //(us)                                                                        
	strCmd[700]="S3TV3_TTTTT4";   //(us)                                                                        
	strCmd[701]="S3TV3_TTTTT5";		//(us)                                                                        
	strCmd[702]="S3TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[703]="S3TV3_CK1_T1";	//(us)                                                                      
	strCmd[704]="S3TV3_CK1_T2";	//(us)                                                                      
	strCmd[705]="S3TV3_CK1_T3";	//(us)                                                                      
	strCmd[706]="S3TV3_CK1_T4";	//(us)                                                                      
	strCmd[707]="S3TV3_CK1_T5";	//(us)                                                                      
	strCmd[708]="S3TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[709]="S3TV3_CK2_T1"; //(us)                                                                      
	strCmd[710]="S3TV3_CK2_T2"; //(us)                                                                      
	strCmd[711]="S3TV3_CK2_T3"; //(us)                                                                      
	strCmd[712]="S3TV3_CK2_T4"; //(us)                                                                      
	strCmd[713]="S3TV3_CK2_T5";	//(us)                                                                      
	strCmd[714]="S3TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[715]="S3TV3_RST_T1"; //(us)                                                                      
	strCmd[716]="S3TV3_RST_T2"; //(us)                                                                      
	strCmd[717]="S3TV3_RST_T3"; //(us)                                                                      
	strCmd[718]="S3TV3_RST_T4"; //(us)                                                                      
	strCmd[719]="S3TV3_RST_T5";	//(us)                                                                      
	strCmd[720]="S3TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[721]="S3TV4_TTTTT1";   //(us)                                                                        
	strCmd[722]="S3TV4_TTTTT2";   //(us)                                                                        
	strCmd[723]="S3TV4_TTTTT3";   //(us)                                                                        
	strCmd[724]="S3TV4_TTTTT4";   //(us)                                                                        
	strCmd[725]="S3TV4_TTTTT5";		//(us)                                                                        
	strCmd[726]="S3TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[727]="S3TV4_CK1_T1";	//(us)                                                                      
	strCmd[728]="S3TV4_CK1_T2";	//(us)                                                                      
	strCmd[729]="S3TV4_CK1_T3";	//(us)                                                                      
	strCmd[730]="S3TV4_CK1_T4";	//(us)                                                                      
	strCmd[731]="S3TV4_CK1_T5";	//(us)                                                                      
	strCmd[732]="S3TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[733]="S3TV4_CK2_T1"; //(us)                                                                      
	strCmd[734]="S3TV4_CK2_T2"; //(us)                                                                     
	strCmd[735]="S3TV4_CK2_T3"; //(us)                                                                     
	strCmd[736]="S3TV4_CK2_T4"; //(us)                                                                     
	strCmd[737]="S3TV4_CK2_T5";	//(us)                                                                    
	strCmd[738]="S3TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[739]="S3TV4_RST_T1"; //(us)                                                                     
	strCmd[740]="S3TV4_RST_T2"; //(us)                                                                     
	strCmd[741]="S3TV4_RST_T3"; //(us)                                                                     
	strCmd[742]="S3TV4_RST_T4"; //(us)                                                                     
	strCmd[743]="S3TV4_RST_T5";	//(us)                                                                    
	strCmd[744]="S3TV4_RST_NN";      

	strCmd[745]="S4TV1_TTTTT1";   //(us)                                                                        
	strCmd[746]="S4TV1_TTTTT2";   //(us)                                                                        
	strCmd[747]="S4TV1_TTTTT3";   //(us)                                                                        
	strCmd[748]="S4TV1_TTTTT4";   //(us)                                                                        
	strCmd[749]="S4TV1_TTTTT5";		//(us)                                                                        
	strCmd[750]="S4TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[751]="S4TV1_CK1_T1";	//(us)                                                                      
	strCmd[752]="S4TV1_CK1_T2";	//(us)                                                                      
	strCmd[753]="S4TV1_CK1_T3";	//(us)                                                                      
	strCmd[754]="S4TV1_CK1_T4";	//(us)                                                                      
	strCmd[755]="S4TV1_CK1_T5";	//(us)                                                                      
	strCmd[756]="S4TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[757]="S4TV1_CK2_T1"; //(us)                                                                      
	strCmd[758]="S4TV1_CK2_T2"; //(us)                                                                      
	strCmd[759]="S4TV1_CK2_T3"; //(us)                                                                      
	strCmd[760]="S4TV1_CK2_T4"; //(us)                                                                      
	strCmd[761]="S4TV1_CK2_T5";	//(us)                                                                      
	strCmd[762]="S4TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[763]="S4TV1_RST_T1"; //(us)                                                                      
	strCmd[764]="S4TV1_RST_T2"; //(us)                                                                      
	strCmd[765]="S4TV1_RST_T3"; //(us)                                                                      
	strCmd[766]="S4TV1_RST_T4"; //(us)                                                                      
	strCmd[767]="S4TV1_RST_T5";	//(us)                                                                      
	strCmd[768]="S4TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[769]="S4TV2_TTTTT1";   //(us)                                                                        
	strCmd[770]="S4TV2_TTTTT2";   //(us)                                                                        
	strCmd[771]="S4TV2_TTTTT3";   //(us)                                                                        
	strCmd[772]="S4TV2_TTTTT4";   //(us)                                                                        
	strCmd[773]="S4TV2_TTTTT5";		//(us)                                                                        
	strCmd[774]="S4TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[775]="S4TV2_CK1_T1";	//(us)                                                                      
	strCmd[776]="S4TV2_CK1_T2";	//(us)                                                                      
	strCmd[777]="S4TV2_CK1_T3";	//(us)                                                                      
	strCmd[778]="S4TV2_CK1_T4";	//(us)                                                                      
	strCmd[779]="S4TV2_CK1_T5";	//(us)                                                                      
	strCmd[780]="S4TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[781]="S4TV2_CK2_T1"; //(us)                                                                      
	strCmd[782]="S4TV2_CK2_T2"; //(us)                                                                      
	strCmd[783]="S4TV2_CK2_T3"; //(us)                                                                      
	strCmd[784]="S4TV2_CK2_T4"; //(us)                                                                      
	strCmd[785]="S4TV2_CK2_T5";	//(us)                                                                      
	strCmd[786]="S4TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[787]="S4TV2_RST_T1"; //(us)                                                                      
	strCmd[788]="S4TV2_RST_T2"; //(us)                                                                      
	strCmd[789]="S4TV2_RST_T3"; //(us)                                                                      
	strCmd[790]="S4TV2_RST_T4"; //(us)                                                                      
	strCmd[791]="S4TV2_RST_T5";	//(us)                                                                      
	strCmd[792]="S4TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[793]="S4TV3_TTTTT1";   //(us)                                                                        
	strCmd[794]="S4TV3_TTTTT2";   //(us)                                                                        
	strCmd[795]="S4TV3_TTTTT3";   //(us)                                                                        
	strCmd[796]="S4TV3_TTTTT4";   //(us)                                                                        
	strCmd[797]="S4TV3_TTTTT5";		//(us)                                                                        
	strCmd[798]="S4TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[799]="S4TV3_CK1_T1";	//(us)                                                                      
	strCmd[800]="S4TV3_CK1_T2";	//(us)                                                                      
	strCmd[801]="S4TV3_CK1_T3";	//(us)                                                                      
	strCmd[802]="S4TV3_CK1_T4";	//(us)                                                                      
	strCmd[803]="S4TV3_CK1_T5";	//(us)                                                                      
	strCmd[804]="S4TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[805]="S4TV3_CK2_T1"; //(us)                                                                      
	strCmd[806]="S4TV3_CK2_T2"; //(us)                                                                      
	strCmd[807]="S4TV3_CK2_T3"; //(us)                                                                      
	strCmd[808]="S4TV3_CK2_T4"; //(us)                                                                      
	strCmd[809]="S4TV3_CK2_T5";	//(us)                                                                      
	strCmd[810]="S4TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[811]="S4TV3_RST_T1"; //(us)                                                                      
	strCmd[812]="S4TV3_RST_T2"; //(us)                                                                      
	strCmd[813]="S4TV3_RST_T3"; //(us)                                                                      
	strCmd[814]="S4TV3_RST_T4"; //(us)                                                                      
	strCmd[815]="S4TV3_RST_T5";	//(us)                                                                      
	strCmd[816]="S4TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[817]="S4TV4_TTTTT1";   //(us)                                                                        
	strCmd[818]="S4TV4_TTTTT2";   //(us)                                                                        
	strCmd[819]="S4TV4_TTTTT3";   //(us)                                                                        
	strCmd[820]="S4TV4_TTTTT4";   //(us)                                                                        
	strCmd[821]="S4TV4_TTTTT5";		//(us)                                                                        
	strCmd[822]="S4TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[823]="S4TV4_CK1_T1";	//(us)                                                                      
	strCmd[824]="S4TV4_CK1_T2";	//(us)                                                                      
	strCmd[825]="S4TV4_CK1_T3";	//(us)                                                                      
	strCmd[826]="S4TV4_CK1_T4";	//(us)                                                                      
	strCmd[827]="S4TV4_CK1_T5";	//(us)                                                                      
	strCmd[828]="S4TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[829]="S4TV4_CK2_T1"; //(us)                                                                      
	strCmd[830]="S4TV4_CK2_T2"; //(us)                                                                     
	strCmd[831]="S4TV4_CK2_T3"; //(us)                                                                     
	strCmd[832]="S4TV4_CK2_T4"; //(us)                                                                     
	strCmd[833]="S4TV4_CK2_T5";	//(us)                                                                    
	strCmd[834]="S4TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[835]="S4TV4_RST_T1"; //(us)                                                                     
	strCmd[836]="S4TV4_RST_T2"; //(us)                                                                     
	strCmd[837]="S4TV4_RST_T3"; //(us)                                                                     
	strCmd[838]="S4TV4_RST_T4"; //(us)                                                                     
	strCmd[839]="S4TV4_RST_T5";	//(us)                                                                    
	strCmd[840]="S4TV4_RST_NN";

	strCmd[841]="S5TV1_TTTTT1";   //(us)                                                                        
	strCmd[842]="S5TV1_TTTTT2";   //(us)                                                                        
	strCmd[843]="S5TV1_TTTTT3";   //(us)                                                                        
	strCmd[844]="S5TV1_TTTTT4";   //(us)                                                                        
	strCmd[845]="S5TV1_TTTTT5";		//(us)                                                                        
	strCmd[846]="S5TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[847]="S5TV1_CK1_T1";	//(us)                                                                      
	strCmd[848]="S5TV1_CK1_T2";	//(us)                                                                      
	strCmd[849]="S5TV1_CK1_T3";	//(us)                                                                      
	strCmd[850]="S5TV1_CK1_T4";	//(us)                                                                      
	strCmd[851]="S5TV1_CK1_T5";	//(us)                                                                      
	strCmd[852]="S5TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[853]="S5TV1_CK2_T1"; //(us)                                                                      
	strCmd[854]="S5TV1_CK2_T2"; //(us)                                                                      
	strCmd[855]="S5TV1_CK2_T3"; //(us)                                                                      
	strCmd[856]="S5TV1_CK2_T4"; //(us)                                                                      
	strCmd[857]="S5TV1_CK2_T5";	//(us)                                                                      
	strCmd[858]="S5TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[859]="S5TV1_RST_T1"; //(us)                                                                      
	strCmd[860]="S5TV1_RST_T2"; //(us)                                                                      
	strCmd[861]="S5TV1_RST_T3"; //(us)                                                                      
	strCmd[862]="S5TV1_RST_T4"; //(us)                                                                      
	strCmd[863]="S5TV1_RST_T5";	//(us)                                                                      
	strCmd[864]="S5TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[865]="S5TV2_TTTTT1";   //(us)                                                                        
	strCmd[866]="S5TV2_TTTTT2";   //(us)                                                                        
	strCmd[867]="S5TV2_TTTTT3";   //(us)                                                                        
	strCmd[868]="S5TV2_TTTTT4";   //(us)                                                                        
	strCmd[869]="S5TV2_TTTTT5";		//(us)                                                                        
	strCmd[870]="S5TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[871]="S5TV2_CK1_T1";	//(us)                                                                      
	strCmd[872]="S5TV2_CK1_T2";	//(us)                                                                      
	strCmd[873]="S5TV2_CK1_T3";	//(us)                                                                      
	strCmd[874]="S5TV2_CK1_T4";	//(us)                                                                      
	strCmd[875]="S5TV2_CK1_T5";	//(us)                                                                      
	strCmd[876]="S5TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[877]="S5TV2_CK2_T1"; //(us)                                                                      
	strCmd[878]="S5TV2_CK2_T2"; //(us)                                                                      
	strCmd[879]="S5TV2_CK2_T3"; //(us)                                                                      
	strCmd[880]="S5TV2_CK2_T4"; //(us)                                                                      
	strCmd[881]="S5TV2_CK2_T5";	//(us)                                                                      
	strCmd[882]="S5TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[883]="S5TV2_RST_T1"; //(us)                                                                      
	strCmd[884]="S5TV2_RST_T2"; //(us)                                                                      
	strCmd[885]="S5TV2_RST_T3"; //(us)                                                                      
	strCmd[886]="S5TV2_RST_T4"; //(us)                                                                      
	strCmd[887]="S5TV2_RST_T5";	//(us)                                                                      
	strCmd[888]="S5TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[889]="S5TV3_TTTTT1";   //(us)                                                                        
	strCmd[890]="S5TV3_TTTTT2";   //(us)                                                                        
	strCmd[891]="S5TV3_TTTTT3";   //(us)                                                                        
	strCmd[892]="S5TV3_TTTTT4";   //(us)                                                                        
	strCmd[893]="S5TV3_TTTTT5";		//(us)                                                                        
	strCmd[894]="S5TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[895]="S5TV3_CK1_T1";	//(us)                                                                      
	strCmd[896]="S5TV3_CK1_T2";	//(us)                                                                      
	strCmd[897]="S5TV3_CK1_T3";	//(us)                                                                      
	strCmd[898]="S5TV3_CK1_T4";	//(us)                                                                      
	strCmd[899]="S5TV3_CK1_T5";	//(us)                                                                      
	strCmd[900]="S5TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[901]="S5TV3_CK2_T1"; //(us)                                                                      
	strCmd[902]="S5TV3_CK2_T2"; //(us)                                                                      
	strCmd[903]="S5TV3_CK2_T3"; //(us)                                                                      
	strCmd[904]="S5TV3_CK2_T4"; //(us)                                                                      
	strCmd[905]="S5TV3_CK2_T5";	//(us)                                                                      
	strCmd[906]="S5TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[907]="S5TV3_RST_T1"; //(us)                                                                      
	strCmd[908]="S5TV3_RST_T2"; //(us)                                                                      
	strCmd[909]="S5TV3_RST_T3"; //(us)                                                                      
	strCmd[910]="S5TV3_RST_T4"; //(us)                                                                      
	strCmd[911]="S5TV3_RST_T5";	//(us)                                                                      
	strCmd[912]="S5TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[913]="S5TV4_TTTTT1";   //(us)                                                                        
	strCmd[914]="S5TV4_TTTTT2";   //(us)                                                                        
	strCmd[915]="S5TV4_TTTTT3";   //(us)                                                                        
	strCmd[916]="S5TV4_TTTTT4";   //(us)                                                                        
	strCmd[917]="S5TV4_TTTTT5";		//(us)                                                                        
	strCmd[918]="S5TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[919]="S5TV4_CK1_T1";	//(us)                                                                      
	strCmd[920]="S5TV4_CK1_T2";	//(us)                                                                      
	strCmd[921]="S5TV4_CK1_T3";	//(us)                                                                      
	strCmd[922]="S5TV4_CK1_T4";	//(us)                                                                      
	strCmd[923]="S5TV4_CK1_T5";	//(us)                                                                      
	strCmd[924]="S5TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[925]="S5TV4_CK2_T1"; //(us)                                                                      
	strCmd[926]="S5TV4_CK2_T2"; //(us)                                                                     
	strCmd[927]="S5TV4_CK2_T3"; //(us)                                                                     
	strCmd[928]="S5TV4_CK2_T4"; //(us)                                                                     
	strCmd[929]="S5TV4_CK2_T5";	//(us)                                                                    
	strCmd[930]="S5TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[931]="S5TV4_RST_T1"; //(us)                                                                     
	strCmd[932]="S5TV4_RST_T2"; //(us)                                                                     
	strCmd[933]="S5TV4_RST_T3"; //(us)                                                                     
	strCmd[934]="S5TV4_RST_T4"; //(us)                                                                     
	strCmd[935]="S5TV4_RST_T5";	//(us)                                                                    
	strCmd[936]="S5TV4_RST_NN";

	strCmd[937]="S6TV1_TTTTT1";   //(us)                                                                        
	strCmd[938]="S6TV1_TTTTT2";   //(us)                                                                        
	strCmd[939]="S6TV1_TTTTT3";   //(us)                                                                        
	strCmd[940]="S6TV1_TTTTT4";   //(us)                                                                        
	strCmd[941]="S6TV1_TTTTT5";		//(us)                                                                        
	strCmd[942]="S6TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[943]="S6TV1_CK1_T1";	//(us)                                                                      
	strCmd[944]="S6TV1_CK1_T2";	//(us)                                                                      
	strCmd[945]="S6TV1_CK1_T3";	//(us)                                                                      
	strCmd[946]="S6TV1_CK1_T4";	//(us)                                                                      
	strCmd[947]="S6TV1_CK1_T5";	//(us)                                                                      
	strCmd[948]="S6TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[949]="S6TV1_CK2_T1"; //(us)                                                                      
	strCmd[950]="S6TV1_CK2_T2"; //(us)                                                                      
	strCmd[951]="S6TV1_CK2_T3"; //(us)                                                                      
	strCmd[952]="S6TV1_CK2_T4"; //(us)                                                                      
	strCmd[953]="S6TV1_CK2_T5";	//(us)                                                                      
	strCmd[954]="S6TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[955]="S6TV1_RST_T1"; //(us)                                                                      
	strCmd[956]="S6TV1_RST_T2"; //(us)                                                                      
	strCmd[957]="S6TV1_RST_T3"; //(us)                                                                      
	strCmd[958]="S6TV1_RST_T4"; //(us)                                                                      
	strCmd[959]="S6TV1_RST_T5";	//(us)                                                                      
	strCmd[960]="S6TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[961]="S6TV2_TTTTT1";   //(us)                                                                        
	strCmd[962]="S6TV2_TTTTT2";   //(us)                                                                        
	strCmd[963]="S6TV2_TTTTT3";   //(us)                                                                        
	strCmd[964]="S6TV2_TTTTT4";   //(us)                                                                        
	strCmd[965]="S6TV2_TTTTT5";		//(us)                                                                        
	strCmd[966]="S6TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[967]="S6TV2_CK1_T1";	//(us)                                                                      
	strCmd[968]="S6TV2_CK1_T2";	//(us)                                                                      
	strCmd[969]="S6TV2_CK1_T3";	//(us)                                                                      
	strCmd[970]="S6TV2_CK1_T4";	//(us)                                                                      
	strCmd[971]="S6TV2_CK1_T5";	//(us)                                                                      
	strCmd[972]="S6TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[973]="S6TV2_CK2_T1"; //(us)                                                                      
	strCmd[974]="S6TV2_CK2_T2"; //(us)                                                                      
	strCmd[975]="S6TV2_CK2_T3"; //(us)                                                                      
	strCmd[976]="S6TV2_CK2_T4"; //(us)                                                                      
	strCmd[977]="S6TV2_CK2_T5";	//(us)                                                                      
	strCmd[978]="S6TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[979]="S6TV2_RST_T1"; //(us)                                                                      
	strCmd[980]="S6TV2_RST_T2"; //(us)                                                                      
	strCmd[981]="S6TV2_RST_T3"; //(us)                                                                      
	strCmd[982]="S6TV2_RST_T4"; //(us)                                                                      
	strCmd[983]="S6TV2_RST_T5";	//(us)                                                                      
	strCmd[984]="S6TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[985]="S6TV3_TTTTT1";   //(us)                                                                        
	strCmd[986]="S6TV3_TTTTT2";   //(us)                                                                        
	strCmd[987]="S6TV3_TTTTT3";   //(us)                                                                        
	strCmd[988]="S6TV3_TTTTT4";   //(us)                                                                        
	strCmd[989]="S6TV3_TTTTT5";		//(us)                                                                        
	strCmd[990]="S6TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[991]="S6TV3_CK1_T1";	//(us)                                                                      
	strCmd[992]="S6TV3_CK1_T2";	//(us)                                                                      
	strCmd[993]="S6TV3_CK1_T3";	//(us)                                                                      
	strCmd[994]="S6TV3_CK1_T4";	//(us)                                                                      
	strCmd[995]="S6TV3_CK1_T5";	//(us)                                                                      
	strCmd[996]="S6TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[997]="S6TV3_CK2_T1"; //(us)                                                                      
	strCmd[998]="S6TV3_CK2_T2"; //(us)                                                                      
	strCmd[999]="S6TV3_CK2_T3"; //(us)                                                                      
	strCmd[1000]="S6TV3_CK2_T4"; //(us)                                                                      
	strCmd[1001]="S6TV3_CK2_T5";	//(us)                                                                      
	strCmd[1002]="S6TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1003]="S6TV3_RST_T1"; //(us)                                                                      
	strCmd[1004]="S6TV3_RST_T2"; //(us)                                                                      
	strCmd[1005]="S6TV3_RST_T3"; //(us)                                                                      
	strCmd[1006]="S6TV3_RST_T4"; //(us)                                                                      
	strCmd[1007]="S6TV3_RST_T5";	//(us)                                                                      
	strCmd[1008]="S6TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1009]="S6TV4_TTTTT1";   //(us)                                                                        
	strCmd[1010]="S6TV4_TTTTT2";   //(us)                                                                        
	strCmd[1011]="S6TV4_TTTTT3";   //(us)                                                                        
	strCmd[1012]="S6TV4_TTTTT4";   //(us)                                                                        
	strCmd[1013]="S6TV4_TTTTT5";		//(us)                                                                        
	strCmd[1014]="S6TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1015]="S6TV4_CK1_T1";	//(us)                                                                      
	strCmd[1016]="S6TV4_CK1_T2";	//(us)                                                                      
	strCmd[1017]="S6TV4_CK1_T3";	//(us)                                                                      
	strCmd[1018]="S6TV4_CK1_T4";	//(us)                                                                      
	strCmd[1019]="S6TV4_CK1_T5";	//(us)                                                                      
	strCmd[1020]="S6TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1021]="S6TV4_CK2_T1"; //(us)                                                                      
	strCmd[1022]="S6TV4_CK2_T2"; //(us)                                                                     
	strCmd[1023]="S6TV4_CK2_T3"; //(us)                                                                     
	strCmd[1024]="S6TV4_CK2_T4"; //(us)                                                                     
	strCmd[1025]="S6TV4_CK2_T5";	//(us)                                                                    
	strCmd[1026]="S6TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1027]="S6TV4_RST_T1"; //(us)                                                                     
	strCmd[1028]="S6TV4_RST_T2"; //(us)                                                                     
	strCmd[1029]="S6TV4_RST_T3"; //(us)                                                                     
	strCmd[1030]="S6TV4_RST_T4"; //(us)                                                                     
	strCmd[1031]="S6TV4_RST_T5";	//(us)                                                                    
	strCmd[1032]="S6TV4_RST_NN";

	strCmd[1033]="S7TV1_TTTTT1";   //(us)                                                                        
	strCmd[1034]="S7TV1_TTTTT2";   //(us)                                                                        
	strCmd[1035]="S7TV1_TTTTT3";   //(us)                                                                        
	strCmd[1036]="S7TV1_TTTTT4";   //(us)                                                                        
	strCmd[1037]="S7TV1_TTTTT5";		//(us)                                                                        
	strCmd[1038]="S7TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[1039]="S7TV1_CK1_T1";	//(us)                                                                      
	strCmd[1040]="S7TV1_CK1_T2";	//(us)                                                                      
	strCmd[1041]="S7TV1_CK1_T3";	//(us)                                                                      
	strCmd[1042]="S7TV1_CK1_T4";	//(us)                                                                      
	strCmd[1043]="S7TV1_CK1_T5";	//(us)                                                                      
	strCmd[1044]="S7TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[1045]="S7TV1_CK2_T1"; //(us)                                                                      
	strCmd[1046]="S7TV1_CK2_T2"; //(us)                                                                      
	strCmd[1047]="S7TV1_CK2_T3"; //(us)                                                                      
	strCmd[1048]="S7TV1_CK2_T4"; //(us)                                                                      
	strCmd[1049]="S7TV1_CK2_T5";	//(us)                                                                      
	strCmd[1050]="S7TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[1051]="S7TV1_RST_T1"; //(us)                                                                      
	strCmd[1052]="S7TV1_RST_T2"; //(us)                                                                      
	strCmd[1053]="S7TV1_RST_T3"; //(us)                                                                      
	strCmd[1054]="S7TV1_RST_T4"; //(us)                                                                      
	strCmd[1055]="S7TV1_RST_T5";	//(us)                                                                      
	strCmd[1056]="S7TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[1057]="S7TV2_TTTTT1";   //(us)                                                                        
	strCmd[1058]="S7TV2_TTTTT2";   //(us)                                                                        
	strCmd[1059]="S7TV2_TTTTT3";   //(us)                                                                        
	strCmd[1060]="S7TV2_TTTTT4";   //(us)                                                                        
	strCmd[1061]="S7TV2_TTTTT5";		//(us)                                                                        
	strCmd[1062]="S7TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[1063]="S7TV2_CK1_T1";	//(us)                                                                      
	strCmd[1064]="S7TV2_CK1_T2";	//(us)                                                                      
	strCmd[1065]="S7TV2_CK1_T3";	//(us)                                                                      
	strCmd[1066]="S7TV2_CK1_T4";	//(us)                                                                      
	strCmd[1067]="S7TV2_CK1_T5";	//(us)                                                                      
	strCmd[1068]="S7TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[1069]="S7TV2_CK2_T1"; //(us)                                                                      
	strCmd[1070]="S7TV2_CK2_T2"; //(us)                                                                      
	strCmd[1071]="S7TV2_CK2_T3"; //(us)                                                                      
	strCmd[1072]="S7TV2_CK2_T4"; //(us)                                                                      
	strCmd[1073]="S7TV2_CK2_T5";	//(us)                                                                      
	strCmd[1074]="S7TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[1075]="S7TV2_RST_T1"; //(us)                                                                      
	strCmd[1076]="S7TV2_RST_T2"; //(us)                                                                      
	strCmd[1077]="S7TV2_RST_T3"; //(us)                                                                      
	strCmd[1078]="S7TV2_RST_T4"; //(us)                                                                      
	strCmd[1079]="S7TV2_RST_T5";	//(us)                                                                      
	strCmd[1080]="S7TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[1081]="S7TV3_TTTTT1";   //(us)                                                                        
	strCmd[1082]="S7TV3_TTTTT2";   //(us)                                                                        
	strCmd[1083]="S7TV3_TTTTT3";   //(us)                                                                        
	strCmd[1084]="S7TV3_TTTTT4";   //(us)                                                                        
	strCmd[1085]="S7TV3_TTTTT5";		//(us)                                                                        
	strCmd[1086]="S7TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[1087]="S7TV3_CK1_T1";	//(us)                                                                      
	strCmd[1088]="S7TV3_CK1_T2";	//(us)                                                                      
	strCmd[1089]="S7TV3_CK1_T3";	//(us)                                                                      
	strCmd[1090]="S7TV3_CK1_T4";	//(us)                                                                      
	strCmd[1091]="S7TV3_CK1_T5";	//(us)                                                                      
	strCmd[1092]="S7TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[1093]="S7TV3_CK2_T1"; //(us)                                                                      
	strCmd[1094]="S7TV3_CK2_T2"; //(us)                                                                      
	strCmd[1095]="S7TV3_CK2_T3"; //(us)                                                                      
	strCmd[1096]="S7TV3_CK2_T4"; //(us)                                                                      
	strCmd[1097]="S7TV3_CK2_T5";	//(us)                                                                      
	strCmd[1098]="S7TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1099]="S7TV3_RST_T1"; //(us)                                                                      
	strCmd[1100]="S7TV3_RST_T2"; //(us)                                                                      
	strCmd[1101]="S7TV3_RST_T3"; //(us)                                                                      
	strCmd[1102]="S7TV3_RST_T4"; //(us)                                                                      
	strCmd[1103]="S7TV3_RST_T5";	//(us)                                                                      
	strCmd[1104]="S7TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1105]="S7TV4_TTTTT1";   //(us)                                                                        
	strCmd[1106]="S7TV4_TTTTT2";   //(us)                                                                        
	strCmd[1107]="S7TV4_TTTTT3";   //(us)                                                                        
	strCmd[1108]="S7TV4_TTTTT4";   //(us)                                                                        
	strCmd[1109]="S7TV4_TTTTT5";		//(us)                                                                        
	strCmd[1110]="S7TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1111]="S7TV4_CK1_T1";	//(us)                                                                      
	strCmd[1112]="S7TV4_CK1_T2";	//(us)                                                                      
	strCmd[1113]="S7TV4_CK1_T3";	//(us)                                                                      
	strCmd[1114]="S7TV4_CK1_T4";	//(us)                                                                      
	strCmd[1115]="S7TV4_CK1_T5";	//(us)                                                                      
	strCmd[1116]="S7TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1117]="S7TV4_CK2_T1"; //(us)                                                                      
	strCmd[1118]="S7TV4_CK2_T2"; //(us)                                                                     
	strCmd[1119]="S7TV4_CK2_T3"; //(us)                                                                     
	strCmd[1120]="S7TV4_CK2_T4"; //(us)                                                                     
	strCmd[1121]="S7TV4_CK2_T5";	//(us)                                                                    
	strCmd[1122]="S7TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1123]="S7TV4_RST_T1"; //(us)                                                                     
	strCmd[1124]="S7TV4_RST_T2"; //(us)                                                                     
	strCmd[1125]="S7TV4_RST_T3"; //(us)                                                                     
	strCmd[1126]="S7TV4_RST_T4"; //(us)                                                                     
	strCmd[1127]="S7TV4_RST_T5";	//(us)                                                                    
	strCmd[1128]="S7TV4_RST_NN";   

	strCmd[1129]="S8TV1_TTTTT1";   //(us)                                                                        
	strCmd[1130]="S8TV1_TTTTT2";   //(us)                                                                        
	strCmd[1131]="S8TV1_TTTTT3";   //(us)                                                                        
	strCmd[1132]="S8TV1_TTTTT4";   //(us)                                                                        
	strCmd[1133]="S8TV1_TTTTT5";		//(us)                                                                        
	strCmd[1134]="S8TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[1135]="S8TV1_CK1_T1";	//(us)                                                                      
	strCmd[1136]="S8TV1_CK1_T2";	//(us)                                                                      
	strCmd[1137]="S8TV1_CK1_T3";	//(us)                                                                      
	strCmd[1138]="S8TV1_CK1_T4";	//(us)                                                                      
	strCmd[1139]="S8TV1_CK1_T5";	//(us)                                                                      
	strCmd[1140]="S8TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[1141]="S8TV1_CK2_T1"; //(us)                                                                      
	strCmd[1142]="S8TV1_CK2_T2"; //(us)                                                                      
	strCmd[1143]="S8TV1_CK2_T3"; //(us)                                                                      
	strCmd[1144]="S8TV1_CK2_T4"; //(us)                                                                      
	strCmd[1145]="S8TV1_CK2_T5";	//(us)                                                                      
	strCmd[1146]="S8TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[1147]="S8TV1_RST_T1"; //(us)                                                                      
	strCmd[1148]="S8TV1_RST_T2"; //(us)                                                                      
	strCmd[1149]="S8TV1_RST_T3"; //(us)                                                                      
	strCmd[1150]="S8TV1_RST_T4"; //(us)                                                                      
	strCmd[1151]="S8TV1_RST_T5";	//(us)                                                                      
	strCmd[1152]="S8TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[1153]="S8TV2_TTTTT1";   //(us)                                                                        
	strCmd[1154]="S8TV2_TTTTT2";   //(us)                                                                        
	strCmd[1155]="S8TV2_TTTTT3";   //(us)                                                                        
	strCmd[1156]="S8TV2_TTTTT4";   //(us)                                                                        
	strCmd[1157]="S8TV2_TTTTT5";		//(us)                                                                        
	strCmd[1158]="S8TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[1159]="S8TV2_CK1_T1";	//(us)                                                                      
	strCmd[1160]="S8TV2_CK1_T2";	//(us)                                                                      
	strCmd[1161]="S8TV2_CK1_T3";	//(us)                                                                      
	strCmd[1162]="S8TV2_CK1_T4";	//(us)                                                                      
	strCmd[1163]="S8TV2_CK1_T5";	//(us)                                                                      
	strCmd[1164]="S8TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[1165]="S8TV2_CK2_T1"; //(us)                                                                      
	strCmd[1166]="S8TV2_CK2_T2"; //(us)                                                                      
	strCmd[1167]="S8TV2_CK2_T3"; //(us)                                                                      
	strCmd[1168]="S8TV2_CK2_T4"; //(us)                                                                      
	strCmd[1169]="S8TV2_CK2_T5";	//(us)                                                                      
	strCmd[1170]="S8TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[1171]="S8TV2_RST_T1"; //(us)                                                                      
	strCmd[1172]="S8TV2_RST_T2"; //(us)                                                                      
	strCmd[1173]="S8TV2_RST_T3"; //(us)                                                                      
	strCmd[1174]="S8TV2_RST_T4"; //(us)                                                                      
	strCmd[1175]="S8TV2_RST_T5";	//(us)                                                                      
	strCmd[1176]="S8TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[1177]="S8TV3_TTTTT1";   //(us)                                                                        
	strCmd[1178]="S8TV3_TTTTT2";   //(us)                                                                        
	strCmd[1179]="S8TV3_TTTTT3";   //(us)                                                                        
	strCmd[1180]="S8TV3_TTTTT4";   //(us)                                                                        
	strCmd[1181]="S8TV3_TTTTT5";		//(us)                                                                        
	strCmd[1182]="S8TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[1183]="S8TV3_CK1_T1";	//(us)                                                                      
	strCmd[1184]="S8TV3_CK1_T2";	//(us)                                                                      
	strCmd[1185]="S8TV3_CK1_T3";	//(us)                                                                      
	strCmd[1186]="S8TV3_CK1_T4";	//(us)                                                                      
	strCmd[1187]="S8TV3_CK1_T5";	//(us)                                                                      
	strCmd[1188]="S8TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[1189]="S8TV3_CK2_T1"; //(us)                                                                      
	strCmd[1190]="S8TV3_CK2_T2"; //(us)                                                                      
	strCmd[1191]="S8TV3_CK2_T3"; //(us)                                                                      
	strCmd[1192]="S8TV3_CK2_T4"; //(us)                                                                      
	strCmd[1193]="S8TV3_CK2_T5";	//(us)                                                                      
	strCmd[1194]="S8TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1195]="S8TV3_RST_T1"; //(us)                                                                      
	strCmd[1196]="S8TV3_RST_T2"; //(us)                                                                      
	strCmd[1197]="S8TV3_RST_T3"; //(us)                                                                      
	strCmd[1198]="S8TV3_RST_T4"; //(us)                                                                      
	strCmd[1199]="S8TV3_RST_T5";	//(us)                                                                      
	strCmd[1200]="S8TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1201]="S8TV4_TTTTT1";   //(us)                                                                        
	strCmd[1202]="S8TV4_TTTTT2";   //(us)                                                                        
	strCmd[1203]="S8TV4_TTTTT3";   //(us)                                                                        
	strCmd[1204]="S8TV4_TTTTT4";   //(us)                                                                        
	strCmd[1205]="S8TV4_TTTTT5";		//(us)                                                                        
	strCmd[1206]="S8TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1207]="S8TV4_CK1_T1";	//(us)                                                                      
	strCmd[1208]="S8TV4_CK1_T2";	//(us)                                                                      
	strCmd[1209]="S8TV4_CK1_T3";	//(us)                                                                      
	strCmd[1210]="S8TV4_CK1_T4";	//(us)                                                                      
	strCmd[1211]="S8TV4_CK1_T5";	//(us)                                                                      
	strCmd[1212]="S8TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1213]="S8TV4_CK2_T1"; //(us)                                                                      
	strCmd[1214]="S8TV4_CK2_T2"; //(us)                                                                     
	strCmd[1215]="S8TV4_CK2_T3"; //(us)                                                                     
	strCmd[1216]="S8TV4_CK2_T4"; //(us)                                                                     
	strCmd[1217]="S8TV4_CK2_T5";	//(us)                                                                    
	strCmd[1218]="S8TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1219]="S8TV4_RST_T1"; //(us)                                                                     
	strCmd[1220]="S8TV4_RST_T2"; //(us)                                                                     
	strCmd[1221]="S8TV4_RST_T3"; //(us)                                                                     
	strCmd[1222]="S8TV4_RST_T4"; //(us)                                                                     
	strCmd[1223]="S8TV4_RST_T5";	//(us)                                                                    
	strCmd[1224]="S8TV4_RST_NN";

	strCmd[1225]="S9TV1_TTTTT1";   //(us)                                                                        
	strCmd[1226]="S9TV1_TTTTT2";   //(us)                                                                        
	strCmd[1227]="S9TV1_TTTTT3";   //(us)                                                                        
	strCmd[1228]="S9TV1_TTTTT4";   //(us)                                                                        
	strCmd[1229]="S9TV1_TTTTT5";		//(us)                                                                        
	strCmd[1230]="S9TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[1231]="S9TV1_CK1_T1";	//(us)                                                                      
	strCmd[1232]="S9TV1_CK1_T2";	//(us)                                                                      
	strCmd[1233]="S9TV1_CK1_T3";	//(us)                                                                      
	strCmd[1234]="S9TV1_CK1_T4";	//(us)                                                                      
	strCmd[1235]="S9TV1_CK1_T5";	//(us)                                                                      
	strCmd[1236]="S9TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[1237]="S9TV1_CK2_T1"; //(us)                                                                      
	strCmd[1238]="S9TV1_CK2_T2"; //(us)                                                                      
	strCmd[1239]="S9TV1_CK2_T3"; //(us)                                                                      
	strCmd[1240]="S9TV1_CK2_T4"; //(us)                                                                      
	strCmd[1241]="S9TV1_CK2_T5";	//(us)                                                                      
	strCmd[1242]="S9TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[1243]="S9TV1_RST_T1"; //(us)                                                                      
	strCmd[1244]="S9TV1_RST_T2"; //(us)                                                                      
	strCmd[1245]="S9TV1_RST_T3"; //(us)                                                                      
	strCmd[1246]="S9TV1_RST_T4"; //(us)                                                                      
	strCmd[1247]="S9TV1_RST_T5";	//(us)                                                                      
	strCmd[1248]="S9TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[1249]="S9TV2_TTTTT1";   //(us)                                                                        
	strCmd[1250]="S9TV2_TTTTT2";   //(us)                                                                        
	strCmd[1251]="S9TV2_TTTTT3";   //(us)                                                                        
	strCmd[1252]="S9TV2_TTTTT4";   //(us)                                                                        
	strCmd[1253]="S9TV2_TTTTT5";		//(us)                                                                        
	strCmd[1254]="S9TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[1255]="S9TV2_CK1_T1";	//(us)                                                                      
	strCmd[1256]="S9TV2_CK1_T2";	//(us)                                                                      
	strCmd[1257]="S9TV2_CK1_T3";	//(us)                                                                      
	strCmd[1258]="S9TV2_CK1_T4";	//(us)                                                                      
	strCmd[1259]="S9TV2_CK1_T5";	//(us)                                                                      
	strCmd[1260]="S9TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[1261]="S9TV2_CK2_T1"; //(us)                                                                      
	strCmd[1262]="S9TV2_CK2_T2"; //(us)                                                                      
	strCmd[1263]="S9TV2_CK2_T3"; //(us)                                                                      
	strCmd[1264]="S9TV2_CK2_T4"; //(us)                                                                      
	strCmd[1265]="S9TV2_CK2_T5";	//(us)                                                                      
	strCmd[1266]="S9TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[1267]="S9TV2_RST_T1"; //(us)                                                                      
	strCmd[1268]="S9TV2_RST_T2"; //(us)                                                                      
	strCmd[1269]="S9TV2_RST_T3"; //(us)                                                                      
	strCmd[1270]="S9TV2_RST_T4"; //(us)                                                                      
	strCmd[1271]="S9TV2_RST_T5";	//(us)                                                                      
	strCmd[1272]="S9TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[1273]="S9TV3_TTTTT1";   //(us)                                                                        
	strCmd[1274]="S9TV3_TTTTT2";   //(us)                                                                        
	strCmd[1275]="S9TV3_TTTTT3";   //(us)                                                                        
	strCmd[1276]="S9TV3_TTTTT4";   //(us)                                                                        
	strCmd[1277]="S9TV3_TTTTT5";		//(us)                                                                        
	strCmd[1278]="S9TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[1279]="S9TV3_CK1_T1";	//(us)                                                                      
	strCmd[1280]="S9TV3_CK1_T2";	//(us)                                                                      
	strCmd[1281]="S9TV3_CK1_T3";	//(us)                                                                      
	strCmd[1282]="S9TV3_CK1_T4";	//(us)                                                                      
	strCmd[1283]="S9TV3_CK1_T5";	//(us)                                                                      
	strCmd[1284]="S9TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[1285]="S9TV3_CK2_T1"; //(us)                                                                      
	strCmd[1286]="S9TV3_CK2_T2"; //(us)                                                                      
	strCmd[1287]="S9TV3_CK2_T3"; //(us)                                                                      
	strCmd[1288]="S9TV3_CK2_T4"; //(us)                                                                      
	strCmd[1289]="S9TV3_CK2_T5";	//(us)                                                                      
	strCmd[1290]="S9TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1291]="S9TV3_RST_T1"; //(us)                                                                      
	strCmd[1292]="S9TV3_RST_T2"; //(us)                                                                      
	strCmd[1293]="S9TV3_RST_T3"; //(us)                                                                      
	strCmd[1294]="S9TV3_RST_T4"; //(us)                                                                      
	strCmd[1295]="S9TV3_RST_T5";	//(us)                                                                      
	strCmd[1296]="S9TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1297]="S9TV4_TTTTT1";   //(us)                                                                        
	strCmd[1298]="S9TV4_TTTTT2";   //(us)                                                                        
	strCmd[1299]="S9TV4_TTTTT3";   //(us)                                                                        
	strCmd[1300]="S9TV4_TTTTT4";   //(us)                                                                        
	strCmd[1301]="S9TV4_TTTTT5";		//(us)                                                                        
	strCmd[1302]="S9TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1303]="S9TV4_CK1_T1";	//(us)                                                                      
	strCmd[1304]="S9TV4_CK1_T2";	//(us)                                                                      
	strCmd[1305]="S9TV4_CK1_T3";	//(us)                                                                      
	strCmd[1306]="S9TV4_CK1_T4";	//(us)                                                                      
	strCmd[1307]="S9TV4_CK1_T5";	//(us)                                                                      
	strCmd[1308]="S9TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1309]="S9TV4_CK2_T1"; //(us)                                                                      
	strCmd[1310]="S9TV4_CK2_T2"; //(us)                                                                     
	strCmd[1311]="S9TV4_CK2_T3"; //(us)                                                                     
	strCmd[1312]="S9TV4_CK2_T4"; //(us)                                                                     
	strCmd[1313]="S9TV4_CK2_T5";	//(us)                                                                    
	strCmd[1314]="S9TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1315]="S9TV4_RST_T1"; //(us)                                                                     
	strCmd[1316]="S9TV4_RST_T2"; //(us)                                                                     
	strCmd[1317]="S9TV4_RST_T3"; //(us)                                                                     
	strCmd[1318]="S9TV4_RST_T4"; //(us)                                                                     
	strCmd[1319]="S9TV4_RST_T5";	//(us)                                                                    
	strCmd[1320]="S9TV4_RST_NN";    

	strCmd[1321]="S10TV1_TTTTT1";   //(us)                                                                        
	strCmd[1322]="S10TV1_TTTTT2";   //(us)                                                                        
	strCmd[1323]="S10TV1_TTTTT3";   //(us)                                                                        
	strCmd[1324]="S10TV1_TTTTT4";   //(us)                                                                        
	strCmd[1325]="S10TV1_TTTTT5";		//(us)                                                                        
	strCmd[1326]="S10TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[1327]="S10TV1_CK1_T1";	//(us)                                                                      
	strCmd[1328]="S10TV1_CK1_T2";	//(us)                                                                      
	strCmd[1329]="S10TV1_CK1_T3";	//(us)                                                                      
	strCmd[1330]="S10TV1_CK1_T4";	//(us)                                                                      
	strCmd[1331]="S10TV1_CK1_T5";	//(us)                                                                      
	strCmd[1332]="S10TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[1333]="S10TV1_CK2_T1"; //(us)                                                                      
	strCmd[1334]="S10TV1_CK2_T2"; //(us)                                                                      
	strCmd[1335]="S10TV1_CK2_T3"; //(us)                                                                      
	strCmd[1336]="S10TV1_CK2_T4"; //(us)                                                                      
	strCmd[1337]="S10TV1_CK2_T5";	//(us)                                                                      
	strCmd[1338]="S10TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[1339]="S10TV1_RST_T1"; //(us)                                                                      
	strCmd[1340]="S10TV1_RST_T2"; //(us)                                                                      
	strCmd[1341]="S10TV1_RST_T3"; //(us)                                                                      
	strCmd[1342]="S10TV1_RST_T4"; //(us)                                                                      
	strCmd[1343]="S10TV1_RST_T5";	//(us)                                                                      
	strCmd[1344]="S10TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[1345]="S10TV2_TTTTT1";   //(us)                                                                        
	strCmd[1346]="S10TV2_TTTTT2";   //(us)                                                                        
	strCmd[1347]="S10TV2_TTTTT3";   //(us)                                                                        
	strCmd[1348]="S10TV2_TTTTT4";   //(us)                                                                        
	strCmd[1349]="S10TV2_TTTTT5";		//(us)                                                                        
	strCmd[1350]="S10TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[1351]="S10TV2_CK1_T1";	//(us)                                                                      
	strCmd[1352]="S10TV2_CK1_T2";	//(us)                                                                      
	strCmd[1353]="S10TV2_CK1_T3";	//(us)                                                                      
	strCmd[1354]="S10TV2_CK1_T4";	//(us)                                                                      
	strCmd[1355]="S10TV2_CK1_T5";	//(us)                                                                      
	strCmd[1356]="S10TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[1357]="S10TV2_CK2_T1"; //(us)                                                                      
	strCmd[1358]="S10TV2_CK2_T2"; //(us)                                                                      
	strCmd[1359]="S10TV2_CK2_T3"; //(us)                                                                      
	strCmd[1360]="S10TV2_CK2_T4"; //(us)                                                                      
	strCmd[1361]="S10TV2_CK2_T5";	//(us)                                                                      
	strCmd[1362]="S10TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[1363]="S10TV2_RST_T1"; //(us)                                                                      
	strCmd[1364]="S10TV2_RST_T2"; //(us)                                                                      
	strCmd[1365]="S10TV2_RST_T3"; //(us)                                                                      
	strCmd[1366]="S10TV2_RST_T4"; //(us)                                                                      
	strCmd[1367]="S10TV2_RST_T5";	//(us)                                                                      
	strCmd[1368]="S10TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[1369]="S10TV3_TTTTT1";   //(us)                                                                        
	strCmd[1370]="S10TV3_TTTTT2";   //(us)                                                                        
	strCmd[1371]="S10TV3_TTTTT3";   //(us)                                                                        
	strCmd[1372]="S10TV3_TTTTT4";   //(us)                                                                        
	strCmd[1373]="S10TV3_TTTTT5";		//(us)                                                                        
	strCmd[1374]="S10TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[1375]="S10TV3_CK1_T1";	//(us)                                                                      
	strCmd[1376]="S10TV3_CK1_T2";	//(us)                                                                      
	strCmd[1377]="S10TV3_CK1_T3";	//(us)                                                                      
	strCmd[1378]="S10TV3_CK1_T4";	//(us)                                                                      
	strCmd[1379]="S10TV3_CK1_T5";	//(us)                                                                      
	strCmd[1380]="S10TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[1381]="S10TV3_CK2_T1"; //(us)                                                                      
	strCmd[1382]="S10TV3_CK2_T2"; //(us)                                                                      
	strCmd[1383]="S10TV3_CK2_T3"; //(us)                                                                      
	strCmd[1384]="S10TV3_CK2_T4"; //(us)                                                                      
	strCmd[1385]="S10TV3_CK2_T5";	//(us)                                                                      
	strCmd[1386]="S10TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1387]="S10TV3_RST_T1"; //(us)                                                                      
	strCmd[1388]="S10TV3_RST_T2"; //(us)                                                                      
	strCmd[1389]="S10TV3_RST_T3"; //(us)                                                                      
	strCmd[1390]="S10TV3_RST_T4"; //(us)                                                                      
	strCmd[1391]="S10TV3_RST_T5";	//(us)                                                                      
	strCmd[1392]="S10TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1393]="S10TV4_TTTTT1";   //(us)                                                                        
	strCmd[1394]="S10TV4_TTTTT2";   //(us)                                                                        
	strCmd[1395]="S10TV4_TTTTT3";   //(us)                                                                        
	strCmd[1396]="S10TV4_TTTTT4";   //(us)                                                                        
	strCmd[1397]="S10TV4_TTTTT5";		//(us)                                                                        
	strCmd[1398]="S10TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1399]="S10TV4_CK1_T1";	//(us)                                                                      
	strCmd[1400]="S10TV4_CK1_T2";	//(us)                                                                      
	strCmd[1401]="S10TV4_CK1_T3";	//(us)                                                                      
	strCmd[1402]="S10TV4_CK1_T4";	//(us)                                                                      
	strCmd[1403]="S10TV4_CK1_T5";	//(us)                                                                      
	strCmd[1404]="S10TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1405]="S10TV4_CK2_T1"; //(us)                                                                      
	strCmd[1406]="S10TV4_CK2_T2"; //(us)                                                                     
	strCmd[1407]="S10TV4_CK2_T3"; //(us)                                                                     
	strCmd[1408]="S10TV4_CK2_T4"; //(us)                                                                     
	strCmd[1409]="S10TV4_CK2_T5";	//(us)                                                                    
	strCmd[1410]="S10TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1411]="S10TV4_RST_T1"; //(us)                                                                     
	strCmd[1412]="S10TV4_RST_T2"; //(us)                                                                     
	strCmd[1413]="S10TV4_RST_T3"; //(us)                                                                     
	strCmd[1414]="S10TV4_RST_T4"; //(us)                                                                     
	strCmd[1415]="S10TV4_RST_T5";	//(us)                                                                    
	strCmd[1416]="S10TV4_RST_NN";  

	strCmd[1417]="S11TV1_TTTTT1";   //(us)                                                                        
	strCmd[1418]="S11TV1_TTTTT2";   //(us)                                                                        
	strCmd[1419]="S11TV1_TTTTT3";   //(us)                                                                        
	strCmd[1420]="S11TV1_TTTTT4";   //(us)                                                                        
	strCmd[1421]="S11TV1_TTTTT5";		//(us)                                                                        
	strCmd[1422]="S11TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[1423]="S11TV1_CK1_T1";	//(us)                                                                      
	strCmd[1424]="S11TV1_CK1_T2";	//(us)                                                                      
	strCmd[1425]="S11TV1_CK1_T3";	//(us)                                                                      
	strCmd[1426]="S11TV1_CK1_T4";	//(us)                                                                      
	strCmd[1427]="S11TV1_CK1_T5";	//(us)                                                                      
	strCmd[1428]="S11TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[1429]="S11TV1_CK2_T1"; //(us)                                                                      
	strCmd[1430]="S11TV1_CK2_T2"; //(us)                                                                      
	strCmd[1431]="S11TV1_CK2_T3"; //(us)                                                                      
	strCmd[1432]="S11TV1_CK2_T4"; //(us)                                                                      
	strCmd[1433]="S11TV1_CK2_T5";	//(us)                                                                      
	strCmd[1434]="S11TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[1435]="S11TV1_RST_T1"; //(us)                                                                      
	strCmd[1436]="S11TV1_RST_T2"; //(us)                                                                      
	strCmd[1437]="S11TV1_RST_T3"; //(us)                                                                      
	strCmd[1438]="S11TV1_RST_T4"; //(us)                                                                      
	strCmd[1439]="S11TV1_RST_T5";	//(us)                                                                      
	strCmd[1440]="S11TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[1441]="S11TV2_TTTTT1";   //(us)                                                                        
	strCmd[1442]="S11TV2_TTTTT2";   //(us)                                                                        
	strCmd[1443]="S11TV2_TTTTT3";   //(us)                                                                        
	strCmd[1444]="S11TV2_TTTTT4";   //(us)                                                                        
	strCmd[1445]="S11TV2_TTTTT5";		//(us)                                                                        
	strCmd[1446]="S11TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[1447]="S11TV2_CK1_T1";	//(us)                                                                      
	strCmd[1448]="S11TV2_CK1_T2";	//(us)                                                                      
	strCmd[1449]="S11TV2_CK1_T3";	//(us)                                                                      
	strCmd[1450]="S11TV2_CK1_T4";	//(us)                                                                      
	strCmd[1451]="S11TV2_CK1_T5";	//(us)                                                                      
	strCmd[1452]="S11TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[1453]="S11TV2_CK2_T1"; //(us)                                                                      
	strCmd[1454]="S11TV2_CK2_T2"; //(us)                                                                      
	strCmd[1455]="S11TV2_CK2_T3"; //(us)                                                                      
	strCmd[1456]="S11TV2_CK2_T4"; //(us)                                                                      
	strCmd[1457]="S11TV2_CK2_T5";	//(us)                                                                      
	strCmd[1458]="S11TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[1459]="S11TV2_RST_T1"; //(us)                                                                      
	strCmd[1460]="S11TV2_RST_T2"; //(us)                                                                      
	strCmd[1461]="S11TV2_RST_T3"; //(us)                                                                      
	strCmd[1462]="S11TV2_RST_T4"; //(us)                                                                      
	strCmd[1463]="S11TV2_RST_T5";	//(us)                                                                      
	strCmd[1464]="S11TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[1465]="S11TV3_TTTTT1";   //(us)                                                                        
	strCmd[1466]="S11TV3_TTTTT2";   //(us)                                                                        
	strCmd[1467]="S11TV3_TTTTT3";   //(us)                                                                        
	strCmd[1468]="S11TV3_TTTTT4";   //(us)                                                                        
	strCmd[1469]="S11TV3_TTTTT5";		//(us)                                                                        
	strCmd[1470]="S11TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[1471]="S11TV3_CK1_T1";	//(us)                                                                      
	strCmd[1472]="S11TV3_CK1_T2";	//(us)                                                                      
	strCmd[1473]="S11TV3_CK1_T3";	//(us)                                                                      
	strCmd[1474]="S11TV3_CK1_T4";	//(us)                                                                      
	strCmd[1475]="S11TV3_CK1_T5";	//(us)                                                                      
	strCmd[1476]="S11TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[1477]="S11TV3_CK2_T1"; //(us)                                                                      
	strCmd[1478]="S11TV3_CK2_T2"; //(us)                                                                      
	strCmd[1479]="S11TV3_CK2_T3"; //(us)                                                                      
	strCmd[1480]="S11TV3_CK2_T4"; //(us)                                                                      
	strCmd[1481]="S11TV3_CK2_T5";	//(us)                                                                      
	strCmd[1482]="S11TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1483]="S11TV3_RST_T1"; //(us)                                                                      
	strCmd[1484]="S11TV3_RST_T2"; //(us)                                                                      
	strCmd[1485]="S11TV3_RST_T3"; //(us)                                                                      
	strCmd[1486]="S11TV3_RST_T4"; //(us)                                                                      
	strCmd[1487]="S11TV3_RST_T5";	//(us)                                                                      
	strCmd[1488]="S11TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1489]="S11TV4_TTTTT1";   //(us)                                                                        
	strCmd[1490]="S11TV4_TTTTT2";   //(us)                                                                        
	strCmd[1491]="S11TV4_TTTTT3";   //(us)                                                                        
	strCmd[1492]="S11TV4_TTTTT4";   //(us)                                                                        
	strCmd[1493]="S11TV4_TTTTT5";		//(us)                                                                        
	strCmd[1494]="S11TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1495]="S11TV4_CK1_T1";	//(us)                                                                      
	strCmd[1496]="S11TV4_CK1_T2";	//(us)                                                                      
	strCmd[1497]="S11TV4_CK1_T3";	//(us)                                                                      
	strCmd[1498]="S11TV4_CK1_T4";	//(us)                                                                      
	strCmd[1499]="S11TV4_CK1_T5";	//(us)                                                                      
	strCmd[1500]="S11TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1501]="S11TV4_CK2_T1"; //(us)                                                                      
	strCmd[1502]="S11TV4_CK2_T2"; //(us)                                                                     
	strCmd[1503]="S11TV4_CK2_T3"; //(us)                                                                     
	strCmd[1504]="S11TV4_CK2_T4"; //(us)                                                                     
	strCmd[1505]="S11TV4_CK2_T5";	//(us)                                                                    
	strCmd[1506]="S11TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1507]="S11TV4_RST_T1"; //(us)                                                                     
	strCmd[1508]="S11TV4_RST_T2"; //(us)                                                                     
	strCmd[1509]="S11TV4_RST_T3"; //(us)                                                                     
	strCmd[1510]="S11TV4_RST_T4"; //(us)                                                                     
	strCmd[1511]="S11TV4_RST_T5";	//(us)                                                                    
	strCmd[1512]="S11TV4_RST_NN";

	strCmd[1513]="S12TV1_TTTTT1";   //(us)                                                                        
	strCmd[1514]="S12TV1_TTTTT2";   //(us)                                                                        
	strCmd[1515]="S12TV1_TTTTT3";   //(us)                                                                        
	strCmd[1516]="S12TV1_TTTTT4";   //(us)                                                                        
	strCmd[1517]="S12TV1_TTTTT5";		//(us)                                                                        
	strCmd[1518]="S12TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[1519]="S12TV1_CK1_T1";	//(us)                                                                      
	strCmd[1520]="S12TV1_CK1_T2";	//(us)                                                                      
	strCmd[1521]="S12TV1_CK1_T3";	//(us)                                                                      
	strCmd[1522]="S12TV1_CK1_T4";	//(us)                                                                      
	strCmd[1523]="S12TV1_CK1_T5";	//(us)                                                                      
	strCmd[1524]="S12TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[1525]="S12TV1_CK2_T1"; //(us)                                                                      
	strCmd[1526]="S12TV1_CK2_T2"; //(us)                                                                      
	strCmd[1527]="S12TV1_CK2_T3"; //(us)                                                                      
	strCmd[1528]="S12TV1_CK2_T4"; //(us)                                                                      
	strCmd[1529]="S12TV1_CK2_T5";	//(us)                                                                      
	strCmd[1530]="S12TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[1531]="S12TV1_RST_T1"; //(us)                                                                      
	strCmd[1532]="S12TV1_RST_T2"; //(us)                                                                      
	strCmd[1533]="S12TV1_RST_T3"; //(us)                                                                      
	strCmd[1534]="S12TV1_RST_T4"; //(us)                                                                      
	strCmd[1535]="S12TV1_RST_T5";	//(us)                                                                      
	strCmd[1536]="S12TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[1537]="S12TV2_TTTTT1";   //(us)                                                                        
	strCmd[1538]="S12TV2_TTTTT2";   //(us)                                                                        
	strCmd[1539]="S12TV2_TTTTT3";   //(us)                                                                        
	strCmd[1540]="S12TV2_TTTTT4";   //(us)                                                                        
	strCmd[1541]="S12TV2_TTTTT5";		//(us)                                                                        
	strCmd[1542]="S12TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[1543]="S12TV2_CK1_T1";	//(us)                                                                      
	strCmd[1544]="S12TV2_CK1_T2";	//(us)                                                                      
	strCmd[1545]="S12TV2_CK1_T3";	//(us)                                                                      
	strCmd[1546]="S12TV2_CK1_T4";	//(us)                                                                      
	strCmd[1547]="S12TV2_CK1_T5";	//(us)                                                                      
	strCmd[1548]="S12TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[1549]="S12TV2_CK2_T1"; //(us)                                                                      
	strCmd[1550]="S12TV2_CK2_T2"; //(us)                                                                      
	strCmd[1551]="S12TV2_CK2_T3"; //(us)                                                                      
	strCmd[1552]="S12TV2_CK2_T4"; //(us)                                                                      
	strCmd[1553]="S12TV2_CK2_T5";	//(us)                                                                      
	strCmd[1554]="S12TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[1555]="S12TV2_RST_T1"; //(us)                                                                      
	strCmd[1556]="S12TV2_RST_T2"; //(us)                                                                      
	strCmd[1557]="S12TV2_RST_T3"; //(us)                                                                      
	strCmd[1558]="S12TV2_RST_T4"; //(us)                                                                      
	strCmd[1559]="S12TV2_RST_T5";	//(us)                                                                      
	strCmd[1560]="S12TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[1561]="S12TV3_TTTTT1";   //(us)                                                                        
	strCmd[1562]="S12TV3_TTTTT2";   //(us)                                                                        
	strCmd[1563]="S12TV3_TTTTT3";   //(us)                                                                        
	strCmd[1564]="S12TV3_TTTTT4";   //(us)                                                                        
	strCmd[1565]="S12TV3_TTTTT5";		//(us)                                                                        
	strCmd[1566]="S12TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[1567]="S12TV3_CK1_T1";	//(us)                                                                      
	strCmd[1568]="S12TV3_CK1_T2";	//(us)                                                                      
	strCmd[1569]="S12TV3_CK1_T3";	//(us)                                                                      
	strCmd[1570]="S12TV3_CK1_T4";	//(us)                                                                      
	strCmd[1571]="S12TV3_CK1_T5";	//(us)                                                                      
	strCmd[1572]="S12TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[1573]="S12TV3_CK2_T1"; //(us)                                                                      
	strCmd[1574]="S12TV3_CK2_T2"; //(us)                                                                      
	strCmd[1575]="S12TV3_CK2_T3"; //(us)                                                                      
	strCmd[1576]="S12TV3_CK2_T4"; //(us)                                                                      
	strCmd[1577]="S12TV3_CK2_T5";	//(us)                                                                      
	strCmd[1578]="S12TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1579]="S12TV3_RST_T1"; //(us)                                                                      
	strCmd[1580]="S12TV3_RST_T2"; //(us)                                                                      
	strCmd[1581]="S12TV3_RST_T3"; //(us)                                                                      
	strCmd[1582]="S12TV3_RST_T4"; //(us)                                                                      
	strCmd[1583]="S12TV3_RST_T5";	//(us)                                                                      
	strCmd[1584]="S12TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1585]="S12TV4_TTTTT1";   //(us)                                                                        
	strCmd[1586]="S12TV4_TTTTT2";   //(us)                                                                        
	strCmd[1587]="S12TV4_TTTTT3";   //(us)                                                                        
	strCmd[1588]="S12TV4_TTTTT4";   //(us)                                                                        
	strCmd[1589]="S12TV4_TTTTT5";		//(us)                                                                        
	strCmd[1590]="S12TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1591]="S12TV4_CK1_T1";	//(us)                                                                      
	strCmd[1592]="S12TV4_CK1_T2";	//(us)                                                                      
	strCmd[1593]="S12TV4_CK1_T3";	//(us)                                                                      
	strCmd[1594]="S12TV4_CK1_T4";	//(us)                                                                      
	strCmd[1595]="S12TV4_CK1_T5";	//(us)                                                                      
	strCmd[1596]="S12TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1597]="S12TV4_CK2_T1"; //(us)                                                                      
	strCmd[1598]="S12TV4_CK2_T2"; //(us)                                                                     
	strCmd[1599]="S12TV4_CK2_T3"; //(us)                                                                     
	strCmd[1600]="S12TV4_CK2_T4"; //(us)                                                                     
	strCmd[1601]="S12TV4_CK2_T5";	//(us)                                                                    
	strCmd[1602]="S12TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1603]="S12TV4_RST_T1"; //(us)                                                                     
	strCmd[1604]="S12TV4_RST_T2"; //(us)                                                                     
	strCmd[1605]="S12TV4_RST_T3"; //(us)                                                                     
	strCmd[1606]="S12TV4_RST_T4"; //(us)                                                                     
	strCmd[1607]="S12TV4_RST_T5";	//(us)                                                                    
	strCmd[1608]="S12TV4_RST_NN";

	strCmd[1609]="S13TV1_TTTTT1";   //(us)                                                                        
	strCmd[1610]="S13TV1_TTTTT2";   //(us)                                                                        
	strCmd[1611]="S13TV1_TTTTT3";   //(us)                                                                        
	strCmd[1612]="S13TV1_TTTTT4";   //(us)                                                                        
	strCmd[1613]="S13TV1_TTTTT5";		//(us)                                                                        
	strCmd[1614]="S13TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[1615]="S13TV1_CK1_T1";	//(us)                                                                      
	strCmd[1616]="S13TV1_CK1_T2";	//(us)                                                                      
	strCmd[1617]="S13TV1_CK1_T3";	//(us)                                                                      
	strCmd[1618]="S13TV1_CK1_T4";	//(us)                                                                      
	strCmd[1619]="S13TV1_CK1_T5";	//(us)                                                                      
	strCmd[1620]="S13TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[1621]="S13TV1_CK2_T1"; //(us)                                                                      
	strCmd[1622]="S13TV1_CK2_T2"; //(us)                                                                      
	strCmd[1623]="S13TV1_CK2_T3"; //(us)                                                                      
	strCmd[1624]="S13TV1_CK2_T4"; //(us)                                                                      
	strCmd[1625]="S13TV1_CK2_T5";	//(us)                                                                      
	strCmd[1626]="S13TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[1627]="S13TV1_RST_T1"; //(us)                                                                      
	strCmd[1628]="S13TV1_RST_T2"; //(us)                                                                      
	strCmd[1629]="S13TV1_RST_T3"; //(us)                                                                      
	strCmd[1630]="S13TV1_RST_T4"; //(us)                                                                      
	strCmd[1631]="S13TV1_RST_T5";	//(us)                                                                      
	strCmd[1632]="S13TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[1633]="S13TV2_TTTTT1";   //(us)                                                                        
	strCmd[1634]="S13TV2_TTTTT2";   //(us)                                                                        
	strCmd[1635]="S13TV2_TTTTT3";   //(us)                                                                        
	strCmd[1636]="S13TV2_TTTTT4";   //(us)                                                                        
	strCmd[1637]="S13TV2_TTTTT5";		//(us)                                                                        
	strCmd[1638]="S13TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[1639]="S13TV2_CK1_T1";	//(us)                                                                      
	strCmd[1640]="S13TV2_CK1_T2";	//(us)                                                                      
	strCmd[1641]="S13TV2_CK1_T3";	//(us)                                                                      
	strCmd[1642]="S13TV2_CK1_T4";	//(us)                                                                      
	strCmd[1643]="S13TV2_CK1_T5";	//(us)                                                                      
	strCmd[1644]="S13TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[1645]="S13TV2_CK2_T1"; //(us)                                                                      
	strCmd[1646]="S13TV2_CK2_T2"; //(us)                                                                      
	strCmd[1647]="S13TV2_CK2_T3"; //(us)                                                                      
	strCmd[1648]="S13TV2_CK2_T4"; //(us)                                                                      
	strCmd[1649]="S13TV2_CK2_T5";	//(us)                                                                      
	strCmd[1650]="S13TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[1651]="S13TV2_RST_T1"; //(us)                                                                      
	strCmd[1652]="S13TV2_RST_T2"; //(us)                                                                      
	strCmd[1653]="S13TV2_RST_T3"; //(us)                                                                      
	strCmd[1654]="S13TV2_RST_T4"; //(us)                                                                      
	strCmd[1655]="S13TV2_RST_T5";	//(us)                                                                      
	strCmd[1656]="S13TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[1657]="S13TV3_TTTTT1";   //(us)                                                                        
	strCmd[1658]="S13TV3_TTTTT2";   //(us)                                                                        
	strCmd[1659]="S13TV3_TTTTT3";   //(us)                                                                        
	strCmd[1660]="S13TV3_TTTTT4";   //(us)                                                                        
	strCmd[1661]="S13TV3_TTTTT5";		//(us)                                                                        
	strCmd[1662]="S13TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[1663]="S13TV3_CK1_T1";	//(us)                                                                      
	strCmd[1664]="S13TV3_CK1_T2";	//(us)                                                                      
	strCmd[1665]="S13TV3_CK1_T3";	//(us)                                                                      
	strCmd[1666]="S13TV3_CK1_T4";	//(us)                                                                      
	strCmd[1667]="S13TV3_CK1_T5";	//(us)                                                                      
	strCmd[1668]="S13TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[1669]="S13TV3_CK2_T1"; //(us)                                                                      
	strCmd[1670]="S13TV3_CK2_T2"; //(us)                                                                      
	strCmd[1671]="S13TV3_CK2_T3"; //(us)                                                                      
	strCmd[1672]="S13TV3_CK2_T4"; //(us)                                                                      
	strCmd[1673]="S13TV3_CK2_T5";	//(us)                                                                      
	strCmd[1674]="S13TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1675]="S13TV3_RST_T1"; //(us)                                                                      
	strCmd[1676]="S13TV3_RST_T2"; //(us)                                                                      
	strCmd[1677]="S13TV3_RST_T3"; //(us)                                                                      
	strCmd[1678]="S13TV3_RST_T4"; //(us)                                                                      
	strCmd[1679]="S13TV3_RST_T5";	//(us)                                                                      
	strCmd[1680]="S13TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1681]="S13TV4_TTTTT1";   //(us)                                                                        
	strCmd[1682]="S13TV4_TTTTT2";   //(us)                                                                        
	strCmd[1683]="S13TV4_TTTTT3";   //(us)                                                                        
	strCmd[1684]="S13TV4_TTTTT4";   //(us)                                                                        
	strCmd[1685]="S13TV4_TTTTT5";		//(us)                                                                        
	strCmd[1686]="S13TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1687]="S13TV4_CK1_T1";	//(us)                                                                      
	strCmd[1688]="S13TV4_CK1_T2";	//(us)                                                                      
	strCmd[1689]="S13TV4_CK1_T3";	//(us)                                                                      
	strCmd[1690]="S13TV4_CK1_T4";	//(us)                                                                      
	strCmd[1691]="S13TV4_CK1_T5";	//(us)                                                                      
	strCmd[1692]="S13TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1693]="S13TV4_CK2_T1"; //(us)                                                                      
	strCmd[1694]="S13TV4_CK2_T2"; //(us)                                                                     
	strCmd[1695]="S13TV4_CK2_T3"; //(us)                                                                     
	strCmd[1696]="S13TV4_CK2_T4"; //(us)                                                                     
	strCmd[1697]="S13TV4_CK2_T5";	//(us)                                                                    
	strCmd[1698]="S13TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1699]="S13TV4_RST_T1"; //(us)                                                                     
	strCmd[1700]="S13TV4_RST_T2"; //(us)                                                                     
	strCmd[1701]="S13TV4_RST_T3"; //(us)                                                                     
	strCmd[1702]="S13TV4_RST_T4"; //(us)                                                                     
	strCmd[1703]="S13TV4_RST_T5";	//(us)                                                                    
	strCmd[1704]="S13TV4_RST_NN";

	strCmd[1705]="S14TV1_TTTTT1";   //(us)                                                                        
	strCmd[1706]="S14TV1_TTTTT2";   //(us)                                                                        
	strCmd[1707]="S14TV1_TTTTT3";   //(us)                                                                        
	strCmd[1708]="S14TV1_TTTTT4";   //(us)                                                                        
	strCmd[1709]="S14TV1_TTTTT5";		//(us)                                                                        
	strCmd[1710]="S14TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[1711]="S14TV1_CK1_T1";	//(us)                                                                      
	strCmd[1712]="S14TV1_CK1_T2";	//(us)                                                                      
	strCmd[1713]="S14TV1_CK1_T3";	//(us)                                                                      
	strCmd[1714]="S14TV1_CK1_T4";	//(us)                                                                      
	strCmd[1715]="S14TV1_CK1_T5";	//(us)                                                                      
	strCmd[1716]="S14TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[1717]="S14TV1_CK2_T1"; //(us)                                                                      
	strCmd[1718]="S14TV1_CK2_T2"; //(us)                                                                      
	strCmd[1719]="S14TV1_CK2_T3"; //(us)                                                                      
	strCmd[1720]="S14TV1_CK2_T4"; //(us)                                                                      
	strCmd[1721]="S14TV1_CK2_T5";	//(us)                                                                      
	strCmd[1722]="S14TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[1723]="S14TV1_RST_T1"; //(us)                                                                      
	strCmd[1724]="S14TV1_RST_T2"; //(us)                                                                      
	strCmd[1725]="S14TV1_RST_T3"; //(us)                                                                      
	strCmd[1726]="S14TV1_RST_T4"; //(us)                                                                      
	strCmd[1727]="S14TV1_RST_T5";	//(us)                                                                      
	strCmd[1728]="S14TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[1729]="S14TV2_TTTTT1";   //(us)                                                                        
	strCmd[1730]="S14TV2_TTTTT2";   //(us)                                                                        
	strCmd[1731]="S14TV2_TTTTT3";   //(us)                                                                        
	strCmd[1732]="S14TV2_TTTTT4";   //(us)                                                                        
	strCmd[1733]="S14TV2_TTTTT5";		//(us)                                                                        
	strCmd[1734]="S14TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[1735]="S14TV2_CK1_T1";	//(us)                                                                      
	strCmd[1736]="S14TV2_CK1_T2";	//(us)                                                                      
	strCmd[1737]="S14TV2_CK1_T3";	//(us)                                                                      
	strCmd[1738]="S14TV2_CK1_T4";	//(us)                                                                      
	strCmd[1739]="S14TV2_CK1_T5";	//(us)                                                                      
	strCmd[1740]="S14TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[1741]="S14TV2_CK2_T1"; //(us)                                                                      
	strCmd[1742]="S14TV2_CK2_T2"; //(us)                                                                      
	strCmd[1743]="S14TV2_CK2_T3"; //(us)                                                                      
	strCmd[1744]="S14TV2_CK2_T4"; //(us)                                                                      
	strCmd[1745]="S14TV2_CK2_T5";	//(us)                                                                      
	strCmd[1746]="S14TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[1747]="S14TV2_RST_T1"; //(us)                                                                      
	strCmd[1748]="S14TV2_RST_T2"; //(us)                                                                      
	strCmd[1749]="S14TV2_RST_T3"; //(us)                                                                      
	strCmd[1750]="S14TV2_RST_T4"; //(us)                                                                      
	strCmd[1751]="S14TV2_RST_T5";	//(us)                                                                      
	strCmd[1752]="S14TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[1753]="S14TV3_TTTTT1";   //(us)                                                                        
	strCmd[1754]="S14TV3_TTTTT2";   //(us)                                                                        
	strCmd[1755]="S14TV3_TTTTT3";   //(us)                                                                        
	strCmd[1756]="S14TV3_TTTTT4";   //(us)                                                                        
	strCmd[1757]="S14TV3_TTTTT5";		//(us)                                                                        
	strCmd[1758]="S14TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[1759]="S14TV3_CK1_T1";	//(us)                                                                      
	strCmd[1760]="S14TV3_CK1_T2";	//(us)                                                                      
	strCmd[1761]="S14TV3_CK1_T3";	//(us)                                                                      
	strCmd[1762]="S14TV3_CK1_T4";	//(us)                                                                      
	strCmd[1763]="S14TV3_CK1_T5";	//(us)                                                                      
	strCmd[1764]="S14TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[1765]="S14TV3_CK2_T1"; //(us)                                                                      
	strCmd[1766]="S14TV3_CK2_T2"; //(us)                                                                      
	strCmd[1767]="S14TV3_CK2_T3"; //(us)                                                                      
	strCmd[1768]="S14TV3_CK2_T4"; //(us)                                                                      
	strCmd[1769]="S14TV3_CK2_T5";	//(us)                                                                      
	strCmd[1770]="S14TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1771]="S14TV3_RST_T1"; //(us)                                                                      
	strCmd[1772]="S14TV3_RST_T2"; //(us)                                                                      
	strCmd[1773]="S14TV3_RST_T3"; //(us)                                                                      
	strCmd[1774]="S14TV3_RST_T4"; //(us)                                                                      
	strCmd[1775]="S14TV3_RST_T5";	//(us)                                                                      
	strCmd[1776]="S14TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1777]="S14TV4_TTTTT1";   //(us)                                                                        
	strCmd[1778]="S14TV4_TTTTT2";   //(us)                                                                        
	strCmd[1779]="S14TV4_TTTTT3";   //(us)                                                                        
	strCmd[1780]="S14TV4_TTTTT4";   //(us)                                                                        
	strCmd[1781]="S14TV4_TTTTT5";		//(us)                                                                        
	strCmd[1782]="S14TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1783]="S14TV4_CK1_T1";	//(us)                                                                      
	strCmd[1784]="S14TV4_CK1_T2";	//(us)                                                                      
	strCmd[1785]="S14TV4_CK1_T3";	//(us)                                                                      
	strCmd[1786]="S14TV4_CK1_T4";	//(us)                                                                      
	strCmd[1787]="S14TV4_CK1_T5";	//(us)                                                                      
	strCmd[1788]="S14TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1789]="S14TV4_CK2_T1"; //(us)                                                                      
	strCmd[1790]="S14TV4_CK2_T2"; //(us)                                                                     
	strCmd[1791]="S14TV4_CK2_T3"; //(us)                                                                     
	strCmd[1792]="S14TV4_CK2_T4"; //(us)                                                                     
	strCmd[1793]="S14TV4_CK2_T5";	//(us)                                                                    
	strCmd[1794]="S14TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1795]="S14TV4_RST_T1"; //(us)                                                                     
	strCmd[1796]="S14TV4_RST_T2"; //(us)                                                                     
	strCmd[1797]="S14TV4_RST_T3"; //(us)                                                                     
	strCmd[1798]="S14TV4_RST_T4"; //(us)                                                                     
	strCmd[1799]="S14TV4_RST_T5";	//(us)                                                                    
	strCmd[1800]="S14TV4_RST_NN";

	strCmd[1801]="S15TV1_TTTTT1";   //(us)                                                                        
	strCmd[1802]="S15TV1_TTTTT2";   //(us)                                                                        
	strCmd[1803]="S15TV1_TTTTT3";   //(us)                                                                        
	strCmd[1804]="S15TV1_TTTTT4";   //(us)                                                                        
	strCmd[1805]="S15TV1_TTTTT5";		//(us)                                                                        
	strCmd[1806]="S15TV1_NNNNNN";				//   //*************STV1_CK1 TIMING CONFIG*************                   
	strCmd[1807]="S15TV1_CK1_T1";	//(us)                                                                      
	strCmd[1808]="S15TV1_CK1_T2";	//(us)                                                                      
	strCmd[1809]="S15TV1_CK1_T3";	//(us)                                                                      
	strCmd[1810]="S15TV1_CK1_T4";	//(us)                                                                      
	strCmd[1811]="S15TV1_CK1_T5";	//(us)                                                                      
	strCmd[1812]="S15TV1_CK1_NN";			//                                                                                                               //*************STV1_CK2 TIMING CONFIG*************           
	strCmd[1813]="S15TV1_CK2_T1"; //(us)                                                                      
	strCmd[1814]="S15TV1_CK2_T2"; //(us)                                                                      
	strCmd[1815]="S15TV1_CK2_T3"; //(us)                                                                      
	strCmd[1816]="S15TV1_CK2_T4"; //(us)                                                                      
	strCmd[1817]="S15TV1_CK2_T5";	//(us)                                                                      
	strCmd[1818]="S15TV1_CK2_NN";			//                                                                                                             //*************STV1_RST TIMING CONFIG*************             
	strCmd[1819]="S15TV1_RST_T1"; //(us)                                                                      
	strCmd[1820]="S15TV1_RST_T2"; //(us)                                                                      
	strCmd[1821]="S15TV1_RST_T3"; //(us)                                                                      
	strCmd[1822]="S15TV1_RST_T4"; //(us)                                                                      
	strCmd[1823]="S15TV1_RST_T5";	//(us)                                                                      
	strCmd[1824]="S15TV1_RST_NN";			//                                                                                                                 //*************STV2 TIMING CONFIG*************             
	strCmd[1825]="S15TV2_TTTTT1";   //(us)                                                                        
	strCmd[1826]="S15TV2_TTTTT2";   //(us)                                                                        
	strCmd[1827]="S15TV2_TTTTT3";   //(us)                                                                        
	strCmd[1828]="S15TV2_TTTTT4";   //(us)                                                                        
	strCmd[1829]="S15TV2_TTTTT5";		//(us)                                                                        
	strCmd[1830]="S15TV2_NNNNNN";				//                                                                                                                         //*************STV1_CK2 TIMING CONFIG*************  
	strCmd[1831]="S15TV2_CK1_T1";	//(us)                                                                      
	strCmd[1832]="S15TV2_CK1_T2";	//(us)                                                                      
	strCmd[1833]="S15TV2_CK1_T3";	//(us)                                                                      
	strCmd[1834]="S15TV2_CK1_T4";	//(us)                                                                      
	strCmd[1835]="S15TV2_CK1_T5";	//(us)                                                                      
	strCmd[1836]="S15TV2_CK1_NN";	//                                                                                                          //*************STV2_CK2 TIMING CONFIG*************                   
	strCmd[1837]="S15TV2_CK2_T1"; //(us)                                                                      
	strCmd[1838]="S15TV2_CK2_T2"; //(us)                                                                      
	strCmd[1839]="S15TV2_CK2_T3"; //(us)                                                                      
	strCmd[1840]="S15TV2_CK2_T4"; //(us)                                                                      
	strCmd[1841]="S15TV2_CK2_T5";	//(us)                                                                      
	strCmd[1842]="S15TV2_CK2_NN";	//                                                                                                              //*************STV2_RST TIMING CONFIG*************                
	strCmd[1843]="S15TV2_RST_T1"; //(us)                                                                      
	strCmd[1844]="S15TV2_RST_T2"; //(us)                                                                      
	strCmd[1845]="S15TV2_RST_T3"; //(us)                                                                      
	strCmd[1846]="S15TV2_RST_T4"; //(us)                                                                      
	strCmd[1847]="S15TV2_RST_T5";	//(us)                                                                      
	strCmd[1848]="S15TV2_RST_NN";        	//                                                                                                               //*************STV3 TIMING CONFIG*************           
	strCmd[1849]="S15TV3_TTTTT1";   //(us)                                                                        
	strCmd[1850]="S15TV3_TTTTT2";   //(us)                                                                        
	strCmd[1851]="S15TV3_TTTTT3";   //(us)                                                                        
	strCmd[1852]="S15TV3_TTTTT4";   //(us)                                                                        
	strCmd[1853]="S15TV3_TTTTT5";		//(us)                                                                        
	strCmd[1854]="S15TV3_NNNNNN";   //                                                                                                                                   //*************STV3_CK1 TIMING CONFIG**********
	strCmd[1855]="S15TV3_CK1_T1";	//(us)                                                                      
	strCmd[1856]="S15TV3_CK1_T2";	//(us)                                                                      
	strCmd[1857]="S15TV3_CK1_T3";	//(us)                                                                      
	strCmd[1858]="S15TV3_CK1_T4";	//(us)                                                                      
	strCmd[1859]="S15TV3_CK1_T5";	//(us)                                                                      
	strCmd[1860]="S15TV3_CK1_NN";	  //                                                                                               //*************STV3_CK2 TIMING CONFIG*************                             
	strCmd[1861]="S15TV3_CK2_T1"; //(us)                                                                      
	strCmd[1862]="S15TV3_CK2_T2"; //(us)                                                                      
	strCmd[1863]="S15TV3_CK2_T3"; //(us)                                                                      
	strCmd[1864]="S15TV3_CK2_T4"; //(us)                                                                      
	strCmd[1865]="S15TV3_CK2_T5";	//(us)                                                                      
	strCmd[1866]="S15TV3_CK2_NN";  //                                                                                                  //*************STV3_RST TIMING CONFIG*************                           
	strCmd[1867]="S15TV3_RST_T1"; //(us)                                                                      
	strCmd[1868]="S15TV3_RST_T2"; //(us)                                                                      
	strCmd[1869]="S15TV3_RST_T3"; //(us)                                                                      
	strCmd[1870]="S15TV3_RST_T4"; //(us)                                                                      
	strCmd[1871]="S15TV3_RST_T5";	//(us)                                                                      
	strCmd[1872]="S15TV3_RST_NN";			//                                                                                                       //*************STV4 TIMING CONFIG*************                       
	strCmd[1873]="S15TV4_TTTTT1";   //(us)                                                                        
	strCmd[1874]="S15TV4_TTTTT2";   //(us)                                                                        
	strCmd[1875]="S15TV4_TTTTT3";   //(us)                                                                        
	strCmd[1876]="S15TV4_TTTTT4";   //(us)                                                                        
	strCmd[1877]="S15TV4_TTTTT5";		//(us)                                                                        
	strCmd[1878]="S15TV4_NNNNNN";		//                                                                                                             //*************STV4_CK1 TIMING CONFIG*************                   
	strCmd[1879]="S15TV4_CK1_T1";	//(us)                                                                      
	strCmd[1880]="S15TV4_CK1_T2";	//(us)                                                                      
	strCmd[1881]="S15TV4_CK1_T3";	//(us)                                                                      
	strCmd[1882]="S15TV4_CK1_T4";	//(us)                                                                      
	strCmd[1883]="S15TV4_CK1_T5";	//(us)                                                                      
	strCmd[1884]="S15TV4_CK1_NN";	 //                                                                                                        //*************STV4_CK2 TIMING CONFIG*************                     
	strCmd[1885]="S15TV4_CK2_T1"; //(us)                                                                      
	strCmd[1886]="S15TV4_CK2_T2"; //(us)                                                                     
	strCmd[1887]="S15TV4_CK2_T3"; //(us)                                                                     
	strCmd[1888]="S15TV4_CK2_T4"; //(us)                                                                     
	strCmd[1889]="S15TV4_CK2_T5";	//(us)                                                                    
	strCmd[1890]="S15TV4_CK2_NN";	   //                                                                                                      //*************STV4_RST TIMING CONFIG*************                     
	strCmd[1891]="S15TV4_RST_T1"; //(us)                                                                     
	strCmd[1892]="S15TV4_RST_T2"; //(us)                                                                     
	strCmd[1893]="S15TV4_RST_T3"; //(us)                                                                     
	strCmd[1894]="S15TV4_RST_T4"; //(us)                                                                     
	strCmd[1895]="S15TV4_RST_T5";	//(us)                                                                    
	strCmd[1896]="S15TV4_RST_NN";    

	strCmd[1897]="FPGA1_PIC_NUM";	//                                                                   
	strCmd[1898]="FPGA1_STV_NUM";
	strCmd[1899]="Project_Num";         
	strCmd[1900]="VCOM1_L";


	strCmd[1901]="T1R1H_NN";	
	strCmd[1902]="T1R1L_NN";	
	strCmd[1903]="T1R2H_NN";	
	strCmd[1904]="T1R2L_NN";	
	strCmd[1905]="T1R3H_NN";
	strCmd[1906]="T1R3L_NN";	
	strCmd[1907]="T1R4H_NN";	
	strCmd[1908]="T1R4L_NN"; 
	strCmd[1909]="T1G1H_NN";	
	strCmd[1910]="T1G1L_NN";	
	strCmd[1911]="T1G2H_NN";	
	strCmd[1912]="T1G2L_NN";	
	strCmd[1913]="T1G3H_NN";	
	strCmd[1914]="T1G3L_NN";	
	strCmd[1915]="T1G4H_NN";	
	strCmd[1916]="T1G4L_NN"; 
	strCmd[1917]="T1B1H_NN";	
	strCmd[1918]="T1B1L_NN";	
	strCmd[1919]="T1B2H_NN";	
	strCmd[1920]="T1B2L_NN";	
	strCmd[1921]="T1B3H_NN";	
	strCmd[1922]="T1B3L_NN";	
	strCmd[1923]="T1B4H_NN";	
	strCmd[1924]="T1B4L_NN"; 

	strCmd[1925]="T2R1H_NN";	
	strCmd[1926]="T2R1L_NN";	
	strCmd[1927]="T2R2H_NN";	
	strCmd[1928]="T2R2L_NN";	
	strCmd[1929]="T2R3H_NN"; 
	strCmd[1930]="T2R3L_NN";	
	strCmd[1931]="T2R4H_NN";	
	strCmd[1932]="T2R4L_NN"; 
	strCmd[1933]="T2G1H_NN";	
	strCmd[1934]="T2G1L_NN";	
	strCmd[1935]="T2G2H_NN";	
	strCmd[1936]="T2G2L_NN";	
	strCmd[1937]="T2G3H_NN";	
	strCmd[1938]="T2G3L_NN";	
	strCmd[1939]="T2G4H_NN";	
	strCmd[1940]="T2G4L_NN"; 
	strCmd[1941]="T2B1H_NN";	
	strCmd[1942]="T2B1L_NN";	
	strCmd[1943]="T2B2H_NN";	
	strCmd[1944]="T2B2L_NN";	
	strCmd[1945]="T2B3H_NN";	
	strCmd[1946]="T2B3L_NN";	
	strCmd[1947]="T2B4H_NN";	
	strCmd[1948]="T2B4L_NN"; 

	strCmd[1949]="T3R1H_NN";	
	strCmd[1950]="T3R1L_NN";	
	strCmd[1951]="T3R2H_NN";	
	strCmd[1952]="T3R2L_NN";	
	strCmd[1953]="T3R3H_NN"; 
	strCmd[1954]="T3R3L_NN";	
	strCmd[1955]="T3R4H_NN";	
	strCmd[1956]="T3R4L_NN"; 
	strCmd[1957]="T3G1H_NN";	
	strCmd[1958]="T3G1L_NN";	
	strCmd[1959]="T3G2H_NN";	
	strCmd[1960]="T3G2L_NN";	
	strCmd[1961]="T3G3H_NN";	
	strCmd[1962]="T3G3L_NN";	
	strCmd[1963]="T3G4H_NN";	
	strCmd[1964]="T3G4L_NN"; 
	strCmd[1965]="T3B1H_NN";	
	strCmd[1966]="T3B1L_NN";	
	strCmd[1967]="T3B2H_NN";	
	strCmd[1968]="T3B2L_NN";	
	strCmd[1969]="T3B3H_NN";	
	strCmd[1970]="T3B3L_NN";	
	strCmd[1971]="T3B4H_NN";	
	strCmd[1972]="T3B4L_NN"; 

	strCmd[1973]="T4R1H_NN";	
	strCmd[1974]="T4R1L_NN";	
	strCmd[1975]="T4R2H_NN";	
	strCmd[1976]="T4R2L_NN";	
	strCmd[1977]="T4R3H_NN"; 
	strCmd[1978]="T4R3L_NN";	
	strCmd[1979]="T4R4H_NN";	
	strCmd[1980]="T4R4L_NN"; 
	strCmd[1981]="T4G1H_NN";	
	strCmd[1982]="T4G1L_NN";	
	strCmd[1983]="T4G2H_NN";	
	strCmd[1984]="T4G2L_NN";	
	strCmd[1985]="T4G3H_NN";	
	strCmd[1986]="T4G3L_NN";	
	strCmd[1987]="T4G4H_NN";	
	strCmd[1988]="T4G4L_NN"; 
	strCmd[1989]="T4B1H_NN";	
	strCmd[1990]="T4B1L_NN";	
	strCmd[1991]="T4B2H_NN";	
	strCmd[1992]="T4B2L_NN";	
	strCmd[1993]="T4B3H_NN";	
	strCmd[1994]="T4B3L_NN";	
	strCmd[1995]="T4B4H_NN";	
	strCmd[1996]="T4B4L_NN"; 

	strCmd[1997]="T5R1H_NN";	
	strCmd[1998]="T5R1L_NN";	
	strCmd[1999]="T5R2H_NN";	
	strCmd[2000]="T5R2L_NN";	
	strCmd[2001]="T5R3H_NN"; 
	strCmd[2002]="T5R3L_NN";	
	strCmd[2003]="T5R4H_NN";	
	strCmd[2004]="T5R4L_NN"; 
	strCmd[2005]="T5G1H_NN";	
	strCmd[2006]="T5G1L_NN";	
	strCmd[2007]="T5G2H_NN";	
	strCmd[2008]="T5G2L_NN";	
	strCmd[2009]="T5G3H_NN";	
	strCmd[2010]="T5G3L_NN";	
	strCmd[2011]="T5G4H_NN";	
	strCmd[2012]="T5G4L_NN"; 
	strCmd[2013]="T5B1H_NN";	
	strCmd[2014]="T5B1L_NN";	
	strCmd[2015]="T5B2H_NN";	
	strCmd[2016]="T5B2L_NN";	
	strCmd[2017]="T5B3H_NN";	
	strCmd[2018]="T5B3L_NN";	
	strCmd[2019]="T5B4H_NN";	
	strCmd[2020]="T5B4L_NN"; 

	strCmd[2021]="T6R1H_NN";	
	strCmd[2022]="T6R1L_NN";	
	strCmd[2023]="T6R2H_NN";	
	strCmd[2024]="T6R2L_NN";	
	strCmd[2025]="T6R3H_NN"; 
	strCmd[2026]="T6R3L_NN";	
	strCmd[2027]="T6R4H_NN";	
	strCmd[2028]="T6R4L_NN"; 
	strCmd[2029]="T6G1H_NN";	
	strCmd[2030]="T6G1L_NN";	
	strCmd[2031]="T6G2H_NN";	
	strCmd[2032]="T6G2L_NN";	
	strCmd[2033]="T6G3H_NN";	
	strCmd[2034]="T6G3L_NN";	
	strCmd[2035]="T6G4H_NN";	
	strCmd[2036]="T6G4L_NN"; 
	strCmd[2037]="T6B1H_NN";	
	strCmd[2038]="T6B1L_NN";	
	strCmd[2039]="T6B2H_NN";	
	strCmd[2040]="T6B2L_NN";	
	strCmd[2041]="T6B3H_NN";	
	strCmd[2042]="T6B3L_NN";	
	strCmd[2043]="T6B4H_NN";	
	strCmd[2044]="T6B4L_NN";
	 
	strCmd[2045]="T7R1H_NN";	
	strCmd[2046]="T7R1L_NN";	
	strCmd[2047]="T7R2H_NN";	
	strCmd[2048]="T7R2L_NN";	
	strCmd[2049]="T7R3H_NN"; 
	strCmd[2050]="T7R3L_NN";	
	strCmd[2051]="T7R4H_NN";	
	strCmd[2052]="T7R4L_NN"; 
	strCmd[2053]="T7G1H_NN";	
	strCmd[2054]="T7G1L_NN";	
	strCmd[2055]="T7G2H_NN";	
	strCmd[2056]="T7G2L_NN";	
	strCmd[2057]="T7G3H_NN";	
	strCmd[2058]="T7G3L_NN";	
	strCmd[2059]="T7G4H_NN";	
	strCmd[2060]="T7G4L_NN"; 
	strCmd[2061]="T7B1H_NN";	
	strCmd[2062]="T7B1L_NN";	
	strCmd[2063]="T7B2H_NN";	
	strCmd[2064]="T7B2L_NN";	
	strCmd[2065]="T7B3H_NN";	
	strCmd[2066]="T7B3L_NN";	
	strCmd[2067]="T7B4H_NN";	
	strCmd[2068]="T7B4L_NN"; 

	strCmd[2069]="T8R1H_NN";	
	strCmd[2070]="T8R1L_NN";	
	strCmd[2071]="T8R2H_NN";	
	strCmd[2072]="T8R2L_NN";	
	strCmd[2073]="T8R3H_NN"; 
	strCmd[2074]="T8R3L_NN";	
	strCmd[2075]="T8R4H_NN";	
	strCmd[2076]="T8R4L_NN"; 
	strCmd[2077]="T8G1H_NN";	
	strCmd[2078]="T8G1L_NN";	
	strCmd[2079]="T8G2H_NN";	
	strCmd[2080]="T8G2L_NN";	
	strCmd[2081]="T8G3H_NN";	
	strCmd[2082]="T8G3L_NN";	
	strCmd[2083]="T8G4H_NN";	
	strCmd[2084]="T8G4L_NN"; 
	strCmd[2085]="T8B1H_NN";	
	strCmd[2086]="T8B1L_NN";	
	strCmd[2087]="T8B2H_NN";	
	strCmd[2088]="T8B2L_NN";	
	strCmd[2089]="T8B3H_NN";	
	strCmd[2090]="T8B3L_NN";	
	strCmd[2091]="T8B4H_NN";	
	strCmd[2092]="T8B4L_NN";
	 
	strCmd[2093]="T9R1H_NN";	
	strCmd[2094]="T9R1L_NN";	
	strCmd[2095]="T9R2H_NN";	
	strCmd[2096]="T9R2L_NN";	
	strCmd[2097]="T9R3H_NN"; 
	strCmd[2098]="T9R3L_NN";	
	strCmd[2099]="T9R4H_NN";	
	strCmd[2100]="T9R4L_NN"; 
	strCmd[2101]="T9G1H_NN";	
	strCmd[2102]="T9G1L_NN";	
	strCmd[2103]="T9G2H_NN";	
	strCmd[2104]="T9G2L_NN";	
	strCmd[2105]="T9G3H_NN";	
	strCmd[2106]="T9G3L_NN";	
	strCmd[2107]="T9G4H_NN";	
	strCmd[2108]="T9G4L_NN"; 
	strCmd[2109]="T9B1H_NN";	
	strCmd[2110]="T9B1L_NN";	
	strCmd[2111]="T9B2H_NN";	
	strCmd[2112]="T9B2L_NN";	
	strCmd[2113]="T9B3H_NN";	
	strCmd[2114]="T9B3L_NN";	
	strCmd[2115]="T9B4H_NN";	
	strCmd[2116]="T9B4L_NN"; 
			 
	strCmd[2117]="T10R1H_N";	
	strCmd[2118]="T10R1L_N";	
	strCmd[2119]="T10R2H_N";	
	strCmd[2120]="T10R2L_N";	
	strCmd[2121]="T10R3H_N"; 
	strCmd[2122]="T10R3L_N";	
	strCmd[2123]="T10R4H_N";	
	strCmd[2124]="T10R4L_N"; 
	strCmd[2125]="T10G1H_N";	
	strCmd[2126]="T10G1L_N";	
	strCmd[2127]="T10G2H_N";	
	strCmd[2128]="T10G2L_N";	
	strCmd[2129]="T10G3H_N";	
	strCmd[2130]="T10G3L_N";	
	strCmd[2131]="T10G4H_N";	
	strCmd[2132]="T10G4L_N"; 
	strCmd[2133]="T10B1H_N";	
	strCmd[2134]="T10B1L_N";	
	strCmd[2135]="T10B2H_N";	
	strCmd[2136]="T10B2L_N";	
	strCmd[2137]="T10B3H_N";	
	strCmd[2138]="T10B3L_N";	
	strCmd[2139]="T10B4H_N";	
	strCmd[2140]="T10B4L_N"; 
	 
	strCmd[2141]="T11R1H_N";	
	strCmd[2142]="T11R1L_N";	
	strCmd[2143]="T11R2H_N";	
	strCmd[2144]="T11R2L_N";	
	strCmd[2145]="T11R3H_N"; 
	strCmd[2146]="T11R3L_N";	
	strCmd[2147]="T11R4H_N";	
	strCmd[2148]="T11R4L_N"; 
	strCmd[2149]="T11G1H_N";	
	strCmd[2150]="T11G1L_N";	
	strCmd[2151]="T11G2H_N";	
	strCmd[2152]="T11G2L_N";	
	strCmd[2153]="T11G3H_N";	
	strCmd[2154]="T11G3L_N";	
	strCmd[2155]="T11G4H_N";	
	strCmd[2156]="T11G4L_N"; 
	strCmd[2157]="T11B1H_N";	
	strCmd[2158]="T11B1L_N";	
	strCmd[2159]="T11B2H_N";	
	strCmd[2160]="T11B2L_N";	
	strCmd[2161]="T11B3H_N";	
	strCmd[2162]="T11B3L_N";	
	strCmd[2163]="T11B4H_N";	
	strCmd[2164]="T11B4L_N"; 
		
	strCmd[2165]="T12R1H_N";	
	strCmd[2166]="T12R1L_N";	
	strCmd[2167]="T12R2H_N";	
	strCmd[2168]="T12R2L_N";	
	strCmd[2169]="T12R3H_N"; 
	strCmd[2170]="T12R3L_N";	
	strCmd[2171]="T12R4H_N";	
	strCmd[2172]="T12R4L_N"; 
	strCmd[2173]="T12G1H_N";	
	strCmd[2174]="T12G1L_N";	
	strCmd[2175]="T12G2H_N";	
	strCmd[2176]="T12G2L_N";	
	strCmd[2177]="T12G3H_N";	
	strCmd[2178]="T12G3L_N";	
	strCmd[2179]="T12G4H_N";	
	strCmd[2180]="T12G4L_N"; 
	strCmd[2181]="T12B1H_N";	
	strCmd[2182]="T12B1L_N";	
	strCmd[2183]="T12B2H_N";	
	strCmd[2184]="T12B2L_N";	
	strCmd[2185]="T12B3H_N";	
	strCmd[2186]="T12B3L_N";	
	strCmd[2187]="T12B4H_N";	
	strCmd[2188]="T12B4L_N";
				
	strCmd[2189]="T13R1H_N";	
	strCmd[2190]="T13R1L_N";	
	strCmd[2191]="T13R2H_N";	
	strCmd[2192]="T13R2L_N";	
	strCmd[2193]="T13R3H_N"; 
	strCmd[2194]="T13R3L_N";	
	strCmd[2195]="T13R4H_N";	
	strCmd[2196]="T13R4L_N"; 
	strCmd[2197]="T13G1H_N";	
	strCmd[2198]="T13G1L_N";	
	strCmd[2199]="T13G2H_N";	
	strCmd[2200]="T13G2L_N";	
	strCmd[2201]="T13G3H_N";	
	strCmd[2202]="T13G3L_N";	
	strCmd[2203]="T13G4H_N";	
	strCmd[2204]="T13G4L_N"; 
	strCmd[2205]="T13B1H_N";	
	strCmd[2206]="T13B1L_N";	
	strCmd[2207]="T13B2H_N";	
	strCmd[2208]="T13B2L_N";	
	strCmd[2209]="T13B3H_N";	
	strCmd[2210]="T13B3L_N";	
	strCmd[2211]="T13B4H_N";	
	strCmd[2212]="T13B4L_N"; 
		 
	strCmd[2213]="T14R1H_N";	
	strCmd[2214]="T14R1L_N";	
	strCmd[2215]="T14R2H_N";	
	strCmd[2216]="T14R2L_N";	
	strCmd[2217]="T14R3H_N"; 
	strCmd[2218]="T14R3L_N";	
	strCmd[2219]="T14R4H_N";	
	strCmd[2220]="T14R4L_N"; 
	strCmd[2221]="T14G1H_N";	
	strCmd[2222]="T14G1L_N";	
	strCmd[2223]="T14G2H_N";	
	strCmd[2224]="T14G2L_N";	
	strCmd[2225]="T14G3H_N";	
	strCmd[2226]="T14G3L_N";	
	strCmd[2227]="T14G4H_N";	
	strCmd[2228]="T14G4L_N"; 
	strCmd[2229]="T14B1H_N";	
	strCmd[2230]="T14B1L_N";	
	strCmd[2231]="T14B2H_N";	
	strCmd[2232]="T14B2L_N";	
	strCmd[2233]="T14B3H_N";	
	strCmd[2234]="T14B3L_N";	
	strCmd[2235]="T14B4H_N";	
	strCmd[2236]="T14B4L_N"; 
			
	strCmd[2237]="T15R1H_N";	
	strCmd[2238]="T15R1L_N";	
	strCmd[2239]="T15R2H_N";	
	strCmd[2240]="T15R2L_N";	
	strCmd[2241]="T15R3H_N"; 
	strCmd[2242]="T15R3L_N";	
	strCmd[2243]="T15R4H_N";	
	strCmd[2244]="T15R4L_N"; 
	strCmd[2245]="T15G1H_N";	
	strCmd[2246]="T15G1L_N";	
	strCmd[2247]="T15G2H_N";	
	strCmd[2248]="T15G2L_N";	
	strCmd[2249]="T15G3H_N";	
	strCmd[2250]="T15G3L_N";	
	strCmd[2251]="T15G4H_N";	
	strCmd[2252]="T15G4L_N"; 
	strCmd[2253]="T15B1H_N";	
	strCmd[2254]="T15B1L_N";	
	strCmd[2255]="T15B2H_N";	
	strCmd[2256]="T15B2L_N";	
	strCmd[2257]="T15B3H_N";	
	strCmd[2258]="T15B3L_N";	
	strCmd[2259]="T15B4H_N";	
	strCmd[2260]="T15B4L_N";   

	strCmd[2261]="Tvcom1H_NN";
	strCmd[2262]="Tvcom1L_NN";
	strCmd[2263]="Tvcom2H_NN";
	strCmd[2264]="Tvcom2L_NN";
	strCmd[2265]="Tvcom3H_NN";
	strCmd[2266]="Tvcom3L_NN";
	strCmd[2267]="Tvcom4H_NN";
	strCmd[2268]="Tvcom4L_NN";
	strCmd[2269]="Tvcom5H_NN";
	strCmd[2270]="Tvcom5L_NN";
	strCmd[2271]="Tvcom6H_NN";
	strCmd[2272]="Tvcom6L_NN";
	strCmd[2273]="Tvcom7H_NN";
	strCmd[2274]="Tvcom7L_NN";
	strCmd[2275]="Tvcom8H_NN";
	strCmd[2276]="Tvcom8L_NN";
	strCmd[2277]="Tvcom9H_NN";
	strCmd[2278]="Tvcom9L_NN";
	strCmd[2279]="Tvcom10H_N";
	strCmd[2280]="Tvcom10L_N";
	strCmd[2281]="Tvcom11H_N";
	strCmd[2282]="Tvcom11L_N";
	strCmd[2283]="Tvcom12H_N";
	strCmd[2284]="Tvcom12L_N";
	strCmd[2285]="Tvcom13H_N";
	strCmd[2286]="Tvcom13L_N";
	strCmd[2287]="Tvcom14H_N";
	strCmd[2288]="Tvcom14L_N";
	strCmd[2289]="Tvcom15H_N";
	strCmd[2290]="Tvcom15L_N";
	strCmd[2291]="Pro_Maxnum";
}
	
	while(1)                                                                                                                                                                                                                                                                                                          
	{                                                                                                                                                                                                                                                                                                                 
		if ( strncmp(strCmd[0],dest,12 ) == 0 )                                                                                                                                                                                                                                                                         
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=0;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[1],dest,9 ) == 0 )                                                                                                                                                                                                                                                                          
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=1;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[2],dest,3 ) == 0 )                                                               	    	                                                                                                                                                                                                    
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=2;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strcmp(strCmd[3],dest) == 0 )                                                                                                                                                                                                                                                                              
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=3;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[4],dest,9 ) == 0 )                                                                                                                                                                                                                                                                          
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=4;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[5],dest,9 ) == 0 )                                                                                                                                                                                                                                                                          
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=5;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[6],dest,4 ) == 0 )                                                                                                                                                                                                                                                                          
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=6;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[7],dest,4 ) == 0 )                                                                                                                                                                                                                                                                          
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=7;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																									 
		 if ( strncmp(strCmd[8],dest,4 ) == 0 )                                                                                                                                                                                                                                                                         
		 {                                                                                                                                                                                                                                                                                                              
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=8;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		 }                                                                                                                                                                                                                                                                                                              
																																																																																																																																																										
		if ( strncmp(strCmd[9],dest,4 ) == 0 )                                                                                                                                                                                                                                                                          
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=9;                                                                                                                                                                                                                                                                                                          
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[10],dest,7 ) == 0 )                                                                                                                                                                                                                                                                         
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=10;                                                                                                                                                                                                                                                                                                         
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[11],dest,4 ) == 0 )                                                                                                                                                                                                                                                                         
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=11;                                                                                                                                                                                                                                                                                                         
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[12],dest,4 ) == 0 )                                                                                                                                                                                                                                                                         
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=12;                                                                                                                                                                                                                                                                                                         
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[13],dest,10 ) == 0 )                                                                                                                                                                                                                                                                        
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=13;                                                                                                                                                                                                                                                                                                         
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if ( strncmp(strCmd[14],dest,10 ) == 0 )                                                                                                                                                                                                                                                                        
		{                                                                                                                                                                                                                                                                                                               
			flag = 1;                                                                                                                                                                                                                                                                                                     
			i=14;                                                                                                                                                                                                                                                                                                         
			break;                                                                                                                                                                                                                                                                                                        
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		for ( i=15; i<=248; i++ )                       //pic1_pic9                                                                                                                                                                                                                                                     
		{                                                                                                                                                                                                                                                                                                               
			if ( strncmp(strCmd[i],dest,11 ) == 0 )                                                                                                                                                                                                                                                                       
			{                                                                                                                                                                                                                                                                                                             
				flag = 1;                                                                                                                                                                                                                                                                                                   
				break;                                                                                                                                                                                                                                                                                                      
			}                                                                                                                                                                                                                                                                                                             
		}                                                                                                                                                                                                                                                                                                               
		if(((15<=i)&&(i<=248))&&(flag==1))  break;                                                                                                                                                                                                                                                                      
																																																																																																																																																										
		for ( i=249; i<=456; i++ )                          //pic10_pic17                                                                                                                                                                                                                                               
		{                                                                                                                                                                                                                                                                                                               
			if ( strncmp(strCmd[i],dest,12 ) == 0 )                                                                                                                                                                                                                                                                       
			{                                                                                                                                                                                                                                                                                                             
				flag = 1;                                                                                                                                                                                                                                                                                                   
				break;                                                                                                                                                                                                                                                                                                      
			}                                                                                                                                                                                                                                                                                                             
		}                                                                                                                                                                                                                                                                                                               
		if(((249<=i)&&(i<=456))&&(flag==1))  break;           //                                                                                                                                                                                                                                                        
																																																																																																																																																										
		for ( i=457; i<=1320; i++ )                                     //s1tv _s9tv                                                                                                                                                                                                                                    
		{                                                                                                                                                                                                                                                                                                               
			if ( strncmp(strCmd[i],dest,12 ) == 0 )                                                                                                                                                                                                                                                                       
			{                                                                                                                                                                                                                                                                                                             
				flag = 1;                                                                                                                                                                                                                                                                                                   
				break;                                                                                                                                                                                                                                                                                                      
			}                                                                                                                                                                                                                                                                                                             
		}                                                                                                                                                                                                                                                                                                               
		if(((457<=i)&&(i<=1320))&&(flag==1))  break;           //	                                                                                                                                                                                                                                                      
																																																																																																																																																										
		for ( i=1321; i<=1896; i++ )                              //s10tv_s20tv                                                                                                                                                                                                                                         
		{                                                                                                                                                                                                                                                                                                               
			if ( strncmp(strCmd[i],dest,13 ) == 0 )                                                                                                                                                                                                                                                                       
			{                                                                                                                                                                                                                                                                                                             
				flag = 1;                                                                                                                                                                                                                                                                                                   
				break;                                                                                                                                                                                                                                                                                                      
			}                                                                                                                                                                                                                                                                                                             
		}                                                                                                                                                                                                                                                                                                               
		if(((1321<=i)&&(i<=1896))&&(flag==1))  break;           //		                                                                                                                                                                                                                                                  
																																																																																																																																																								
																																																																																																																																																										
		if( strncmp(strCmd[1897],dest,13 ) == 0 )       //FPGA1_PIC_NUM                                                                                                                                                                                                                                                 
		{                                                                                                                                                                                                                                                                                                               
			 flag = 1;                                                                                                                                                                                                                                                                                                    
			 i=1897;                                                                                                                                                                                                                                                                                                      
			 break;                                                                                                                                                                                                                                                                                                       
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if( strncmp(strCmd[1898],dest,13 ) == 0 )      //FPGA1_STV_NUM                                                                                                                                                                                                                                                  
		{                                                                                                                                                                                                                                                                                                               
			 flag = 1;                                                                                                                                                                                                                                                                                                    
			 i=1898;                                                                                                                                                                                                                                                                                                      
			 break;                                                                                                                                                                                                                                                                                                       
		}                                                                                                                                                                                                                                                                                                               
																																																																																																																																																										
		if( strncmp(strCmd[1899],dest,11 ) == 0 )      //FPGA1 project number                                                                                                                                                                                                                                           
		{                                                                                                                                                                                                                                                                                                               
			 flag = 1;                                                                                                                                                                                                                                                                                                    
			 i=1899;                                                                                                                                                                                                                                                                                                      
			 break;                                                                                                                                                                                                                                                                                                       
		}   

		if( strncmp(strCmd[1900],dest,7 ) == 0 )      //FPGA1 project number                                                                                                                                                                                                                                           
		{                                                                                                                                                                                                                                                                                                               
			 flag = 1;                                                                                                                                                                                                                                                                                                    
			 i=1900;                                                                                                                                                                                                                                                                                                      
			 break;                                                                                                                                                                                                                                                                                                       
		} 
		
		
		for( i=1901; i<=2260; i++ )                           //TRGB_N1_ TRGB-N15                                                                                                                                                                                                                                        
		{                                                                                                                                                                                                                                                                                                               
			if ( strncmp(strCmd[i],dest,8 ) == 0 )                                                                                                                                                                                                                                                                       
			{                                                                                                                                                                                                                                                                                                             
				flag = 1;                                                                                                                                                                                                                                                                                                   
				break;                                                                                                                                                                                                                                                                                                      
			}                                                                                                                                                                                                                                                                                                             
		}                                                                                                                                                                                                                                                                                                               
		if(((1901<=i)&&(i<=2260))&&(flag==1))  break;           //		                                                                                                                                                                                                                                                  
																																			 
		
		for( i=2261; i<=2291; i++ )                           //Tvcom1_15                                                                                                                                                                                                                                      
		{                                                                                                                                                                                                                                                                                                               
			if ( strncmp(strCmd[i],dest,10 ) == 0 )                                                                                                                                                                                                                                                                       
			{                                                                                                                                                                                                                                                                                                             
				flag = 1;                                                                                                                                                                                                                                                                                                   
				break;                                                                                                                                                                                                                                                                                                      
			}                                                                                                                                                                                                                                                                                                             
		}                                                                                                                                                                                                                                                                                                               
		if(((2261<=i)&&(i<=2291))&&(flag==1))  break;           //		       
				
		break;                                                                                                                                                                                                                                                                                                                   
	}                                                                                                                                                                                                                                                                                                                

  if ( flag == 1 )                                                                                                                                                                                                                                                                                                
  {                                                                                                                                                                                                                                                                                                               
  	return i;                                                                                                                                                                                                                                                                                                     
  }                                                                                                                                                                                                                                                                                                               
  else                                                                                                                                                                                                                                                                                                            
  {                                                                                                                                                                                                                                                                                                               
  	return -1;                                                                                                                                                                                                                                                                                                    
  }                                                                                                                                                                                                                                                                                                               
} 


void setStv(uint16_t index_base,uint16_t index,uint8_t nn,char *para2,FPGA1_STV *buf)
{
	FPGA1_STV *STV;
	
	STV = buf;
	
	while(1)                                //FPGA1 parameter     //s6tv _s10tv
	{
	  if ( index == index_base+24*nn )
	  {
	    STV->stv[nn].STV_TTTTT1=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+1+24*nn )
	  {
	    STV->stv[nn].STV_TTTTT2=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+2+24*nn )
	  {
	    STV->stv[nn].STV_TTTTT3=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+3+24*nn )
	  {
	    STV->stv[nn].STV_TTTTT4=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+4+24*nn )
	  {
	    STV->stv[nn].STV_TTTTT5=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+5+24*nn )
	  {
	    STV->stv[nn].STV_NNNNNN=(int)atof(para2);
	    break;
	  }
	  
	  if ( index == index_base+6+24*nn )
	  {
	    STV->stv[nn].STV_CK1_T1=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+7+24*nn )
	  {
	    STV->stv[nn].STV_CK1_T2=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+8+24*nn )
	  {
	    STV->stv[nn].STV_CK1_T3=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+9+24*nn )
	  {
	    STV->stv[nn].STV_CK1_T4=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+10+24*nn )
	  {
	    STV->stv[nn].STV_CK1_T5=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+11+24*nn )
	  {
	    STV->stv[nn].STV_CK1_NN=(int)atof(para2);
	    break;
	  }
	  
	  if ( index == index_base+12+24*nn )
	  {
	    STV->stv[nn].STV_CK2_T1=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+13+24*nn )
	  {
	    STV->stv[nn].STV_CK2_T2=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+14+24*nn )
	  {
	    STV->stv[nn].STV_CK2_T3=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+15+24*nn )
	  {
	    STV->stv[nn].STV_CK2_T4=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+16+24*nn )
	  {
	    STV->stv[nn].STV_CK2_T5=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+17+24*nn )
	  {
	    STV->stv[nn].STV_CK2_NN=(int)atof(para2);
	    break;
	  }
	  
	  if ( index == index_base+18+24*nn )
	  {
	    STV->stv[nn].STV_RST_T1=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+19+24*nn )
	  {
	    STV->stv[nn].STV_RST_T2=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+20+24*nn )
	  {
	    STV->stv[nn].STV_RST_T3=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+21+24*nn )
	  {
	    STV->stv[nn].STV_RST_T4=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+22+24*nn )
	  {
	    STV->stv[nn].STV_RST_T5=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == index_base+23+24*nn )
	  {
	    STV->stv[nn].STV_RST_NN=(int)atof(para2);
	    break;
	  }
		break;
	  
	}   
}

void SetPara( int index, char*para2 )
{
  int n=0,n1=0,n2=0,nn=0,nnn=0,m=0;
  double x;
  
  if ( index == 0 )
	{
		strcpy(BL_fpga1.Project_Name,para2);
	}
	
	if(index==1)
	{
	  strcpy(BL_fpga1.CFG_Part1,para2);
	}
	
	if(index==2)
	{
		x=atof(para2);
	  BL_fpga1.VBL=(int)((73.47 - x) / 45.0 * 1024);
	}
	
	if(index==3)
	{
		x=atof(para2);
	  BL_fpga1.IBL=(int)(3.4374*x+1718.6975);
	}
	
	if(index==4)
	{
		x=atof(para2);
	  BL_fpga1.IBL_LIMIT=(int)(2.048*x+1024);
	}
	
	if(index==5)
	{
	  strcpy(BL_fpga1.CFG_Part2,para2);
	}
	
	if(index==6)
	{
		x=atof(para2);
	  BL_fpga1.VGL1=(int)(-200*x);
	}
	
	if(index==7)
	{
		x=atof(para2);
	  BL_fpga1.VGH1=(int)(130.4348*x);
	}
	
	if(index==8)
	{
		x=atof(para2);
	  BL_fpga1.VGG1=(int)(130.4348*x);
	}
	
	if(index==9)
	{
		x=atof(para2);
	  BL_fpga1.VSW1=(int)(2400-80*x);
	}
	
	if(index==10)
	{
		x=atof(para2);
	  BL_fpga1.VCOM1=(int)(2000-100*x);
	}
	
	if(index==11)
	{
		x=atof(para2);
	  BL_fpga1.VBW1=(int)(2000-100*x);
	}
	
	if(index==12)
	{
		x=atof(para2);
	  BL_fpga1.VFW1=(int)(2000-100*x);
	}
	
	if(index==13)
	{
		x=atof(para2);
	  BL_fpga1.VTIMING_H1=(int)(203.1873*x);
	}
	
	if(index==14)
	{
		x=atof(para2);
	  BL_fpga1.VTIMING_L1=(int)(-195*x);
	}
	
	if((15<=index)&&(index<=40))                           //FPGA1 picture parameter
	n1=0;
	if((41<=index)&&(index<=66))
	n1=1;
	if((67<=index)&&(index<=92))
	n1=2;
	if((93<=index)&&(index<=118))
	n1=3;
	if((119<=index)&&(index<=144))
	n1=4;
	if((145<=index)&&(index<=170))
	n1=5;
	if((171<=index)&&(index<=196))
	n1=6;
	if((197<=index)&&(index<=222))
	n1=7;
  if((223<=index)&&(index<=248))
	n1=8;
	if((249<=index)&&(index<=274))
	n1=9;
	
	if((275<=index)&&(index<=300))                           //FPGA1 picture parameter
	n1=10;
	if((301<=index)&&(index<=326))
	n1=11;
	if((327<=index)&&(index<=352))
	n1=12;
	if((353<=index)&&(index<=378))
	n1=13;
	if((379<=index)&&(index<=404))
	n1=14;
	if((405<=index)&&(index<=430))
	n1=15;
	if((431<=index)&&(index<=456))
	n1=16;
	
	if((15<=index)&&(index<=456)) 
	while(1)
	{
	  if ( index == 15+26*n1 )
	  {	 
	    strcpy(BL_fpga1.fpic[n1].FPIC_NAMEE,para2);
	    break;
	  }
	  
	  if ( index == 16+26*n1 )
	  {
	    BL_fpga1.fpic[n1].FPIC_TIMEE=(int)((float)atof(para2)*10);
	    break;
	  }
	  
	  if ( index == 17+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_R1_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 18+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_R1_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 19+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_G1_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 20+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_G1_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 21+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_B1_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 22+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_B1_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 23+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_R2_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 24+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_R2_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 25+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_G2_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 26+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_G2_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 27+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_B2_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 28+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_B2_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 29+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_R3_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 30+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_R3_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 31+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_G3_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 32+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_G3_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 33+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_B3_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 34+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_B3_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 35+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_R4_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 36+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_R4_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 37+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_G4_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 38+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_G4_VL=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 39+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_B4_VH=(int)(2000-100*x);
	    break;
	  }
	  
	  if ( index == 40+26*n1 )
	  {	
	    x=atof(para2);
	    BL_fpga1.fpic[n1].FPIC_B4_VL=(int)(2000-100*x);
      break;	  
	  }
	  
		break;
	}                                                                   //FPGA1 picture parameter
	
	if(index==1897)
	{		
	  BL_fpga1.FPGA1_PIC_NUM=atof(para2);
	}
	
	if(index==1898)
	{
	  BL_fpga1.FPGA1_STV_NUM=atof(para2);
	}
	
	if(index==1899)
	{
	  BL_fpga1.Project_Num=atof(para2);
	}
	
	if(index==1900)
	{		
	  x=atof(para2);
	  BL_fpga1.VCOM1_L=(int)(2000-100*x);
	}
	
	{
	if ((457<=index)&&(index<=480))                          //FPGA1   time parameter   s1tv
	n=0;
  if ((481<=index)&&(index<=504))
	n=1;
	if ((505<=index)&&(index<=528))
	n=2;
  if ((529<=index)&&(index<=552))
	n=3;  
	
	if ((553<=index)&&(index<=576))                          //FPGA1   time parameter s2tv
	n=4;
  if ((577<=index)&&(index<=600))
	n=5;
	if ((601<=index)&&(index<=624))
	n=6;
  if ((625<=index)&&(index<=648))
	n=7;     
	
	if ((649<=index)&&(index<=672))                          //FPGA1   time parameter  s3tv
	n=8;
  if ((673<=index)&&(index<=696))
	n=9;
	if ((697<=index)&&(index<=720))
	n=10;
  if ((721<=index)&&(index<=744))
	n=11;       
	
	if ((745<=index)&&(index<=768))                          //FPGA1   time parameter  s4tv
	n=12;
  if ((769<=index)&&(index<=792))
	n=13;
	if ((793<=index)&&(index<=816))
	n=14;
  if ((817<=index)&&(index<=840))
	n=15;     
	
	if ((841<=index)&&(index<=864))                          //FPGA1   time parameter  s5tv
	n=16;
  if ((865<=index)&&(index<=888))
	n=17;
	if ((889<=index)&&(index<=912))
	n=18;
  if ((913<=index)&&(index<=936))
	n=19;     
	
	if((457<=index)&&(index<=936))				//FPGA1 parameter     //s1tv _s5tv
	{
		setStv(457,index,n,para2,&fpga1_STV1);
	}

	if ((937<=index)&&(index<=960))                          //FPGA1   time parameter   s6tv
	nn=0;
  if ((961<=index)&&(index<=984))
	nn=1;
	if ((985<=index)&&(index<=1008))
	nn=2;
  if ((1009<=index)&&(index<=1032))
	nn=3;  
	
	if ((1033<=index)&&(index<=1056))                          //FPGA1   time parameter s7tv
	nn=4;
  if ((1057<=index)&&(index<=1080))
	nn=5;
	if ((1081<=index)&&(index<=1104))
	nn=6;
  if ((1105<=index)&&(index<=1128))
	nn=7;     
	
	if ((1129<=index)&&(index<=1152))                          //FPGA1   time parameter  s8tv
	nn=8;
  if ((1153<=index)&&(index<=1176))
	nn=9;
	if ((1177<=index)&&(index<=1200))
	nn=10;
  if ((1201<=index)&&(index<=1224))
	nn=11;       
	
	if ((1225<=index)&&(index<=1248))                          //FPGA1   time parameter  s9tv
	nn=12;
  if ((1249<=index)&&(index<=1272))
	nn=13;
	if ((1273<=index)&&(index<=1296))
	nn=14;
  if ((1297<=index)&&(index<=1320))
	nn=15;     
	
	if ((1321<=index)&&(index<=1344))                          //FPGA1   time parameter  s10tv
	nn=16;
  if ((1345<=index)&&(index<=1368))
	nn=17;
	if ((1369<=index)&&(index<=1392))
	nn=18;
  if ((1393<=index)&&(index<=1416))
	nn=19;     
	
	if((937<=index)&&(index<=1416))				//FPGA1 parameter     //s6tv _s10tv	
	{
		setStv(937,index,nn,para2,&fpga1_STV2);
	}
		
	if ((1417<=index)&&(index<=1440))                          //FPGA1   time parameter   s11tv
	nnn=0;
  if ((1441<=index)&&(index<=1464))
	nnn=1;
	if ((1465<=index)&&(index<=1488))
	nnn=2;
  if ((1489<=index)&&(index<=1512))
	nnn=3;  
	
	if ((1513<=index)&&(index<=1536))                          //FPGA1   time parameter s12tv
	nnn=4;
  if ((1537<=index)&&(index<=1560))
	nnn=5;
	if ((1561<=index)&&(index<=1584))
	nnn=6;
  if ((1585<=index)&&(index<=1608))
	nnn=7;     
	
	if ((1609<=index)&&(index<=1632))                          //FPGA1   time parameter  s13tv
	nnn=8;
  if ((1633<=index)&&(index<=1656))
	nnn=9;
	if ((1657<=index)&&(index<=1680))
	nnn=10;
  if ((1681<=index)&&(index<=1704))
	nnn=11;       
	
	if ((1705<=index)&&(index<=1728))                          //FPGA1   time parameter  s14tv
	nnn=12;
  if ((1729<=index)&&(index<=1752))
	nnn=13;
	if ((1753<=index)&&(index<=1776))
	nnn=14;
  if ((1777<=index)&&(index<=1800))
	nnn=15;     
	
	if ((1801<=index)&&(index<=1824))                          //FPGA1   time parameter  s15tv
	nnn=16;
  if ((1825<=index)&&(index<=1848))
	nnn=17;
	if ((1849<=index)&&(index<=1872))
	nnn=18;
  if ((1873<=index)&&(index<=1896))
	nnn=19;     
	
	if((1417<=index)&&(index<=1896))							//FPGA1 parameter     //s11tv _s15tv
	{
		setStv(1417,index,nnn,para2,&fpga1_STV3);
	}
	}	
	if((1901<=index)&&(index<=1924))                           //TRGB_N1_15
	m=0;
	if((1925<=index)&&(index<=1948))
	m=1;
	if((1949<=index)&&(index<=1972))
	m=2;
	if((1973<=index)&&(index<=1996))
	m=3;
	if((1997<=index)&&(index<=2020))
	m=4;
	if((2021<=index)&&(index<=2044))
	m=5;
	if((2045<=index)&&(index<=2068))
	m=6;
	if((2069<=index)&&(index<=2092))
	m=7;
  if((2093<=index)&&(index<=2116))
	m=8;
	if((2117<=index)&&(index<=2140))
	m=9;
	
	if((2141<=index)&&(index<=2164))                           //FPGA1 picture parameter
	m=10;
	if((2165<=index)&&(index<=2188))
	m=11;
	if((2189<=index)&&(index<=2212))
	m=12;
	if((2213<=index)&&(index<=2236))
	m=13;
	if((2237<=index)&&(index<=2260))
	m=14;

	if((1901<=index)&&(index<=2260)) 
	while(1)
	{
	  if ( index == 1901+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TR1H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1902+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TR1L_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1903+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TR2H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1904+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TR2L_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1905+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TR3H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1906+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TR3L_N=(char)x;
	    break;
	  }
	  
	   if ( index == 1907+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TR4H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1908+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TR4L_N=(char)x;
	    break;
	  }
	 
	  if ( index == 1909+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TG1H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1910+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TG1L_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1911+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TG2H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1912+24*m )
	  {	
	    x=atof(para2);
      fpga1_rgb_n.rgb_n[m].TG2L_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1913+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TG3H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1914+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TG3L_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1915+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TG4H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1916+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TG4L_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1917+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TB1H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1918+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TB1L_N=(char)x;
	    break;
	  }
	  
	   if ( index == 1919+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TB2H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1920+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TB2L_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1921+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TB3H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1922+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TB3L_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1923+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TB4H_N=(char)x;
	    break;
	  }
	  
	  if ( index == 1924+24*m )
	  {	
	    x=atof(para2);
	    fpga1_rgb_n.rgb_n[m].TB4L_N=(char)x;
      break;
	  }
	  	  
	  break;
	}          
	
	if(index==2261)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom1H_N=(char)x;
	}
	
	if(index==2262)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom1L_N=(char)x;
	}
	
	if(index==2263)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom2H_N=(char)x;
	}
	
	if(index==2264)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom2L_N=(char)x;
	}
	
	if(index==2265)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom3H_N=(char)x;
	}
	
	if(index==2266)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom3L_N=(char)x;
	}
	
	if(index==2267)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom4H_N=(char)x;
	}
	
	if(index==2268)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom4L_N=(char)x;
	}
	
	if(index==2269)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom5H_N=(char)x;
	}
	
	if(index==2270)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom5L_N=(char)x;
	}
	
	if(index==2271)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom6H_N=(char)x;
	}
	
	if(index==2272)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom6L_N=(char)x;
	}
	
	if(index==2273)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom7H_N=(char)x;
	}
	
	if(index==2274)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom7L_N=(char)x;
	}
	
	if(index==2275)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom8H_N=(char)x;
	}
	
	if(index==2276)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom8L_N=(char)x;
	}
	
	if(index==2277)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom9H_N=(char)x;
	}
	
	if(index==2278)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom9L_N=(char)x;
	}
	
	if(index==2279)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom10H_N=(char)x;
	}
	
	if(index==2280)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom10L_N=(char)x;
	}
	
	if(index==2281)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom11H_N=(char)x;
	}
	
	if(index==2282)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom11L_N=(char)x;
	}
	
	if(index==2283)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom12H_N=(char)x;
	}
	
	if(index==2284)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom12L_N=(char)x;
	}
	
	if(index==2285)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom13H_N=(char)x;
	}
	
	if(index==2286)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom13L_N=(char)x;
	}
	
	if(index==2287)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom14H_N=(char)x;
	}
	
	if(index==2288)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom14L_N=(char)x;
	}
	
	if(index==2289)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom15H_N=(char)x;
	}
	
	if(index==2290)
	{		
	  x=atof(para2);
	  fpga1_rgb_n.Tvcom15L_N=(char)x;
	}
	
	if(index==2291)
	{		
	  x=atoi(para2);
	  BL_fpga1.Pro_Maxnum=(char)x;
	}
}

void analysisCfgFile(void)
{
	int i = 0;
	int j = 0;
	int ret = 0;
	int flag = 0;
	uint32_t byteRead = 0;
	char buff[100] = {0};
	char para1[100] = {0};
	char para2[100] = {0};

  while(1)
	{
		ret = f_read(&MyFiles,buff,1,&byteRead);
		if ( ( strcmp(para1, "END") == 0 ) )
		{
			jiexiwan_flag=1;
			break;
		}
		
		if (  ret  || (byteRead == 0) )
		{
			break;
		}
		
		if ( flag == 0 )
		{
				if ( buff[0] == '\r' || buff[0] == '\n' || buff[0] == ' ' || buff[0] == '#')
				{
					continue;
				}
				if ( (buff[0]!='=')&&(buff[0]!=0x09))
				{
					para1[i] = buff[0];
					i++;
				}
				else if ( buff[0] =='=' )
				{
					i=0;
					flag = 1;
					continue;
				}
		}
		if ( flag == 1 )
		{
				if ( ( buff[0] == '\r' ) || ( buff[0] == '\n' ) )
				{
					flag = 0;
					i = 0;
					ret = strCompare(para1);
					if ( ret != -1 )
					{
						SetPara(ret,para2);
					}

					for ( j=0; j<100; j++ )
					{
						para1[j] = 0;
						para2[j] = 0;
					}
					continue;
				}
				else if ( (buff[0] == '=') )//|| (buff[0]==';') || (buff[0]==' ') )
				{
					continue;
				}
				else if((buff[0]!=';')&&(buff[0]!=' '))
				{					
					para2[i] = buff[0];
					i++;
				}
				else if((buff[0]==';')||(buff[0]==' '))
				{
					continue;
        }			
		}		
  }
}

void MSC_App3()
{
	uint8_t i,read_num=1,pro_number=1;
	BL_FPGA1 bl_temp,*bl;
	uint8_t filename[22];
	
	DeleteDirFile(FS_VOLUME_SD);
	for(i=0;i<pro_number;i++)
	{
		analysisCfgFile();                                     //
		if(	jiexiwan_flag==1)                           
		{                                               
			jiexiwan_flag=0;
//			CreateNewFile(i,name_to_str(BL_fpga1),&BL_fpga1,sizeof(BL_fpga1));
			CreateNewFile(i);		 
		}
		if(read_num)
		{
			read_num=0;
			sprintf(filename, "Project00BL_fpga1");	
			ReadFileData(FS_VOLUME_SD,filename,&bl_temp,sizeof(bl_temp));
//			bl = (BL_FPGA1*)&bl_temp;
			pro_number=bl_temp.Pro_Maxnum;
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: ReadFileData
*	功能说明: 读取文件armfly.txt前128个字符，并打印到串口
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void ReadFileData(char *_ucVolume,char *_fileName,void *buf,uint16_t len)
{
	/* 本函数使用的局部变量占用较多，请修改任务栈大小，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
//	char buf[128];
	char path[5];
	char pathname[32];

 	/* 挂载文件系统 */
	result = f_mount(&fs, _ucVolume, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

	/* 打开根文件夹 */
	sprintf(path, "%s/", _ucVolume);
	result = f_opendir(&DirInf, path); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
		return;
	}

	/* 打开文件 */
	sprintf(pathname, "%s%s.txt", path , _fileName);
	result = f_open(&file, pathname, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
		printf("Don't Find File : %s\r\n",pathname);
		return;
	}

	/* 读取文件 */
	result = f_read(&file, buf, len - 1, &bw);
	if (bw > 0)
	{
//		buf[bw] = 0;
		*((char*)buf+bw) = 0;
//		printf("\r\narmfly.txt 文件内容 : \r\n%s\r\n", buf);
		printf("\r\narmfly.txt 文件读取成功 \r\n\r\n");
	}
	else
	{
		printf("\r\narmfly.txt 文件内容 : \r\n");
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL, path, 0);
}
static void DeleteDirFile(char *_ucVolume)
{
	/* 本函数使用的局部变量占用较多，请修改任务栈大小，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	char FileName[13];
	uint32_t cnt = 0;
	DIR DirInf;
	FILINFO FileInf;
	char lfname[20];
	char path[32];

 	/* 挂载文件系统 */
	result = f_mount(&fs, _ucVolume, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}

	#if 0
	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%s)\r\n", FR_Table[result]);
		return;
	}
	#endif
	
	/* 打开根文件夹 */
	sprintf(path, "%s/", _ucVolume);	 /* 1: 表示盘符 */
	result = f_opendir(&DirInf, path);
	if (result != FR_OK)
	{
		printf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 读取当前文件夹下的文件和目录 */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf,&FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}

		if (FileInf.fname[0] == '.')
		{
			continue;
		}

		/* 判断是文件还是子目录 */
		if (!(FileInf.fattrib & AM_DIR))
		{
			if(strstr((char *)FileInf.lfname,"Project"))
			{				
				result = f_unlink(FileInf.lfname);
				if (result == FR_OK)
				{
					printf("删除文件%s成功\r\n", (char *)FileInf.lfname);
				}
			}
		}
	}

	/* 卸载文件系统 */
	f_mount(NULL, _ucVolume, 0);
}


/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/


