#ifndef __SYSINFOTEST_H__
#define __SYSINFOTEST_H__

union int_2_bit{
	struct {
	  unsigned int res : 7;
	  unsigned int exit_en : 1;
	  unsigned int res2 : 24;
	}temp_bit;
	unsigned int temp_int;
};
 
enum key_status_enum{
  LEFT = 0,
	RIGHT
};

struct roundbutton_struct{
	enum key_status_enum status;
	char Valid_flag;
};

#define EXIT_OPEN ((union int_2_bit *)EXTI_BASE)->temp_bit.exit_en = 0x01
#define EXIT_CLOSE ((union int_2_bit *)EXTI_BASE)->temp_bit.exit_en = 0x00


void EXTI_Config(void);

#endif
