
#ifndef __ARM_FPGA_H
#define __ARM_FPGA_H	
#include "stdint.h"
#include <stdio.h>
#include "demo_fatfs.h"
#include "can_network.h"

void CAN_SendMessage(unsigned int ID,unsigned char *array);
void ARM_FPGA1(void);
void ARM_FPGA2(void);
void ARM_FPGA1_uart(void);

void Auto_Mode(void);
#endif

