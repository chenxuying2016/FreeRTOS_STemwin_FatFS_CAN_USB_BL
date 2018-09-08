#include "ARM_FPGA.h"

/****************uart************/
//extern uint8_t aRxBuffer[RXBUFFERSIZE];
//uint8_t project_number=0,BL_I=0,FPGA_U=0,picture_num=0,display_U_flag=0;
extern uint8_t FPGA1_BL[2048];

/****************uart************/

//**************************存下发给FPGA的初始化参数的数组***********//
//extern uint8_t Rnand[2048];   //FPGA1
//extern uint8_t FPGA1_STV1[2048];   //s1tv-s5tv
//extern uint8_t FPGA1_STV2[2048];   //s6tv-s10tv
//extern uint8_t FPGA1_STV3[2048];   //s11tv-s15tv
//extern uint8_t FPGA1_RGBN[2048];   //TRGB1-15
extern BL_FPGA1 Rnand ;
extern FPGA1_STV fpga1_STV1;   //1到5套时序参数
extern FPGA1_STV fpga1_STV2;   //第六到第10套时序参数
extern FPGA1_STV fpga1_STV3;   //第11套到第15套时序参数
extern FPGA1_RGB_N fpga1_rgb_n;
//**************************存下发给FPGA的初始化参数的数组***********//
//extern BL_FPGA1 BL_fpga1 ;
 
typedef struct{
  uint32_t byte3 : 8;
	uint32_t byte2 : 8;
	uint32_t byte1 : 8;
	uint32_t ID    : 5;
	uint32_t RES   : 3;
}ID_Define;
                                       

void ARM_FPGA1_uart(void)                                       //FPGA1 parameter
{
	int a,b,c,d;
	unsigned char data[8]={0};
  unsigned int k=0;
  unsigned int y=0;
  
//	NAND_Reads(0x41,Rnand);  
 
//  CAN_SendMessage(0x020802ff,data);           //end play     FPGA1                                                                                                                        
//	CAN_SendMessage(0x030802ff,data);           //end play       FPGA2                                                                                                                      
//	data[0]=0;                                                                                                                                                                              
//	CAN_SendMessage(0x0103ffff,data);           //disable BL  
//  Delay_Nms(1);
  a=((BL_FPGA1*)(&FPGA1_BL))->VBL;                   //BL parameter
  b=((BL_FPGA1*)(&FPGA1_BL))->IBL;
  c=((BL_FPGA1*)(&FPGA1_BL))->IBL_LIMIT;
  data[0]=(unsigned char)((a&(0x0000ff00))>>8);                           //BL parameter
  data[1]=(unsigned char)((a&(0x000000ff)));
  data[2]=(unsigned char)((b&(0x0000ff00))>>8); 
  data[3]=(unsigned char)((b&(0x000000ff)));
  data[4]=(unsigned char)((c&(0x0000ff00))>>8); 
  data[5]=(unsigned char)((c&(0x000000ff)));
  data[6]=0xff;
  data[7]=0xff;
	CAN_SendMessage(0x0101ffff,data);        //BL parameter

	data[0]=0xff;                           //start commmand of FPGA1 INIT parameter
  data[1]=0xff;
  data[2]=0xff;
  data[3]=0xff;
  data[4]=0xff;
  data[5]=0xff;
  data[6]=0xff;
  data[7]=0xff;
	CAN_SendMessage(0x020401ff,data);
	
	
	a=((BL_FPGA1*)(&FPGA1_BL))->VGL1;                            //FPGA1 U	parameter 
  b=((BL_FPGA1*)(&FPGA1_BL))->VGH1;
  c=((BL_FPGA1*)(&FPGA1_BL))->VGG1;
  d=((BL_FPGA1*)(&FPGA1_BL))->VSW1;
  data[0]=(unsigned char)((a&(0x0000ff00))>>8);                        
  data[1]=(unsigned char)((a&(0x000000ff)));
  data[2]=(unsigned char)((b&(0x0000ff00))>>8); 
  data[3]=(unsigned char)((b&(0x000000ff)));
  data[4]=(unsigned char)((c&(0x0000ff00))>>8); 
  data[5]=(unsigned char)((c&(0x000000ff)));
  data[6]=(unsigned char)((d&(0x0000ff00))>>8);
  data[7]=(unsigned char)((d&(0x000000ff)));
	CAN_SendMessage(0x020101ff,data);
	
	a=((BL_FPGA1*)(&FPGA1_BL))->VCOM1;                            //FPGA1 U	parameter 
  b=((BL_FPGA1*)(&FPGA1_BL))->VBW1;
  c=((BL_FPGA1*)(&FPGA1_BL))->VFW1;
  d=((BL_FPGA1*)(&FPGA1_BL))->VTIMING_H1;
  data[0]=(unsigned char)((a&(0x0000ff00))>>8);                        
  data[1]=(unsigned char)((a&(0x000000ff)));
  data[2]=(unsigned char)((b&(0x0000ff00))>>8); 
  data[3]=(unsigned char)((b&(0x000000ff)));
  data[4]=(unsigned char)((c&(0x0000ff00))>>8); 
  data[5]=(unsigned char)((c&(0x000000ff)));
  data[6]=(unsigned char)((d&(0x0000ff00))>>8);
  data[7]=(unsigned char)((d&(0x000000ff)));
	CAN_SendMessage(0x020102ff,data);
	
	a=((BL_FPGA1*)(&FPGA1_BL))->VTIMING_L1;                            //FPGA1 U	parameter 
  data[0]=(unsigned char)((a&(0x0000ff00))>>8);                        
  data[1]=(unsigned char)((a&(0x000000ff)));
  data[2]=0xff; 
  data[3]=0xff;
  data[4]=0xff;
  data[5]=0xff;
  data[6]=0xff;
  data[7]=0xff;
	CAN_SendMessage(0x020103ff,data);
	
	

	
	for(k=0;k<20;k++)                                         //FPGA1   picture  parameter
	{
	  a=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_R1_VH;                            //FPGA1 	parameter 
		b=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_R1_VL; 
		c=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_G1_VH;                            //FPGA1 	parameter 
		d=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_G1_VL; 
		data[0]=(unsigned char)((a&(0x0000ff00))>>8);                       
		data[1]=(unsigned char)((a&(0x000000ff)));
		data[2]=(unsigned char)((b&(0x0000ff00))>>8);
		data[3]=(unsigned char)((b&(0x000000ff)));
		data[4]=(unsigned char)((c&(0x0000ff00))>>8);
		data[5]=(unsigned char)((c&(0x000000ff)));
		data[6]=(unsigned char)((d&(0x0000ff00))>>8);
		data[7]=(unsigned char)((d&(0x000000ff)));
		y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x02030001|y,data);
		
		a=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_B1_VH;                            //FPGA1 	parameter 
		b=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_B1_VL; 
		c=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_R2_VH;                            //FPGA1 	parameter 
		d=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_R2_VL; 
		data[0]=(unsigned char)((a&(0x0000ff00))>>8);                       
		data[1]=(unsigned char)((a&(0x000000ff)));
		data[2]=(unsigned char)((b&(0x0000ff00))>>8);
		data[3]=(unsigned char)((b&(0x000000ff)));
		data[4]=(unsigned char)((c&(0x0000ff00))>>8);
		data[5]=(unsigned char)((c&(0x000000ff)));
		data[6]=(unsigned char)((d&(0x0000ff00))>>8);
		data[7]=(unsigned char)((d&(0x000000ff)));
		y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x02030002|y,data);
		
		a=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_G2_VH;                            //FPGA1 	parameter 
		b=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_G2_VL; 
		c=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_B2_VH;                            //FPGA1 	parameter 
		d=(((BL_FPGA1*)(&FPGA1_BL))->fpic[k]).FPIC_B2_VL; 
		data[0]=(unsigned char)((a&(0x0000ff00))>>8);                       
		data[1]=(unsigned char)((a&(0x000000ff)));
		data[2]=(unsigned char)((b&(0x0000ff00))>>8);
		data[3]=(unsigned char)((b&(0x000000ff)));
		data[4]=(unsigned char)((c&(0x0000ff00))>>8);
		data[5]=(unsigned char)((c&(0x000000ff)));
		data[6]=(unsigned char)((d&(0x0000ff00))>>8);
		data[7]=(unsigned char)((d&(0x000000ff)));
		y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x02030003|y,data);
		
  }
	
	
	
	                                                               //add send code of stv parameter to FPGA 
	
	
	data[0]=0xff;                           //end commmand of FPGA1 INIT parameter
  data[1]=0xff;
  data[2]=0xff;
  data[3]=0xff;
  data[4]=0xff;
  data[5]=0xff;
  data[6]=0xff;
  data[7]=0xff;
	CAN_SendMessage(0x020402ff,data);
	
	//ARM_FPGA2();
	
}










void ARM_FPGA1(void)                                       //FPGA1 parameter
{
	int a,b,c,d;
	unsigned char data[8]={0};
  unsigned char f1=0x01,f2=0x02,f3=0x03,f4=0x04,f5=0x05,f6=0x06,f7=0x07,f8=0x08,f9=0x09,f10=0x0a,f11=0x0b,f12=0x0c;
  unsigned short int k=0,kk=0;
  unsigned int y=0;
  unsigned short int t=0,t1=0;
  
//	NAND_Reads(0x41,Rnand);  
 
//  CAN_SendMessage(0x020802ff,data);           //end play     FPGA1                                                                                                                        
//	CAN_SendMessage(0x030802ff,data);           //end play       FPGA2                                                                                                                      
//	data[0]=0;                                                                                                                                                                              
//	CAN_SendMessage(0x0103ffff,data);           //disable BL  
//  Delay_Nms(1);
	{
  a = Rnand.VBL;//((BL_FPGA1*)(&Rnand))->VBL;                   //BL parameter
  b = Rnand.IBL;//((BL_FPGA1*)(&Rnand))->IBL;
  c = Rnand.IBL_LIMIT;//((BL_FPGA1*)(&Rnand))->IBL_LIMIT;
  data[0]=(unsigned char)((a&(0x0000ff00))>>8);                           //BL parameter
  data[1]=(unsigned char)((a&(0x000000ff)));
  data[2]=(unsigned char)((b&(0x0000ff00))>>8); 
  data[3]=(unsigned char)((b&(0x000000ff)));
  data[4]=(unsigned char)((c&(0x0000ff00))>>8); 
  data[5]=(unsigned char)((c&(0x000000ff)));
  data[6]=0xff;
  data[7]=0xff;
	CAN_SendMessage(0x0101ffff,data);        //BL parameter

	data[0]=0xff;                           //start commmand of FPGA1 INIT parameter
  data[1]=0xff;
  data[2]=0xff;
  data[3]=0xff;
  data[4]=0xff;
  data[5]=0xff;
  data[6]=0xff;
  data[7]=0xff;
	CAN_SendMessage(0x020401ff,data);		

	a = Rnand.FPGA1_PIC_NUM;//((BL_FPGA1*)(&Rnand))->FPGA1_PIC_NUM;                   //picture number
  b = Rnand.FPGA1_STV_NUM;//((BL_FPGA1*)(&Rnand))->FPGA1_STV_NUM;                   //stv number
  c = Rnand.Project_Num;//((BL_FPGA1*)(&Rnand))->Project_Num;   
	data[0]=(unsigned char)a;                           //
  data[1]=(unsigned char)b;
  data[2]=(unsigned char)c;
  data[3]=0xff;
  data[4]=0xff;
  data[5]=0xff;
  data[6]=0xff;
  data[7]=0xff;
	CAN_SendMessage(0x020a01ff,data);                    //picture and stv number
	
	a=Rnand.VGL1;//((BL_FPGA1*)(&Rnand))->VGL1;                            //FPGA1 U	parameter 
  b=Rnand.VGH1;//((BL_FPGA1*)(&Rnand))->VGH1;
  c=Rnand.VGG1;//((BL_FPGA1*)(&Rnand))->VGG1;
  d=Rnand.VSW1;//((BL_FPGA1*)(&Rnand))->VSW1;
  data[0]=(unsigned char)((a&(0x0000ff00))>>8);                        
  data[1]=(unsigned char)((a&(0x000000ff)));
  data[2]=(unsigned char)((b&(0x0000ff00))>>8); 
  data[3]=(unsigned char)((b&(0x000000ff)));
  data[4]=(unsigned char)((c&(0x0000ff00))>>8); 
  data[5]=(unsigned char)((c&(0x000000ff)));
  data[6]=(unsigned char)((d&(0x0000ff00))>>8);
  data[7]=(unsigned char)((d&(0x000000ff)));
	CAN_SendMessage(0x020101ff,data);
	
	a=Rnand.VCOM1;//((BL_FPGA1*)(&Rnand))->VCOM1;                            //FPGA1 U	parameter 
  b=Rnand.VBW1;//((BL_FPGA1*)(&Rnand))->VBW1;
  c=Rnand.VFW1;//((BL_FPGA1*)(&Rnand))->VFW1;
  d=Rnand.VTIMING_H1;//((BL_FPGA1*)(&Rnand))->VTIMING_H1;
  data[0]=(unsigned char)((a&(0x0000ff00))>>8);                        
  data[1]=(unsigned char)((a&(0x000000ff)));
  data[2]=(unsigned char)((b&(0x0000ff00))>>8); 
  data[3]=(unsigned char)((b&(0x000000ff)));
  data[4]=(unsigned char)((c&(0x0000ff00))>>8); 
  data[5]=(unsigned char)((c&(0x000000ff)));
  data[6]=(unsigned char)((d&(0x0000ff00))>>8);
  data[7]=(unsigned char)((d&(0x000000ff)));
	CAN_SendMessage(0x020102ff,data);
	
	a=Rnand.VTIMING_L1;//((BL_FPGA1*)(&Rnand))->VTIMING_L1;                            //FPGA1 U	parameter 
	b=Rnand.VCOM1_L;//((BL_FPGA1*)(&Rnand))->VCOM1_L;
  data[0]=(unsigned char)((a&(0x0000ff00))>>8);                        
  data[1]=(unsigned char)((a&(0x000000ff)));
  data[2]=(unsigned char)((b&(0x0000ff00))>>8);
  data[3]=(unsigned char)((b&(0x000000ff)));
  data[4]=0xff;
  data[5]=0xff;
  data[6]=0xff;
  data[7]=0xff;
	CAN_SendMessage(0x020103ff,data);	
	}
	
	for(k=0;k<15;k++)                                         //FPGA1   picture  parameter
	{
	  a=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_R1_VH;                            //FPGA1 	parameter 
		b=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_R1_VL; 
		c=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_G1_VH;                            //FPGA1 	parameter 
		d=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_G1_VL; 
		data[0]=(unsigned char)((a&(0x0000ff00))>>8);                       
		data[1]=(unsigned char)((a&(0x000000ff)));
		data[2]=(unsigned char)((b&(0x0000ff00))>>8);
		data[3]=(unsigned char)((b&(0x000000ff)));
		data[4]=(unsigned char)((c&(0x0000ff00))>>8);
		data[5]=(unsigned char)((c&(0x000000ff)));
		data[6]=(unsigned char)((d&(0x0000ff00))>>8);
		data[7]=(unsigned char)((d&(0x000000ff)));
		y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x02030001|y,data);
		
		a=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_B1_VH;                            //FPGA1 	parameter 
		b=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_B1_VL; 
		c=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_R2_VH;                            //FPGA1 	parameter 
		d=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_R2_VL; 
		data[0]=(unsigned char)((a&(0x0000ff00))>>8);                       
		data[1]=(unsigned char)((a&(0x000000ff)));
		data[2]=(unsigned char)((b&(0x0000ff00))>>8);
		data[3]=(unsigned char)((b&(0x000000ff)));
		data[4]=(unsigned char)((c&(0x0000ff00))>>8);
		data[5]=(unsigned char)((c&(0x000000ff)));
		data[6]=(unsigned char)((d&(0x0000ff00))>>8);
		data[7]=(unsigned char)((d&(0x000000ff)));
		y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x02030002|y,data);
		
		a=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_G2_VH;                            //FPGA1 	parameter 
		b=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_G2_VL; 
		c=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_B2_VH;                            //FPGA1 	parameter 
		d=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_B2_VL; 
		data[0]=(unsigned char)((a&(0x0000ff00))>>8);                       
		data[1]=(unsigned char)((a&(0x000000ff)));
		data[2]=(unsigned char)((b&(0x0000ff00))>>8);
		data[3]=(unsigned char)((b&(0x000000ff)));
		data[4]=(unsigned char)((c&(0x0000ff00))>>8);
		data[5]=(unsigned char)((c&(0x000000ff)));
		data[6]=(unsigned char)((d&(0x0000ff00))>>8);
		data[7]=(unsigned char)((d&(0x000000ff)));
		y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x02030003|y,data);
		
		
		a=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_R3_VH;                            //FPGA1 	parameter 
		b=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_R3_VL; 
		c=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_G3_VH;                            //FPGA1 	parameter 
		d=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_G3_VL; 
		data[0]=(unsigned char)((a&(0x0000ff00))>>8);                       
		data[1]=(unsigned char)((a&(0x000000ff)));
		data[2]=(unsigned char)((b&(0x0000ff00))>>8);
		data[3]=(unsigned char)((b&(0x000000ff)));
		data[4]=(unsigned char)((c&(0x0000ff00))>>8);
		data[5]=(unsigned char)((c&(0x000000ff)));
		data[6]=(unsigned char)((d&(0x0000ff00))>>8);
		data[7]=(unsigned char)((d&(0x000000ff)));
		y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x02030004|y,data);
		
		a=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_B3_VH;                            //FPGA1 	parameter 
		b=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_B3_VL; 
		c=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_R4_VH;                            //FPGA1 	parameter 
		d=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_R4_VL; 
		data[0]=(unsigned char)((a&(0x0000ff00))>>8);                       
		data[1]=(unsigned char)((a&(0x000000ff)));
		data[2]=(unsigned char)((b&(0x0000ff00))>>8);
		data[3]=(unsigned char)((b&(0x000000ff)));
		data[4]=(unsigned char)((c&(0x0000ff00))>>8);
		data[5]=(unsigned char)((c&(0x000000ff)));
		data[6]=(unsigned char)((d&(0x0000ff00))>>8);
		data[7]=(unsigned char)((d&(0x000000ff)));
		y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x02030005|y,data);
		
		a=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_G4_VH;                            //FPGA1 	parameter 
		b=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_G4_VL; 
		c=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_B4_VH;                            //FPGA1 	parameter 
		d=(((BL_FPGA1*)(&Rnand))->fpic[k]).FPIC_B4_VL; 
		data[0]=(unsigned char)((a&(0x0000ff00))>>8);                       
		data[1]=(unsigned char)((a&(0x000000ff)));
		data[2]=(unsigned char)((b&(0x0000ff00))>>8);
		data[3]=(unsigned char)((b&(0x000000ff)));
		data[4]=(unsigned char)((c&(0x0000ff00))>>8);
		data[5]=(unsigned char)((c&(0x000000ff)));
		data[6]=(unsigned char)((d&(0x0000ff00))>>8);
		data[7]=(unsigned char)((d&(0x000000ff)));
		y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x02030006|y,data);
		
		
		
		
  }
	
	
	
	for(kk=1;kk<6;kk++)     //s1tv _s5tv
	{
		t1=(kk-1)*4;
		t=kk<<8;
		
		
		f1=0x01;
		f2=0x02;
		f3=0x03;
		f4=0x04;
		f5=0x05;
		f6=0x06;
		f7=0x07;
		f8=0x08;		
		f9=0x09;
		f10=0x0a;
		f11=0x0b;
		f12=0x0c;
																	
		
	 for(k=0;k<4;k++)               //stv parameter
	 {
		  if(k>0)
		  {
		  	f1=f1+0x0c;
		  	f2=f2+0x0c;
		  	f3=f3+0x0c;
		  	f4=f4+0x0c;
		  	f5=f5+0x0c;
		  	f6=f6+0x0c;
		  	f7=f7+0x0c;
		  	f8=f8+0x0c;
		  	
		  	f9=f9+0x0c;
		  	f10=f10+0x0c;
		  	f11=f11+0x0c;
		  	f12=f12+0x0c;
      }
    
      a=fpga1_STV1.stv[k+t1].STV_TTTTT1;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_TTTTT1;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_TTTTT2;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_TTTTT2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f1|t,data);
      
      
      a=fpga1_STV1.stv[k+t1].STV_TTTTT3;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_TTTTT3;     //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_TTTTT4;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_TTTTT4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f2|t,data);
		  
		  a=fpga1_STV1.stv[k+t1].STV_TTTTT5;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_TTTTT5;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_NNNNNN;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_NNNNNN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f3|t,data);
      
      
      
      a=fpga1_STV1.stv[k+t1].STV_CK1_T1;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK1_T1;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_CK1_T2;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK1_T2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f4|t,data);
      
      
      a=fpga1_STV1.stv[k+t1].STV_CK1_T3;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK1_T3;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_CK1_T4;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK1_T4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f5|t,data);
		  
		  a=fpga1_STV1.stv[k+t1].STV_CK1_T5;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK1_T5;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_CK1_NN;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK1_NN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f6|t,data);
      
      
      a=fpga1_STV1.stv[k+t1].STV_CK2_T1;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK2_T1;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_CK2_T2;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK2_T2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f7|t,data);
		  
		  a=fpga1_STV1.stv[k+t1].STV_CK2_T3;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK2_T3;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_CK2_T4;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK2_T4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f8|t,data);
      
      a=fpga1_STV1.stv[k+t1].STV_CK2_T5;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK2_T5;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_CK2_NN;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_CK2_NN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f9|t,data);
		  
		  
		  a=fpga1_STV1.stv[k+t1].STV_RST_T1;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_RST_T1;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_RST_T2;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_RST_T2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f10|t,data);
		  
		  a=fpga1_STV1.stv[k+t1].STV_RST_T3;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_RST_T3;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_RST_T4;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_RST_T4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f11|t,data);
		  
		  
		  a=fpga1_STV1.stv[k+t1].STV_RST_T5;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_RST_T5;    //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV1.stv[k+t1].STV_RST_NN;//(((FPGA1_STV*)(&FPGA1_STV1))->stv[k+t1]).STV_RST_NN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f12|t,data);
    
   }
		
	}
	
	
	for(kk=1;kk<6;kk++)          //s6tv _s10tv
	{
		t1=(kk-1)*4;
		t=(kk+5)<<8;
		
		
		f1=0x01;
		f2=0x02;
		f3=0x03;
		f4=0x04;
		f5=0x05;
		f6=0x06;
		f7=0x07;
		f8=0x08;		
		f9=0x09;
		f10=0x0a;
		f11=0x0b;
		f12=0x0c;
																	
		
	 for(k=0;k<4;k++)               //stv parameter
	 {
		  if(k>0)
		  {
		  	f1=f1+0x0c;
		  	f2=f2+0x0c;
		  	f3=f3+0x0c;
		  	f4=f4+0x0c;
		  	f5=f5+0x0c;
		  	f6=f6+0x0c;
		  	f7=f7+0x0c;
		  	f8=f8+0x0c;
		  	
		  	f9=f9+0x0c;
		  	f10=f10+0x0c;
		  	f11=f11+0x0c;
		  	f12=f12+0x0c;
      }
    
      a=fpga1_STV2.stv[k+t1].STV_TTTTT1;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_TTTTT1;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_TTTTT2;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_TTTTT2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f1|t,data);
      
      
      a=fpga1_STV2.stv[k+t1].STV_TTTTT3;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_TTTTT3;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_TTTTT4;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_TTTTT4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f2|t,data);
		  
		  a=fpga1_STV2.stv[k+t1].STV_TTTTT5;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_TTTTT5;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_NNNNNN;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_NNNNNN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f3|t,data);
      
      
      
      a=fpga1_STV2.stv[k+t1].STV_CK1_T1;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK1_T1;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_CK1_T2;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK1_T2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f4|t,data);
      
      
      a=fpga1_STV2.stv[k+t1].STV_CK1_T3;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK1_T3;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_CK1_T4;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK1_T4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f5|t,data);
		  
		  a=fpga1_STV2.stv[k+t1].STV_CK1_T5;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK1_T5;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_CK1_NN;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK1_NN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f6|t,data);
      
      
      a=fpga1_STV2.stv[k+t1].STV_CK2_T1;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK2_T1;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_CK2_T2;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK2_T2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f7|t,data);
		  
		  a=fpga1_STV2.stv[k+t1].STV_CK2_T3;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK2_T3;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_CK2_T4;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK2_T4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f8|t,data);
      
      a=fpga1_STV2.stv[k+t1].STV_CK2_T5;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK2_T5;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_CK2_NN;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_CK2_NN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f9|t,data);
		  
		  
		  a=fpga1_STV2.stv[k+t1].STV_RST_T1;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_RST_T1;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_RST_T2;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_RST_T2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f10|t,data);
		  
		  a=fpga1_STV2.stv[k+t1].STV_RST_T3;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_RST_T3;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_RST_T4;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_RST_T4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f11|t,data);
		  
		  
		  a=fpga1_STV2.stv[k+t1].STV_RST_T5;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_RST_T5;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV2.stv[k+t1].STV_RST_NN;//(((FPGA1_STV*)(&FPGA1_STV2))->stv[k+t1]).STV_RST_NN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f12|t,data);
      
      
      
      
      
   }
		
		
		
	}
	
	
	for(kk=1;kk<6;kk++)   //s11tv_s15tv
	{
		t1=(kk-1)*4;
		t=(kk+0x0a)<<8;
		
		
		f1=0x01;
		f2=0x02;
		f3=0x03;
		f4=0x04;
		f5=0x05;
		f6=0x06;
		f7=0x07;
		f8=0x08;		
		f9=0x09;
		f10=0x0a;
		f11=0x0b;
		f12=0x0c;
																	
		
	 for(k=0;k<4;k++)               //stv parameter
	 {
		  if(k>0)
		  {
		  	f1=f1+0x0c;
		  	f2=f2+0x0c;
		  	f3=f3+0x0c;
		  	f4=f4+0x0c;
		  	f5=f5+0x0c;
		  	f6=f6+0x0c;
		  	f7=f7+0x0c;
		  	f8=f8+0x0c;
		  	
		  	f9=f9+0x0c;
		  	f10=f10+0x0c;
		  	f11=f11+0x0c;
		  	f12=f12+0x0c;
      }
    
      a=fpga1_STV3.stv[k+t1].STV_TTTTT1;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_TTTTT1;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_TTTTT2;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_TTTTT2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f1|t,data);
      
      
      a=fpga1_STV3.stv[k+t1].STV_TTTTT3;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_TTTTT3;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_TTTTT4;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_TTTTT4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f2|t,data);
		  
		  a=fpga1_STV3.stv[k+t1].STV_TTTTT5;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_TTTTT5;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_NNNNNN;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_NNNNNN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f3|t,data);
      
      
      
      a=fpga1_STV3.stv[k+t1].STV_CK1_T1;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK1_T1;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_CK1_T2;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK1_T2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f4|t,data);
      
      
      a=fpga1_STV3.stv[k+t1].STV_CK1_T3;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK1_T3;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_CK1_T4;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK1_T4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f5|t,data);
		  
		  a=fpga1_STV3.stv[k+t1].STV_CK1_T5;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK1_T5;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_CK1_NN;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK1_NN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f6|t,data);
      
      
      a=fpga1_STV3.stv[k+t1].STV_CK2_T1;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK2_T1;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_CK2_T2;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK2_T2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f7|t,data);
		  
		  a=fpga1_STV3.stv[k+t1].STV_CK2_T3;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK2_T3;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_CK2_T4;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK2_T4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f8|t,data);
      
      a=fpga1_STV3.stv[k+t1].STV_CK2_T5;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK2_T5;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_CK2_NN;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_CK2_NN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f9|t,data);
		  
		  
		  a=fpga1_STV3.stv[k+t1].STV_RST_T1;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_RST_T1;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_RST_T2;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_RST_T2; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f10|t,data);
		  
		  a=fpga1_STV3.stv[k+t1].STV_RST_T3;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_RST_T3;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_RST_T4;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_RST_T4; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f11|t,data);
		  
		  
		  a=fpga1_STV3.stv[k+t1].STV_RST_T5;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_RST_T5;                            //FPGA1 STV_TTTTT	parameter 
		  b=fpga1_STV3.stv[k+t1].STV_RST_NN;//(((FPGA1_STV*)(&FPGA1_STV3))->stv[k+t1]).STV_RST_NN; 
	    data[0]=(unsigned char)((a&(0x00ff0000))>>16);                        
		  data[1]=(unsigned char)((a&(0x0000ff00))>>8); 
		  data[2]=(unsigned char)((a&(0x000000ff)));
		  data[3]=(unsigned char)((b&(0x00ff0000))>>16); 
		  data[4]=(unsigned char)((b&(0x0000ff00))>>8);
		  data[5]=(unsigned char)((b&(0x000000ff)));
		  data[6]=0xff;
		  data[7]=0xff;
		  CAN_SendMessage(0x02020000|f12|t,data);
      
      
      
      
      
   }
		
		
		
	}
	
	
	
	
	for(k=0;k<15;k++) 
	{
		data[0]=fpga1_rgb_n.rgb_n[k].TR1H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TR1H_N;  
		data[1]=fpga1_rgb_n.rgb_n[k].TR1L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TR1L_N; 
		data[2]=fpga1_rgb_n.rgb_n[k].TR2H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TR2H_N; 
		data[3]=fpga1_rgb_n.rgb_n[k].TR2L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TR2L_N; 
		data[4]=fpga1_rgb_n.rgb_n[k].TR3H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TR3H_N; 
		data[5]=fpga1_rgb_n.rgb_n[k].TR3L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TR3L_N; 
		data[6]=fpga1_rgb_n.rgb_n[k].TR4H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TR4H_N; 
		data[7]=fpga1_rgb_n.rgb_n[k].TR4L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TR4L_N;

    y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x020b0001|y,data);		
		
		
		data[0]=fpga1_rgb_n.rgb_n[k].TG1H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TG1H_N;  
		data[1]=fpga1_rgb_n.rgb_n[k].TG1L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TG1L_N; 
		data[2]=fpga1_rgb_n.rgb_n[k].TG2H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TG2H_N; 
		data[3]=fpga1_rgb_n.rgb_n[k].TG2L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TG2L_N; 
		data[4]=fpga1_rgb_n.rgb_n[k].TG3H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TG3H_N; 
		data[5]=fpga1_rgb_n.rgb_n[k].TG3L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TG3L_N; 
		data[6]=fpga1_rgb_n.rgb_n[k].TG4H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TG4H_N; 
		data[7]=fpga1_rgb_n.rgb_n[k].TG4L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TG4L_N;

    y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x020b0002|y,data);		
		
		
		data[0]=fpga1_rgb_n.rgb_n[k].TB1H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TB1H_N;  
		data[1]=fpga1_rgb_n.rgb_n[k].TB1L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TB1L_N; 
		data[2]=fpga1_rgb_n.rgb_n[k].TB2H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TB2H_N; 
		data[3]=fpga1_rgb_n.rgb_n[k].TB2L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TB2L_N; 
		data[4]=fpga1_rgb_n.rgb_n[k].TB3H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TB3H_N; 
		data[5]=fpga1_rgb_n.rgb_n[k].TB3L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TB3L_N; 
		data[6]=fpga1_rgb_n.rgb_n[k].TB4H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TB4H_N; 
		data[7]=fpga1_rgb_n.rgb_n[k].TB4L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->rgb_n[k]).TB4L_N;

    y=(unsigned int)(k+1)<<8;
		CAN_SendMessage(0x020b0003|y,data);		
  }
	
	data[0]=fpga1_rgb_n.Tvcom1H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom1H_N);  
	data[1]=fpga1_rgb_n.Tvcom1L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom1L_N); 
	data[2]=fpga1_rgb_n.Tvcom2H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom2H_N); 
	data[3]=fpga1_rgb_n.Tvcom2L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom2L_N); 
	data[4]=fpga1_rgb_n.Tvcom3H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom3H_N); 
	data[5]=fpga1_rgb_n.Tvcom3L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom3L_N); 
	data[6]=fpga1_rgb_n.Tvcom4H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom4H_N); 
	data[7]=fpga1_rgb_n.Tvcom4L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom4L_N);
	CAN_SendMessage(0x020c0101,data);	
	
	data[0]=fpga1_rgb_n.Tvcom5H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom5H_N);  
	data[1]=fpga1_rgb_n.Tvcom5L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom5L_N); 
	data[2]=fpga1_rgb_n.Tvcom6H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom6H_N); 
	data[3]=fpga1_rgb_n.Tvcom6L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom6L_N); 
	data[4]=fpga1_rgb_n.Tvcom7H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom7H_N); 
	data[5]=fpga1_rgb_n.Tvcom7L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom7L_N); 
	data[6]=fpga1_rgb_n.Tvcom8H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom8H_N); 
	data[7]=fpga1_rgb_n.Tvcom8L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom8L_N);
	CAN_SendMessage(0x020c0102,data);	
	
	data[0]=fpga1_rgb_n.Tvcom9H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom9H_N);  
	data[1]=fpga1_rgb_n.Tvcom9L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom9L_N); 
	data[2]=fpga1_rgb_n.Tvcom10H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom10H_N); 
	data[3]=fpga1_rgb_n.Tvcom10L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom10L_N); 
	data[4]=fpga1_rgb_n.Tvcom11H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom11H_N); 
	data[5]=fpga1_rgb_n.Tvcom11L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom11L_N); 
	data[6]=fpga1_rgb_n.Tvcom12H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom12H_N); 
	data[7]=fpga1_rgb_n.Tvcom12L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom12L_N);
	CAN_SendMessage(0x020c0103,data);	
	
	data[0]=fpga1_rgb_n.Tvcom13H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom13H_N);  
	data[1]=fpga1_rgb_n.Tvcom13L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom13L_N); 
	data[2]=fpga1_rgb_n.Tvcom14H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom14H_N); 
	data[3]=fpga1_rgb_n.Tvcom14L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom14L_N); 
	data[4]=fpga1_rgb_n.Tvcom15H_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom15H_N); 
	data[5]=fpga1_rgb_n.Tvcom15L_N;//(unsigned char)(((FPGA1_RGB_N*)(&FPGA1_RGBN))->Tvcom15L_N); 
  CAN_SendMessage(0x020c0104,data);	
	
	data[0]=0xff;                           //end commmand of FPGA1 INIT parameter
  data[1]=0xff;
  data[2]=0xff;
  data[3]=0xff;
  data[4]=0xff;
  data[5]=0xff;
  data[6]=0xff;
  data[7]=0xff;
	CAN_SendMessage(0x020402ff,data);
	
	//ARM_FPGA2();
	
}



