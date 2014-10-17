/**
  ******************************************************************************
  * @file    Project/main.c 
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    18-November-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 


/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "stm8s.h"
#include "sx9513.h"
#include "soft_i2c.h"
#include "task.h"
#include "board.h"

/* Private defines -----------------------------------------------------------*/
#define FIRMWARE_VERSION        "2.0.0"
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

static tTaskInstance *p_task = 0;

static void update_fwVersion(void)
{
  uint8_t version[VERSION_MAX_SIZE+1];
  
  memset(version, 0, VERSION_MAX_SIZE+1);
  EEPROM_Read(FIRMWARE_VERSION_ADDRESS, version, VERSION_MAX_SIZE);
  if(strcmp(version, FIRMWARE_VERSION) != 0)
  {
    EEPROM_Write(FIRMWARE_VERSION_ADDRESS, FIRMWARE_VERSION, VERSION_MAX_SIZE);
  }
}

void main(void)
{   
  disableInterrupts();
  
  /* STM8S003 hardware init */
  Board_Init();
  
  /* Semtech SX9513 init */
  SX9513_Init();
  
  /* main application init */
  p_task = Task_Init();
  
  update_fwVersion();
  
  enableInterrupts();
  
  /* Infinite loop */
  while (1)
  {
    Task_Exec(p_task);
  }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
