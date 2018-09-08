
#ifndef _DEMO_FATFS_H
#define _DEMO_FATFS_H

#include "stdint.h"
#include "diskio.h"

struct FPIC{ 
	char  FPIC_NAMEE[10];      //picture name user define
	int FPIC_TIMEE;  //????(s)                           
	int FPIC_R1_VH; //R1????                             
	int FPIC_R1_VL; //R1????                             
	int FPIC_G1_VH; //G1????                             
	int FPIC_G1_VL; //G1????                             
	int FPIC_B1_VH; //B1????                             
	int FPIC_B1_VL; //B1????                             
	int FPIC_R2_VH; //R2????                             
	int FPIC_R2_VL; //R2????                             
	int FPIC_G2_VH; //G2????                             
	int FPIC_G2_VL; //G2????                             
	int FPIC_B2_VH; //B2????                             
	int FPIC_B2_VL; //B2????   

	int FPIC_R3_VH; //R1????                          
	int FPIC_R3_VL; //R1????
	int FPIC_G3_VH; //G1????
	int FPIC_G3_VL; //G1????
	int FPIC_B3_VH; //B1????
	int FPIC_B3_VL; //B1????
	int FPIC_R4_VH; //R2????
	int FPIC_R4_VL; //R2????
	int FPIC_G4_VH; //G2????
	int FPIC_G4_VL; //G2????
	int FPIC_B4_VH; //B2????
	int FPIC_B4_VL; //B2????     
};

typedef struct {
	char Pro_Maxnum;        
	char Project_Name[15];
	char Project_Num;
	char CFG_Part1[10];
	int VBL;                        //??????(V)
	int IBL;                      //??????(mA)
	int IBL_LIMIT;                  //??????(mA)   
		
	char CFG_Part2[10]; 
	int VGL1;
	int VGH1;        
	int VGG1;
	int VSW1;
	int VCOM1;
	int VCOM1_L;
	int VBW1;
	int VFW1;
	int VTIMING_H1;
	int VTIMING_L1;
	
	//struct STV stv[4];
	struct FPIC  fpic[17];
	char FPGA1_PIC_NUM;
	char FPGA1_STV_NUM;
//	char A[88];	
} BL_FPGA1;

struct STV{
	int  STV_TTTTT1;                          //(us)                                        
	int  STV_TTTTT2;                          //(us)                                          
	int  STV_TTTTT3;                          //(us)                                          
	int  STV_TTTTT4;                          //(us)                                          
	int  STV_TTTTT5;		                       //(us)                                          
	int  STV_NNNNNN;				                       //                                              
																						//*************STV1_CK1 TIMING CONFIG*************
	int  STV_CK1_T1;	                    //(us)                                        
	int  STV_CK1_T2;	                    //(us)                     
	int  STV_CK1_T3;	                    //(us)                     
	int  STV_CK1_T4;	                    //(us)                     
	int  STV_CK1_T5;	                    //(us)                     
	int  STV_CK1_NN;			                    //                         
																									 //*************STV1_CK2 TIMING CONFIG*************
	int  STV_CK2_T1;                      //(us)                      
	int  STV_CK2_T2;                      //(us)                      
	int  STV_CK2_T3;                      //(us)                      
	int  STV_CK2_T4;                      //(us)                      
	int  STV_CK2_T5;	                     //(us)                     
	int  STV_CK2_NN;			                     //                          
																																						//*************STV1_RST TIMING CONFIG*************;
	int  STV_RST_T1;                       //(us)                      
	int  STV_RST_T2;                       //(us)                                       
	int  STV_RST_T3;                       //(us)                                       
	int  STV_RST_T4;                       //(us)                                       
	int  STV_RST_T5;                      	//(us)                                      
	int  STV_RST_NN;                          
}; 

typedef struct { 
	 struct STV stv[20];
	 char C[128];
 } FPGA1_STV; 

 struct RGB_N{
	char TR1H_N;
	char TR1L_N;
	char TR2H_N;
	char TR2L_N;
	char TR3H_N;
	char TR3L_N;
	char TR4H_N;
	char TR4L_N;   
						 
	char TG1H_N;
	char TG1L_N;
	char TG2H_N;
	char TG2L_N;
	char TG3H_N;
	char TG3L_N;
	char TG4H_N;
	char TG4L_N;  
						 
	char TB1H_N;
	char TB1L_N;
	char TB2H_N;
	char TB2L_N;
	char TB3H_N;
	char TB3L_N;
	char TB4H_N;
	char TB4L_N;     
};
 
typedef struct {
	struct RGB_N rgb_n[15];
	
	char Tvcom1H_N;	
	char Tvcom1L_N;	
	char Tvcom2H_N;	
	char Tvcom2L_N;	
	char Tvcom3H_N;	
	char Tvcom3L_N;	
	char Tvcom4H_N;	
	char Tvcom4L_N;

	char Tvcom5H_N;	
	char Tvcom5L_N;	
	char Tvcom6H_N;	
	char Tvcom6L_N;	
	char Tvcom7H_N;	
	char Tvcom7L_N;	
	char Tvcom8H_N;	
	char Tvcom8L_N;
								
	char Tvcom9H_N;	
	char Tvcom9L_N;	
	char Tvcom10H_N;	
	char Tvcom10L_N;	
	char Tvcom11H_N;	
	char Tvcom11L_N;	
	char Tvcom12H_N;	
	char Tvcom12L_N;

	char Tvcom13H_N;
	char Tvcom13L_N;
	char Tvcom14H_N;
	char Tvcom14L_N;
	char Tvcom15H_N;
	char Tvcom15L_N;
//	char d[1658];	
} FPGA1_RGB_N;
 

/* 供外部调用的函数声明 */
void DemoFatFS(void);
void ReadFileData(char *_ucVolume,char *_fileName,void *buf,uint16_t len);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
