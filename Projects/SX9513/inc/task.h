/******************************************************************************
 * Project        : STM8Sxxx-SX9513
 * File           : misc.c
 * Copyright      : 2014 Yosun Singapore Pte Ltd
 ******************************************************************************
  miscellaneous function implementation

  Change History:

    Version 1.0.0 - Aug 2014
    > Initial revision

******************************************************************************/
#ifndef _MISC_H_
#define _MISC_H_

#define SX9513_TUNE

/* Take note that below hardware I2C is NOT working. */
/* It may be SCL & SDA need external pull-up registor */
#define SOFT_I2C        

#define VERSION_MAX_SIZE        5
#define FIRMWARE_VERSION_ADDRESS        0x4000

typedef struct sTaskInstance
{
  void *p_device1;
  void *p_data;
  void *p_dataLen;
}tTaskInstance;

void delay_us(unsigned char time);
void SX9513_Handler(void);
#ifdef SX9513_TUNE
void SX9513_Tuner(void);
#endif
void get_input(void);
tTaskInstance* Task_Init(void);
void Task_Exec(tTaskInstance *task);
#endif
