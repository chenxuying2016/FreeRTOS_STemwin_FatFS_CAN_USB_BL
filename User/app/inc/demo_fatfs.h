
#ifndef _DEMO_FATFS_H
#define _DEMO_FATFS_H

#include "stdint.h"
#include "diskio.h"
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>
#include "ff.h"

struct _CONFIG_CH_ZU{
	short ch_zu1_h;
	short ch_zu1_l;
	short ch_zu2_h;
	short ch_zu2_l;
	short ch_zu3_h;
	short ch_zu3_l;
	short ch_zu4_h;
	short ch_zu4_l;
	short ch_zu5_h;
	short ch_zu5_l;
	short ch_zu6_h;
	short ch_zu6_l;
};

struct _CONFIG_DATA{
	int		ch_data1;
	int		ch_data2;
	int		ch_data3;
	int		ch_data4;
	int		ch_data5;
	int		ch_data6;
	int		ch_data7;
	int		ch_data8;
	int		ch_data9;
	int		ch_data10;
	int		ch_data11;
	int		ch_data12;
	int		ch_data13;
	int		ch_data14;
	int		ch_data15;
	int		ch_data16;
};

struct _CONFIG_CHANNEL_{	
	char	ch_name[15];
	char	ch_num;
	struct _CONFIG_DATA cfg_data;
};

struct _CONFIG_CHANNEL{
	struct _CONFIG_CHANNEL_ cfg_ch1;
	struct _CONFIG_CHANNEL_ cfg_ch2;
	struct _CONFIG_CHANNEL_ cfg_ch3;
	struct _CONFIG_CHANNEL_ cfg_ch4;
	struct _CONFIG_CHANNEL_ cfg_ch5;
	struct _CONFIG_CHANNEL_ cfg_ch6;
	struct _CONFIG_CHANNEL_ cfg_ch7;
	struct _CONFIG_CHANNEL_ cfg_ch8;
	struct _CONFIG_CHANNEL_ cfg_ch9;
	struct _CONFIG_CHANNEL_ cfg_ch10;
	struct _CONFIG_CHANNEL_ cfg_ch11;
	struct _CONFIG_CHANNEL_ cfg_ch12;
	struct _CONFIG_CHANNEL_ cfg_ch13;
	struct _CONFIG_CHANNEL_ cfg_ch14;
	struct _CONFIG_CHANNEL_ cfg_ch15;
	struct _CONFIG_CHANNEL_ cfg_ch16;
	struct _CONFIG_CHANNEL_ cfg_ch17;
	struct _CONFIG_CHANNEL_ cfg_ch18;
	struct _CONFIG_CHANNEL_ cfg_ch19;
	struct _CONFIG_CHANNEL_ cfg_ch20;
	struct _CONFIG_CHANNEL_ cfg_ch21;
	struct _CONFIG_CHANNEL_ cfg_ch22;
	struct _CONFIG_CHANNEL_ cfg_ch23;
	struct _CONFIG_CHANNEL_ cfg_ch24;
	struct _CONFIG_CHANNEL_ cfg_ch25;
	struct _CONFIG_CHANNEL_ cfg_ch26;
	struct _CONFIG_CHANNEL_ cfg_ch27;
	struct _CONFIG_CHANNEL_ cfg_ch28;
	struct _CONFIG_CHANNEL_ cfg_ch29;
	struct _CONFIG_CHANNEL_ cfg_ch30;
	struct _CONFIG_CHANNEL_ cfg_ch31;
	struct _CONFIG_CHANNEL_ cfg_ch32;
	struct _CONFIG_CHANNEL_ cfg_ch33;
	struct _CONFIG_CHANNEL_ cfg_ch34;
	struct _CONFIG_CHANNEL_ cfg_ch35;
	struct _CONFIG_CHANNEL_ cfg_ch36;
	struct _CONFIG_CHANNEL_ cfg_ch37;
};

typedef struct _CONFIG_PARA{
	unsigned char	picMaxN;
	char	picName[14];
	char	picNum;
	int		voltage;
	int		current;
	struct _CONFIG_CH_ZU cfg_ch_zu;
	int		totalTime;
	struct _CONFIG_CHANNEL cfg_ch;
}CONFIG_PARA;

typedef struct _CONFIG_PIC{
	unsigned char	picMaxN;
	char	picName[14];
	char	picNum;
}CONFIG_PIC;



/* 供外部调用的函数声明 */
char DemoFatFS(void);
void analysisCfgFile(void);
void strCompare( char *title, char* value,int len);
void zuCompare(char * name,char * num);
void chCompare(char * name,char * value);
void chSave(int num);
void str_split(char *left,char *right,char *src, char c);
void ReadFileData(char *_ucVolume,char *_fileName,void *buf,uint16_t len);
int ViewRootDir(char *_ucVolume,char buf[][16]);
void CreateNewFile(uint8_t pronum,uint8_t picnum,uint8_t flag);
void DeleteDirFile(char *_ucVolume,char *_fileName);

void WriteChData(FIL *fileU,char chnum,char datanum,unsigned int datatmp);
void WriteChAllData(FIL *fileU,char num);
void WriteCh(FIL *fileU,char num);
void WriteAllCh(FIL *fileU,CONFIG_PARA *picbuf);
void WtriePicInf(FIL *fileU,CONFIG_PARA *picbuf);
void WriteZu(FIL *fileU,CONFIG_PARA *picbuf);
void Write2U(FIL *fileU,CONFIG_PARA *picbuf);
void WriteCfg2U(CONFIG_PARA *picbuf,char flag,char *filename);
void correctPara(void);
void addPro(char len,char proname[][16]);
void readPro(char maxProNum);
uint8_t File_Init(void);
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
