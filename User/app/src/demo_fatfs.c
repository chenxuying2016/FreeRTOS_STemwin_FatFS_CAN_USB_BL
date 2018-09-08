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

extern TaskHandle_t xHandleTaskGUI;

char cfgname[40][16] = {0};
uint8_t jiexiwan_flag=0;
CONFIG_PARA cfg_para;
struct _CONFIG_CHANNEL cfg_allch_tmp;
struct _CONFIG_CHANNEL_ cfg_ch_tmp;
struct _CONFIG_DATA cfg_data_tmp;
double corretPara[52] = {0};
char flag_correct = 0;
char correctbuf[550]={0};

extern FIL MyFiles;
//extern char proname[640][16];
extern char currentPro;

/* 仅允许本文件内调用的函数声明 */
static void MSC_App3(int pronum);

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

void readPro(char maxProNum)
{
	char cur_file_name[16] = {0};
	char (*ptProName)[16];
	
	ptProName = (char (*)[16])pvPortMalloc(640*16);//sizeof(char)*640*16

//	ViewRootDir(FS_VOLUME_SD,proname);
//	addPro(maxProNum,proname);
	ViewRootDir(FS_VOLUME_SD,ptProName);
	addPro(maxProNum,ptProName);
	vPortFree(ptProName);
	
	memset(cur_file_name,0,sizeof(cur_file_name));
	sprintf(cur_file_name, "proname");
	ReadFileData(FS_VOLUME_SD,cur_file_name,&currentPro,1);
}

/*
*********************************************************************************************************
*	函 数 名: DemoFatFS
*	功能说明: FatFS文件系统演示主程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
char DemoFatFS(void)
{
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	uint32_t bw;
	char path[32]={0};
	uint32_t i;
	char max_pro_num = 0;
		
	max_pro_num = ViewRootDir(FS_VOLUME_USB,cfgname);
//	ViewRootDir(FS_VOLUME_SD,proname);
//	addPro(max_pro_num);
	readPro(max_pro_num);
		
//	DeleteDirFile(FS_VOLUME_SD);
	CreateNewFile(0,0,1);

	result = f_mount(&fs, FS_VOLUME_USB, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
//		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
		return 0;
	}

	sprintf(path, "%s/", FS_VOLUME_USB);
	result = f_opendir(&DirInf, path); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
//		printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
		return 0;
	}

	for(i = 0;i < max_pro_num;i++)
	{
		sprintf(path, "%s/%s.txt", FS_VOLUME_USB,cfgname[i]);
		result = f_open(&MyFiles, path, FA_OPEN_EXISTING | FA_READ);
		if (result !=  FR_OK)
		{
//			printf("Don't Find File : %s\r\n",path);
			return 0;
		}
		
		MSC_App3(i);

		/* 关闭文件*/
		f_close(&MyFiles);
	}
	
	/* 卸载文件系统 */
	f_mount(NULL, FS_VOLUME_USB, 0);
	
	return max_pro_num;
//		vTaskDelay(10);
}

/*
*********************************************************************************************************
*	函 数 名: ViewRootDir
*	功能说明: 显示SD卡根目录下的文件名
*	形    参: _ucVolume : 磁盘卷标 0 - 2
*	返 回 值: 无
*********************************************************************************************************
*/
int ViewRootDir(char *_ucVolume,char buf[][16])
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t i,cnt = 0;
	char lfname[32];
	char path[32];

 	/* 挂载文件系统 */
	result = f_mount(&fs, _ucVolume, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
//		printf("挂载文件系统失败 (%d)\r\n", result);
		return 0;
	}

	/* 打开根文件夹 */
	sprintf(path, "%s/", _ucVolume);	 /* 1: 表示盘符 */
	result = f_opendir(&DirInf, path);
	if (result != FR_OK)
	{
//		printf("打开根目录失败 (%d)\r\n", result);
		return 0;
	}

	/* 读取当前文件夹下的文件和目录 */
	FileInf.lfname = lfname;
	FileInf.lfsize = 32;

	for (i = 0; ;i++)
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
//			if(!strcmp(FileInf.fname,"jiaozhun.txt") && !strcmp(_ucVolume,"2:"))
			if(!strcmp(FileInf.fname,"jiaozhun.txt"))
			{
				if(!strcmp(_ucVolume,"2:"))
					flag_correct = 1;
				continue;
			}
			if(!strcmp(FileInf.fname,"proname.txt"))
			{
//				flag_correct = 1;
				continue;
			}
			if(strstr(FileInf.fname,".txt"))
			{
					strncpy(buf[cnt],FileInf.fname,strlen(FileInf.fname)-4);
					++cnt;
			}
			if(strstr(FileInf.lfname,".txt"))
			{
					strncpy(buf[cnt],FileInf.lfname,strlen(FileInf.lfname)-4);
					++cnt;
			}
		}
	}

	/* 卸载文件系统 */
	f_mount(NULL,_ucVolume, 0);
	return cnt;
}

void MSC_App3(int pronum)
{	
	uint8_t i,pic_number=1;

	for(i=0;i<pic_number;i++)
	{
		analysisCfgFile();                                     //
		if(	jiexiwan_flag==1)                           
		{                                               
			jiexiwan_flag=0;
			CreateNewFile(pronum,i,0);
		}
		pic_number = cfg_para.picNum +2;
	}
}

void analysisCfgFile(void)
{	
	FRESULT result;
	int i=0,count=0;
	char flag = 1;
	uint32_t byteRead = 0;
	char buf[16]={0},para1[64]={0},para2[64]={0};

	for(;;){
		result = f_read(&MyFiles, &buf, 1, &byteRead);
		if(result  || (byteRead == 0) )
			break;
		
		if (strcmp(para1, "END") == 0)
		{
			jiexiwan_flag=1;
			count = 0;
			break;
		}
		
		if(flag)
		{
			if((buf[0] == ' ') || (buf[0] == '\n') || (buf[0] == '\r') || buf[0] == '#') 
			{
				continue;
			}
			else if(buf[0] == '=')
			{
				i = 0;
				flag = 0;
				continue;
			}
			else
			{
				para1[i++] = buf[0];
			}
		}else{
			if((buf[0] == ' ') || (buf[0] == ';'))
			{
				continue;
			}
			else if((buf[0] == '\n') || (buf[0] == '\r'))
			{
				i = 0;
				flag = 1;
				count++;
				strCompare(para1,para2,count);
				memset(para1,0,64);
				memset(para2,0,64);
			}
			else
			{
				para2[i++] = buf[0];
			}
		}			
	}
}

void strCompare( char *title, char* value,int len)
{	
	switch(len)
	{
		case 1:
			if(memcmp("picmaxn",title,7) == 0)
				cfg_para.picMaxN = atoi(value);
			break;
			
		case 2:
			if(memcmp("picname",title,7) == 0)
				strncpy(cfg_para.picName,value,7);
			break;
			
		case 3:
			if(memcmp("picnum",title,6) == 0)
				cfg_para.picNum = atoi(value);
			break;
			
		case 4:
			if(memcmp("voltage",title,7) == 0)
				cfg_para.voltage = atoi(value);
			break;
			
		case 5:
			if(memcmp("current",title,7) == 0)
				cfg_para.current = atoi(value);
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			zuCompare(title,value);
			break;
			
		case 18:
			if(memcmp("total_time",title,9) == 0)
				cfg_para.totalTime = atoi(value);
			break;
			
		default:
			chCompare(title,value);
			break;
	}		
}

void zuCompare(char * name,char * value)
{
	char num[2]={0},level[2]={0};
	strncpy(num,name+5,1);
	strncpy(level,name+7,1);
	
	switch(atoi(num))
	{
		case 1:
			if(memcmp("h",level,1)==0)
				cfg_para.cfg_ch_zu.ch_zu1_h = atoi(value);
			else
				cfg_para.cfg_ch_zu.ch_zu1_l = atoi(value);
			break;
		case 2:
			if(memcmp("h",level,1)==0)
				cfg_para.cfg_ch_zu.ch_zu2_h = atoi(value);
			else
				cfg_para.cfg_ch_zu.ch_zu2_l = atoi(value);
			break;
		case 3:
			if(memcmp("h",level,1)==0)
				cfg_para.cfg_ch_zu.ch_zu3_h = atoi(value);
			else
				cfg_para.cfg_ch_zu.ch_zu3_l = atoi(value);
			break;
		case 4:
			if(memcmp("h",level,1)==0)
				cfg_para.cfg_ch_zu.ch_zu4_h = atoi(value);
			else
				cfg_para.cfg_ch_zu.ch_zu4_l = atoi(value);
			break;
		case 5:
			if(memcmp("h",level,1)==0)
				cfg_para.cfg_ch_zu.ch_zu5_h = atoi(value);
			else
				cfg_para.cfg_ch_zu.ch_zu5_l = atoi(value);
			break;
		case 6:
			if(memcmp("h",level,1)==0)
				cfg_para.cfg_ch_zu.ch_zu6_h = atoi(value);
			else
				cfg_para.cfg_ch_zu.ch_zu6_l = atoi(value);
			break;
	}
}

void chCompare(char * name,char * value)
{
	int len=0;
	char channel[5],namedata[8];
	char chnum[3]={0},datanum[3]={0};
	str_split(channel,namedata,name,'_');
	
	strncpy(chnum,channel+2,2);
	len =strlen(namedata);
	
	if(len==3)
		cfg_ch_tmp.ch_num = atoi(value);
	else if(len==4)
	{
		memset(&cfg_data_tmp,0,sizeof(cfg_data_tmp));
		strncpy(cfg_ch_tmp.ch_name,value,7);
	}
	else if(len>=5){
		strncpy(datanum,namedata+4,2);
		switch(atoi(datanum))
		{
			case 0:
				cfg_data_tmp.ch_data1 = strtoul(value,0,0);
				break;
			case 1:
				cfg_data_tmp.ch_data2 = strtoul(value,0,0);
				break;
			case 2:
				cfg_data_tmp.ch_data3 = strtoul(value,0,0);
				break;
			case 3:
				cfg_data_tmp.ch_data4 = strtoul(value,0,0);
				break;
			case 4:
				cfg_data_tmp.ch_data5 = strtoul(value,0,0);
				break;
			case 5:
				cfg_data_tmp.ch_data6 = strtoul(value,0,0);
				break;
			case 6:
				cfg_data_tmp.ch_data7 = strtoul(value,0,0);
				break;
			case 7:
				cfg_data_tmp.ch_data8 = strtoul(value,0,0);
				break;
			
			case 8:
				cfg_data_tmp.ch_data9 = strtoul(value,0,0);
				break;
			case 9:
				cfg_data_tmp.ch_data10 = strtoul(value,0,0);
				break;
			case 10:
				cfg_data_tmp.ch_data11 = strtoul(value,0,0);
				break;
			case 11:
				cfg_data_tmp.ch_data12 = strtoul(value,0,0);
				break;
			case 12:
				cfg_data_tmp.ch_data13 = strtoul(value,0,0);
				break;
			case 13:
				cfg_data_tmp.ch_data14 = strtoul(value,0,0);
				break;
			case 14:
				cfg_data_tmp.ch_data15 = strtoul(value,0,0);
				break;
			case 15:
				cfg_data_tmp.ch_data16 = strtoul(value,0,0);
				break;
		}
	}
	cfg_ch_tmp.cfg_data = cfg_data_tmp;
	chSave(atoi(chnum));
	cfg_para.cfg_ch = cfg_allch_tmp;
}

void chSave(int num)
{
	switch(num)
	{
		case 0:
			cfg_allch_tmp.cfg_ch1 = cfg_ch_tmp;	break;
		case 1:
			cfg_allch_tmp.cfg_ch2 = cfg_ch_tmp;	break;
		case 2:
			cfg_allch_tmp.cfg_ch3 = cfg_ch_tmp;	break;
		case 3:
			cfg_allch_tmp.cfg_ch4 = cfg_ch_tmp;	break;
		case 4:
			cfg_allch_tmp.cfg_ch5 = cfg_ch_tmp;	break;
		case 5:
			cfg_allch_tmp.cfg_ch6 = cfg_ch_tmp;	break;
		case 6:
			cfg_allch_tmp.cfg_ch7 = cfg_ch_tmp;	break;
		case 7:
			cfg_allch_tmp.cfg_ch8 = cfg_ch_tmp;	break;
		case 8:
			cfg_allch_tmp.cfg_ch9 = cfg_ch_tmp;	break;
		case 9:
			cfg_allch_tmp.cfg_ch10 = cfg_ch_tmp;	break;
		
		case 10:
			cfg_allch_tmp.cfg_ch11 = cfg_ch_tmp;	break;
		case 11:
			cfg_allch_tmp.cfg_ch12 = cfg_ch_tmp;	break;
		case 12:
			cfg_allch_tmp.cfg_ch13 = cfg_ch_tmp;	break;
		case 13:
			cfg_allch_tmp.cfg_ch14 = cfg_ch_tmp;	break;
		case 14:
			cfg_allch_tmp.cfg_ch15 = cfg_ch_tmp;	break;
		case 15:
			cfg_allch_tmp.cfg_ch16 = cfg_ch_tmp;	break;
		case 16:
			cfg_allch_tmp.cfg_ch17 = cfg_ch_tmp;	break;
		case 17:
			cfg_allch_tmp.cfg_ch18 = cfg_ch_tmp;	break;
		case 18:
			cfg_allch_tmp.cfg_ch19 = cfg_ch_tmp;	break;
		case 19:
			cfg_allch_tmp.cfg_ch20 = cfg_ch_tmp;	break;
		
		case 20:
			cfg_allch_tmp.cfg_ch21 = cfg_ch_tmp;	break;
		case 21:
			cfg_allch_tmp.cfg_ch22 = cfg_ch_tmp;	break;
		case 22:
			cfg_allch_tmp.cfg_ch23 = cfg_ch_tmp;	break;
		case 23:
			cfg_allch_tmp.cfg_ch24 = cfg_ch_tmp;	break;
		case 24:
			cfg_allch_tmp.cfg_ch25 = cfg_ch_tmp;	break;
		case 25:
			cfg_allch_tmp.cfg_ch26 = cfg_ch_tmp;	break;
		case 26:
			cfg_allch_tmp.cfg_ch27 = cfg_ch_tmp;	break;
		case 27:
			cfg_allch_tmp.cfg_ch28 = cfg_ch_tmp;	break;
		case 28:
			cfg_allch_tmp.cfg_ch29 = cfg_ch_tmp;	break;
		case 29:
			cfg_allch_tmp.cfg_ch30 = cfg_ch_tmp;	break;
		
		case 30:
			cfg_allch_tmp.cfg_ch31 = cfg_ch_tmp;	break;
		case 31:
			cfg_allch_tmp.cfg_ch32 = cfg_ch_tmp;	break;
		case 32:
			cfg_allch_tmp.cfg_ch33 = cfg_ch_tmp;	break;
		case 33:
			cfg_allch_tmp.cfg_ch34 = cfg_ch_tmp;	break;
		case 34:
			cfg_allch_tmp.cfg_ch35 = cfg_ch_tmp;	break;
		case 35:
			cfg_allch_tmp.cfg_ch36 = cfg_ch_tmp;	break;
		case 36:
			cfg_allch_tmp.cfg_ch37 = cfg_ch_tmp;	break;		
	}
}

void str_split(char *left,char *right,char *src, char c)
{
    char *p = src;
    char *q = left;
    char *r = right;
    int flag = 0;

    while(*p != '\0'){
        if(*p==c)
            flag = 1;
        else{
            if(flag)
                *(r++) = *p;
            else
                *(q++) = *p;
        }
        p++;
    }
    *(q++)='\0';
    *(r++)='\0';
}

/*
*********************************************************************************************************
*	函 数 名: CreateNewFile
*	功能说明: 在SD卡创建一个新文件，文件内容填写“www.armfly.com”
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void CreateNewFile(uint8_t pronum,uint8_t picnum,uint8_t flag)
{
	/* 本函数使用的局部变量占用较多，请修改任务栈大小，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	FIL file;
	DIR DirInf;
	uint32_t bw;
	char ProjectFileName[32];
	char FileNamebuf[16]={0};

 	/* 挂载文件系统 */
	result = f_mount(&fs, "0:", 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
//		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
		return;
	}

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "0:/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
//		printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
		return;
	}

	switch(flag)
	{
		case 1:
			result = f_open(&file, "0:/proname.txt", FA_CREATE_ALWAYS | FA_WRITE);
//			result = f_write(&file, &cfgname, sizeof(cfgname), &bw);
			result = f_write(&file, &currentPro, 1, &bw);
			break;
		case 2:
			flag_correct = 0;
			result = f_open(&file, "0:/jiaozhun.txt", FA_CREATE_ALWAYS | FA_WRITE);
			result = f_write(&file, &correctbuf, sizeof(correctbuf), &bw);
			break;
		case 0:
			strncpy(FileNamebuf,cfgname[pronum],strlen(cfgname[pronum]));
			sprintf(ProjectFileName, "0:/%s%02d.txt",FileNamebuf, picnum);
			result = f_open(&file, ProjectFileName, FA_CREATE_ALWAYS | FA_WRITE);
			result = f_write(&file, &cfg_para, sizeof(cfg_para), &bw);
			break;
	}
	if (result == FR_OK)
	{
//		printf("%s 文件写入成功\r\n",ProjectFileName);
	}
	else
	{
//		printf("%s 文件写入失败  (%s)\r\n", ProjectFileName, FR_Table[result]);
		return;
	}

	f_close(&file);

	/* 卸载文件系统 */
	 f_mount(NULL, "0:", 0);
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
//		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
		return;
	}

	/* 打开根文件夹 */
	sprintf(path, "%s/", _ucVolume);
	result = f_opendir(&DirInf, path); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK)
	{
//		printf("打开根目录失败  (%s)\r\n", FR_Table[result]);
		return;
	}

	/* 打开文件 */
	sprintf(pathname, "%s%s.txt", path , _fileName);
	result = f_open(&file, pathname, FA_OPEN_EXISTING | FA_READ);
	if (result !=  FR_OK)
	{
//		printf("Don't Find File : %s\r\n",pathname);
		return;
	}

	/* 读取文件 */
	result = f_read(&file, buf, len, &bw);
	if (bw > 0)
	{
//		buf[bw] = 0;
//		*((char*)buf+bw) = 0;
//		printf("\r\narmfly.txt 文件内容 : \r\n%s\r\n", buf);
//		printf("\r\n%s 文件读取成功 \r\n",pathname);
	}
	else
	{
//		printf("\r\n%s 文件读取失败\r\n",pathname);
		return;
	}

	/* 关闭文件*/
	f_close(&file);

	/* 卸载文件系统 */
	f_mount(NULL, path, 0);
}

void DeleteDirFile(char *_ucVolume,char *_fileName)
{
	FRESULT result;
	FATFS fs;
//	char FileName[13];
	uint32_t cnt = 0;
	DIR DirInf;
	FILINFO FileInf;
	char lfname[32];
	char path[32];

 	/* 挂载文件系统 */
	result = f_mount(&fs, _ucVolume, 0);			/* Mount a logical drive */
	if (result != FR_OK)
	{
//		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
		return;
	}
	
	/* 打开根文件夹 */
	sprintf(path, "%s/", _ucVolume);	 /* 1: 表示盘符 */
	result = f_opendir(&DirInf, path);
	if (result != FR_OK)
	{
//		printf("打开根目录失败 (%d)\r\n", result);
		return;
	}

	/* 读取当前文件夹下的文件和目录 */
	FileInf.lfname = lfname;
	FileInf.lfsize = 32;
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
//			if(strstr((char *)FileInf.fname,".txt"))
			if(strstr((char *)FileInf.fname,_fileName))
			{				
				result = f_unlink(FileInf.fname);
				if (result == FR_OK)
				{
//					printf("删除文件%s成功\r\n", (char *)FileInf.fname);
				}
			}
			if(strstr((char *)FileInf.lfname,_fileName))
			{				
				result = f_unlink(FileInf.lfname);
				if (result == FR_OK)
				{
//					printf("删除文件%s成功\r\n", (char *)FileInf.lfname);
				}
			}
		}
	}

	/* 卸载文件系统 */
//	f_mount(NULL, _ucVolume, 0);
}


void WriteChData(FIL *fileU,char chnum,char datanum,unsigned int datatmp)
{	
	FRESULT resultU;
	uint32_t bw;
	char bufN[16];
	char buf[16];
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	sprintf(buf,"ch%d_data%d",chnum,datanum);
	resultU = f_write(fileU, buf, strlen(buf), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	sprintf(bufN,"%u",datatmp);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;	
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;
}

void WriteChAllData(FIL *fileU,char num)
{	
	char i;
	unsigned int databuf=0;
	
	for(i=0;i<16;i++)
	{
		switch(i)
		{
			case 0:
				databuf = cfg_ch_tmp.cfg_data.ch_data1;
				break;
			case 1:
				databuf = cfg_ch_tmp.cfg_data.ch_data2;
				break;
			case 2:
				databuf = cfg_ch_tmp.cfg_data.ch_data3;
				break;
			case 3:
				databuf = cfg_ch_tmp.cfg_data.ch_data4;
				break;
			case 4:
				databuf = cfg_ch_tmp.cfg_data.ch_data5;
				break;
			case 5:
				databuf = cfg_ch_tmp.cfg_data.ch_data6;
				break;
			case 6:
				databuf = cfg_ch_tmp.cfg_data.ch_data7;
				break;
			case 7:
				databuf = cfg_ch_tmp.cfg_data.ch_data8;
				break;
			
			case 8:
				databuf = cfg_ch_tmp.cfg_data.ch_data9;
				break;
			case 9:
				databuf = cfg_ch_tmp.cfg_data.ch_data10;
				break;
			case 10:
				databuf = cfg_ch_tmp.cfg_data.ch_data11;
				break;
			case 11:
				databuf = cfg_ch_tmp.cfg_data.ch_data12;
				break;
			case 12:
				databuf = cfg_ch_tmp.cfg_data.ch_data13;
				break;
			case 13:
				databuf = cfg_ch_tmp.cfg_data.ch_data14;
				break;
			case 14:
				databuf = cfg_ch_tmp.cfg_data.ch_data15;
				break;
			case 15:
				databuf = cfg_ch_tmp.cfg_data.ch_data16;
				break;
		}
		if(databuf==0)
			break;
		WriteChData(fileU,num,i,databuf);
	}
}

void WriteCh(FIL *fileU,char num)
{	
	FRESULT resultU;
	uint32_t bw;
	char bufN[16];
	char buf[16];
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	sprintf(buf,"ch%d_name",num);
	resultU = f_write(fileU, buf, strlen(buf), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, cfg_ch_tmp.ch_name, strlen(cfg_ch_tmp.ch_name), &bw);
//	if (resultU != FR_OK)
//		return;	
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	sprintf(buf,"ch%d_num",num);
	resultU = f_write(fileU, buf, strlen(buf), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	sprintf(bufN,"%d",cfg_ch_tmp.ch_num);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;	
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;
	
	WriteChAllData(fileU,num);
}

void WriteAllCh(FIL *fileU,CONFIG_PARA *picbuf)
{
	char i;
	
	for(i=0;i<37;i++)
	{
		switch(i)
		{
			case 0:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch1;
				break;
			case 1:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch2;
				break;
			case 2:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch3;
				break;
			case 3:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch4;
				break;
			case 4:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch5;
				break;
			case 5:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch6;
				break;
			case 6:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch7;
				break;
			case 7:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch8;
				break;
			case 8:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch9;
				break;
			case 9:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch10;
				break;
			
			case 10:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch11;
				break;
			case 11:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch12;
				break;
			case 12:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch13;
				break;
			case 13:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch14;
				break;
			case 14:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch15;
				break;
			case 15:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch16;
				break;
			case 16:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch17;
				break;
			case 17:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch18;
				break;
			case 18:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch19;
				break;
			case 19:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch20;
				break;
			
			case 20:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch21;
				break;
			case 21:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch22;
				break;
			case 22:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch23;
				break;
			case 23:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch24;
				break;
			case 24:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch25;
				break;
			case 25:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch26;
				break;
			case 26:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch27;
				break;
			case 27:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch28;
				break;
			case 28:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch29;
				break;
			case 29:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch30;
				break;
			
			
			case 30:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch31;
				break;
			case 31:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch32;
				break;
			case 32:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch33;
				break;
			case 33:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch34;
				break;
			case 34:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch35;
				break;
			case 35:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch36;
				break;
			case 36:
				cfg_ch_tmp = picbuf->cfg_ch.cfg_ch37;
				break;
		}
		WriteCh(fileU,i);
	}
}

void WtriePicInf(FIL *fileU,CONFIG_PARA *picbuf)
{
	FRESULT resultU;
	uint32_t bw;
	char bufN[8];
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "picmaxn", 7, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.picMaxN);
	sprintf(bufN,"%d",picbuf->picMaxN);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "picname", 7, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	resultU = f_write(fileU, cfg_para.picName, strlen(cfg_para.picName), &bw);
	resultU = f_write(fileU, picbuf->picName, strlen(picbuf->picName), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "picnum", 6, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.picNum);
	sprintf(bufN,"%d",picbuf->picNum);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "voltage", 7, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.voltage);
	sprintf(bufN,"%d",picbuf->voltage);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "current", 7, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.current);
	sprintf(bufN,"%d",picbuf->current);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;
}
void WriteZu(FIL *fileU,CONFIG_PARA *picbuf)
{
	FRESULT resultU;
	uint32_t bw;
	char bufN[8];
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu1_h", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu1_h);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu1_h);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu1_l", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu1_l);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu1_l);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu2_h", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu2_h);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu2_h);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu2_l", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu2_l);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu2_l);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu3_h", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu3_h);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu3_h);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu3_l", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu3_l);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu3_l);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu4_h", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu4_h);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu4_h);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu4_l", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu4_l);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu4_l);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu5_h", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu5_h);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu5_h);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu5_l", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu5_l);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu5_l);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu6_h", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu6_h);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu6_h);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "ch_zu6_l", 8, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.cfg_ch_zu.ch_zu6_l);
	sprintf(bufN,"%d",picbuf->cfg_ch_zu.ch_zu6_l);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;	
}

void Write2U(FIL *fileU,CONFIG_PARA *picbuf)
{
	FRESULT resultU;
	uint32_t bw;
	char bufN[8];
	
	WtriePicInf(fileU,picbuf);
	WriteZu(fileU,picbuf);
	
	resultU = f_write(fileU, "#", 1, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "total_time", 10, &bw);
//	if (resultU != FR_OK)
//		return;
	resultU = f_write(fileU, "=", 1, &bw);
//	if (resultU != FR_OK)
//		return;
//	sprintf(bufN,"%d",cfg_para.totalTime);
	sprintf(bufN,"%d",picbuf->totalTime);
	resultU = f_write(fileU, bufN, strlen(bufN), &bw);
//	if (resultU != FR_OK)
//		return;	
	resultU = f_write(fileU, ";\r\n", 3, &bw);
//	if (resultU != FR_OK)
//		return;
	
	WriteAllCh(fileU,picbuf);
	
	resultU = f_write(fileU, "END\r\n", 5, &bw);
//	if (resultU != FR_OK)
//		return;
}

void WriteCfg2U(CONFIG_PARA *picbuf,char flag,char *filename)
{
	FRESULT resultU;
	FATFS fsU;
	FIL fileU;
	DIR DirInfU;
	char path[32];
		
	resultU = f_mount(&fsU, "2:", 0);			/* Mount a logical drive */
	if (resultU != FR_OK)
	{
//		printf("挂载文件系统失败 (%d)\r\n", resultU);
		return;
	}
	resultU = f_opendir(&DirInfU, "2:/");
	if (resultU != FR_OK)
	{
//		printf("打开根目录失败 (%d)\r\n", resultU);
		return;
	}
//	sprintf(path, "2:/ceshi.txt");
	sprintf(path, "2:/%s.txt",filename);
	if(flag==0)
		resultU = f_open(&fileU, path, FA_CREATE_ALWAYS | FA_WRITE);
	else
		resultU = f_open(&fileU, path, FA_OPEN_EXISTING | FA_WRITE);
	if (resultU !=  FR_OK)
	{
//		printf("Don't Find File : %s\r\n", path);
		return;
	}
	
	f_lseek(&fileU,fileU.fsize);
	Write2U(&fileU,picbuf);
	 
	f_close(&fileU);
	f_mount(NULL,"2:", 0);
}

void correctPara(void)
{
	int i=0,j=0,k=0,m;
	char tmp[16];
	
	for(i;correctbuf[i] != '\0';i++)
	{
		if((correctbuf[i] == '\r') || (correctbuf[i] == '\n') || (correctbuf[i] == ' ') || (correctbuf[i] == ','))
		{
			j = 0;
			memset(tmp,0,sizeof(tmp));
		} else {
			tmp[j++] = correctbuf[i];
			if((correctbuf[i+1] == '\r') || (correctbuf[i+1] == ',') || (correctbuf[i+1] == ' '))
				corretPara[k++] = atof(tmp);
		}
	}
}

void OP_JiaoZhun(void)
{
	if(flag_correct)
	{	
		ReadFileData(FS_VOLUME_USB,"jiaozhun",correctbuf,sizeof(correctbuf));
		CreateNewFile(0,0,2);
	}else{
		ReadFileData(FS_VOLUME_SD,"jiaozhun",correctbuf,sizeof(correctbuf));
	}
	correctPara();
}

void addPro(char len,char proname[][16])
{
	int i,j,k;
	
	for(i=0;proname[i][0] != '\0';i++)
	{
		if(len == 0){
			strncpy(cfgname[0],proname[i],strlen(proname[i])-2);
			len = 1;
		}else{
			for(j=0;j<len;j++)
			{
				if(!strncmp(cfgname[j],proname[i],strlen(proname[i])-2))
					break;
				else if(j>=len-1)
					strncpy(cfgname[len++],proname[i],strlen(proname[i])-2);
			}
		}
	}
}

uint8_t File_Init(void)
{ 
	usbh_OpenMassStorage();
	
	f_mount(NULL,"2:", 0);
	result = f_mount(&fs_usb, "2:", 0);
	if (result != FR_OK)
	{
		f_mount(NULL,"2:", 0);
		return 0;
	}
	
	result = f_opendir(&DirInf_usb, "2:/"); 
	if (result != FR_OK)
	{
//		printf("open USB dir error!\r\n");
		f_mount(NULL,"2:", 0);
		return 0;
	}
	else
	{
		f_mount(NULL,"2:", 0);
		return 1;
	}
}
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/



