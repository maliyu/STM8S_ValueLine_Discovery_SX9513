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
#include <string.h>
#include "stm8s.h"
#include "sx9513.h"
#include "task.h"
#include "board.h"

#define CMD_BODY_MAX_SIZE 20
#define CMD_OPTIONS_MAX_SIZE 6
#define INPUT_BUFFER_SIZE (CMD_BODY_MAX_SIZE+CMD_OPTIONS_MAX_SIZE+2)

#define ISUNSIGNED(a) (a>0 && ~a>0)

#define IS_SX9513_DIFFDATA(a) ((a & 0x01) == 0x01)
#define IS_SX9513_COMP(a) ((a & 0x02) == 0x02)
#define IS_SX9513_TOUCHSTATUS(a) ((a & 0x04) == 0x04)

#define IS_ASCII_NUMERIC(a) (a >= 0x30 && a <= 0x39)

typedef enum
{
  CMD_INVALID = 0,
  CMD_VALID,
  CMD_SET,
  CMD_QUERY
}CMD_TYPE;

static uint8_t IrqSrc = 0;
static uint8_t PreTouchStatus = 0;
static uint8_t TouchStatus;
//static uint8_t touchIRQ = 0x20;
static uint8_t PowerFunc = 0;
static uint8_t SpeedFunc = 0;
static uint8_t TimerFunc = 0;
static uint8_t total_input_char_number = 0;
static uint8_t input_buffer[INPUT_BUFFER_SIZE];
static tTaskInstance taskInstance;
static uint8_t sx9513DiffDataCnt = 0;
static uint8_t sx9513CompCnt = 0;
static uint8_t sx9513TouchStatusCnt = 0;
static uint8_t sx9513Tuner = 0x00;
static char byteMap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

static void discard_input_buffer(void)
{
  uint8_t i;
  
  for(i=0;i<INPUT_BUFFER_SIZE;i++)
  {
    input_buffer[i] = 0;
  }
  
  total_input_char_number = 0;
}

static void convert_ascii2byte(uint8_t *p_ascii, uint8_t *p_byte)
{
  if(p_ascii == 0 || p_byte == 0)
  {
    return;
  }
  
  if(IS_ASCII_NUMERIC(p_ascii[0]) && IS_ASCII_NUMERIC(p_ascii[1]))
  {
    *p_byte = (((p_ascii[0]-0x30) & 0x0F)<<4) | ((p_ascii[1]-0x30) & 0x0F);
  }
}

/* command would be like at command */
/* at+xxxx=x,x is for setting */
/* at+xxxx=? is for query */
static void cmd_decoder(uint8_t *p_cmd, uint8_t cmdLen)
{
  if((p_cmd != 0) && (cmdLen <= (CMD_BODY_MAX_SIZE+CMD_OPTIONS_MAX_SIZE+1)))
  {
    uint8_t cmdBody[CMD_BODY_MAX_SIZE];
    uint8_t cmdOptions[CMD_OPTIONS_MAX_SIZE];
    uint8_t i;
    CMD_TYPE cmd_type = CMD_INVALID;
    
    memset(cmdBody, 0, CMD_BODY_MAX_SIZE);
    memset(cmdOptions, 0, CMD_OPTIONS_MAX_SIZE);
    for(i=0;i<cmdLen;i++)
    {
      if(p_cmd[i] == '=')
      {
        /* command body found */
        if(i <= CMD_BODY_MAX_SIZE)
        {
          memcpy(cmdBody, p_cmd, i);
          if(CMD_OPTIONS_MAX_SIZE < cmdLen-i-1)
          {
            memcpy(cmdOptions, p_cmd+i+1, CMD_OPTIONS_MAX_SIZE);
            cmdOptions[CMD_OPTIONS_MAX_SIZE-1] = '\0';
          }
          else
          {
            memcpy(cmdOptions, p_cmd+i+1, cmdLen-i-1);
            //cmdOptions[cmdLen-i-1] = '\0';
          }
          cmd_type = CMD_VALID;
        }
        break;
      }
    }
    
    if(CMD_VALID == cmd_type)
    {
      if(strcmp(cmdBody, "at+version") == 0)
      {
        /* Only retreive firmware version                       */
        /* User could not set firmware version by this command  */
        /* at+version=?                                         */
        if(cmdOptions[0] == '?')
        {
          uint8_t version[VERSION_MAX_SIZE+1];
          
          memset(version, 0, VERSION_MAX_SIZE+1);
          EEPROM_Read(FIRMWARE_VERSION_ADDRESS, version, VERSION_MAX_SIZE);
          version[VERSION_MAX_SIZE]= '\r';
          Uart_Prints(version, VERSION_MAX_SIZE+1);
        }
      }
      else if(strcmp(cmdBody, "at+diffdata") == 0)
      {
        /* This command is to check usefull data of SX9513 */
        if((cmdOptions[0] >= 0x30) && (cmdOptions[0] <= 0x37))
        {
          if(cmdOptions[1] == ',')
          {
            if((cmdOptions[2] >= 0x30) && (cmdOptions[2] <= 0x39) && (cmdOptions[3] >= 0x30) && (cmdOptions[3] <= 0x39)) 
            {
              sx9513DiffDataCnt = (cmdOptions[2]-0x30)*10+cmdOptions[3]-0x30;
              sx9513Tuner |= 0x01;
            }
          }
        }
      }
      else if(strcmp(cmdBody, "at+comp") == 0)
      {
        /* This command is to check offset compensation DAC code of SX9513 */
        if(cmdOptions[0] == '?')
        {
          sx9513CompCnt = 5;
          sx9513Tuner |= 0x02;
        }
      }
      else if(strcmp(cmdBody, "at+touchstatus") == 0)
      {
        /* This command is to check touch detected on indicated BL channel */
        if(cmdOptions[0] == '?')
        {
          sx9513TouchStatusCnt = 30;
          sx9513Tuner |= 0x04;
        }
      }
      else if(strcmp(cmdBody, "at+sx9513conf") == 0)
      {
        /* This command is to check or set configuration parameter of SX9513 */
        if(IS_ASCII_NUMERIC(cmdOptions[0]) && IS_ASCII_NUMERIC(cmdOptions[1]))
        {
          if(cmdOptions[2] == ',')
          {
            /* Set parameter */
          }
          else
          {
            /* Check parameter */
            uint8_t address = 0;
            uint8_t para = 0;
            
            convert_ascii2byte(cmdOptions, &address);
            para = SX9513_Read(address);
            Uart_Prints(&para, 1); 
          }
        }
      }
    }
  }
}

#ifdef SX9513_TUNE
/*
static void SX9513_Tuner(void)
{
  CapSenseUsefulDataMsb = SX9513_Read(0x64);
  CapSenseUsefulDataLsb = SX9513_Read(0x65);
  CapSenseAverageDataMsb = SX9513_Read(0x66);
  CapSenseAverageDataLsb = SX9513_Read(0x67);
  CapSenseDiffDataMsb = SX9513_Read(0x68);
  CapSenseDiffDataLsb = SX9513_Read(0x69);
  CapSenseCompMsb = SX9513_Read(0x6A);
  CapSenseCompLsb = SX9513_Read(0x6B);
}
*/
#endif

/* Utility function to convert nibbles (4 bit values) into a hex character representation */
static char nibbleToChar(uint8_t nibble)
{
  uint8_t byteMapLen = sizeof(byteMap);
  
  if(nibble < byteMapLen) 
  {
    return byteMap[nibble];
  }
  else
  {
    return '*';
  }
	
}

/* Convert a buffer of binary values into a hex string representation */
static void bytesToHexString(char *str, uint8_t *bytes, size_t buflen)
{
  uint8_t i;
  
  if((str == 0) || (bytes == 0) || (buflen > INPUT_BUFFER_SIZE))
  {
    str[0] = '\0';
    
    return;
  }
	
  for(i=0; i<buflen; i++) 
  {
    str[i*2] = nibbleToChar(bytes[i] >> 4);
    str[i*2+1] = nibbleToChar(bytes[i] & 0x0f);
  }
  //str[i] = '\0';
}

tTaskInstance* Task_Init(void)
{
  uint8_t i;
  
  for(i=0;i<INPUT_BUFFER_SIZE;i++)
  {
    input_buffer[i] = 0;
  }
  
  taskInstance.p_device1 = 0;
  taskInstance.p_data = input_buffer;
  taskInstance.p_dataLen = &total_input_char_number;
  
  //EEPROM_Read(DEVICE_PARAMETERS_ADDRESS, (unsigned char *)(&devicePara), sizeof(struct t_DeviceParameters));
  
  return (&taskInstance);
}

void Task_Exec(tTaskInstance *task)
{     
  if(task == 0)
  {
    return;
  }
  
  //SX9513_Handler();
  
  disableInterrupts();
  /* check if the last char is Carriage Return(CR) */
  /* If yes, then decode the message */
  /* If no, then discard the buffer if max buffer size reach */
  /* Otherwise, wait for CR */
  if((total_input_char_number > 0) && (input_buffer[total_input_char_number-1] == '\r'))
  {
    //Uart_Prints2(input_buffer, total_input_char_number);
    
    /* No transmit UART RX data through RF */  
    //radio->SetTxPacket(input_buffer, total_input_char_number);
    
    cmd_decoder(input_buffer, total_input_char_number);
    
    discard_input_buffer();
  }
  else if(total_input_char_number == INPUT_BUFFER_SIZE)
  {
      discard_input_buffer();
  }
  enableInterrupts();
  
  if(IS_SX9513_DIFFDATA(sx9513Tuner))
  {
    if(sx9513DiffDataCnt > 0)
    {
      char diffDataStr[7];
      uint8_t capSenseDiffData[2];
  
      /* Selected channel diff data. Signed, 2's complement format */
      capSenseDiffData[0] = SX9513_Read(0x68);
      capSenseDiffData[1] = SX9513_Read(0x69);
      
#if 0
      /* convert 2's complement to +xxxx or -xxxx format */
      /* Here xxxx is decimal value, not hex value       */  
      if(capSenseDiffData[0] & 0x10 == 0x10)
      {
        uint16_t tmp;
        
        diffDataStr[0] = '-';
        tmp = capSenseDiffData[0] & 0x0F;
        tmp <<= 8;
        tmp |= capSenseDiffData[1];
        tmp = 0x0FFF-tmp+1;
        capSenseDiffData[0] = (uint8_t)((tmp & 0xFF00)>>8);
        capSenseDiffData[1] = (uint8_t)(tmp & 0x00FF);
      }
      else
      {
        diffDataStr[0] = '+';
      }
#else
      diffDataStr[0] = 0x20;
#endif
      
      
      bytesToHexString(&diffDataStr[1], capSenseDiffData, sizeof(capSenseDiffData));
      /* create new line for last data */
      if(sx9513DiffDataCnt == 1)
      {
        diffDataStr[5] = '\r';
      }
      else
      {
        diffDataStr[5] = 0x20;
      }
      diffDataStr[6] = '\0';
      Uart_Prints((unsigned char *)diffDataStr, strlen(diffDataStr));
      
      --sx9513DiffDataCnt;
    }
    else
    {
      sx9513Tuner &= ~(0x01);
    }
  }
  else if(IS_SX9513_COMP(sx9513Tuner))
  {
    if(sx9513CompCnt > 0)
    {
      char compStr[6];
      uint8_t capSenseComp[2];
  
      /* Offset compensation DAC code*/
      capSenseComp[0] = SX9513_Read(0x6A);
      capSenseComp[1] = SX9513_Read(0x6B);
      
      bytesToHexString(&compStr[0], capSenseComp, sizeof(capSenseComp));
      /* create new line for last data */
      if(sx9513CompCnt == 1)
      {
        compStr[4] = '\r';
      }
      else
      {
        compStr[4] = 0x20;
      }
      compStr[5] = '\0';
      Uart_Prints((unsigned char *)compStr, strlen(compStr));
      
      --sx9513CompCnt;
    }
    else
    {
      sx9513Tuner &= ~(0x02);
    }
  }
  else if(IS_SX9513_TOUCHSTATUS(sx9513Tuner))
  {
    if(sx9513TouchStatusCnt > 0)
    {
      char compStr[6];
      uint8_t capSenseComp[2];
  
      /* Offset compensation DAC code*/
      capSenseComp[0] = SX9513_Read(0x00);
      capSenseComp[1] = SX9513_Read(0x01);
      
      bytesToHexString(&compStr[0], capSenseComp, sizeof(capSenseComp));
      /* create new line for last data */
      if(sx9513TouchStatusCnt == 1)
      {
        compStr[4] = '\r';
      }
      else
      {
        compStr[4] = 0x20;
      }
      compStr[5] = '\0';
      Uart_Prints((unsigned char *)compStr, strlen(compStr));
      
      --sx9513TouchStatusCnt;
    }
    else
    {
      sx9513Tuner &= ~(0x04);
    }
  }
}

void get_input(void)
{
  /* discard the receiving char if buffer data is not handled by task handler */
  if(total_input_char_number < INPUT_BUFFER_SIZE)
  {
#if defined(STM8S003)
    input_buffer[total_input_char_number++] = UART1->DR;
#elif defined(STM8L15X_MD)
    input_buffer[total_input_char_number++] = USART1->DR;
#endif
  }
}

void delay_us(u8 time)
{
  u8 i=0;
  do
  {
    for(i=0;i<9;i++) /*2???©P´Á*/
    nop();           /*1???©P´Á*/
  }while(time--);
}

/* BL0 - power button with LED0 */
/* BL1 - speed+ button with LED1, LED2, LED3 */
/* BL2 - speed- button with LED1, LED2, LED3 */
/* BL3 - timer button with LED4, LED5, LED6, LED7 */
void SX9513_Handler(void)
{
	uint8_t tmp;
	
	IrqSrc = SX9513_Read(0x00);
	TouchStatus = SX9513_Read(0x01);
        
        //Uart_Prints(&IrqSrc, 1);
        //Uart_Prints(&TouchStatus, 1);
        //tmp = '\r';
        //Uart_Prints(&tmp, 1);
#if 1
        if((IrqSrc & 0x40) == 0x40)
        {
          /* Touch IRQ */
          if((TouchStatus & 0x01) == 0x01)
          {
            Touch_Indicate();
          }
          else if((TouchStatus & 0x02) == 0x02)
          {
            Touch_Indicate();
            
          }
          else if((TouchStatus & 0x04) == 0x04)
          {
            Touch_Indicate();
          }
          else if((TouchStatus & 0x08) == 0x08)
          {
            Touch_Indicate();
          }
        }
        else if((IrqSrc & 0x20) == 0x20)
        {
          /* Release IRQ */
        }
#endif     
#if 0	
	if((IrqSrc & 0x40) == 0x40)
	{
		// detect touch event
		if((TouchStatus & 0x01) == 0x01)
		{
			// BL0
			// LED0 on, LED1 on, other LED off
			//STM_EVAL_LEDToggle(LED3);
			if(PowerFunc == 0)
			{
				SX9513_Write(0x0C, 0x03);
				SX9513_Write(0x10, 0xFF);
				//SX9513_Write(0x1E, 0x0F);
				PowerFunc = 1;
				SpeedFunc = 1;
				TimerFunc = 0;
			}
			else
			{
				SX9513_Write(0x0C, 0x00);
				SX9513_Write(0x10, 0x00);
				//SX9513_Write(0x1E, 0x01);
				PowerFunc = 0;
			}
		}
		else if((TouchStatus & 0x02) == 0x02)
		{
			// BL1
			if(SpeedFunc == 1)
			{
				// LED1 off, LED2 on, LED3 off
				tmp = SX9513_Read(0x0C);
				tmp &= 0xF1;
				tmp |= 0x04;
				SX9513_Write(0x0C, tmp);
				SpeedFunc = 2;
			}
			else if(SpeedFunc == 2)
			{
				// LED1 off, LED2 off, LED3 on
				tmp = SX9513_Read(0x0C);
				tmp &= 0xF1;
				tmp |= 0x08;
				SX9513_Write(0x0C, tmp);
				SpeedFunc = 3;
			}
		}
		else if((TouchStatus & 0x04) == 0x04)
		{
			// BL2
			if(SpeedFunc == 3)
			{
				// LED1 off, LED2 on, LED3 off
				tmp = SX9513_Read(0x0C);
				tmp &= 0xF1;
				tmp |= 0x04;
				SX9513_Write(0x0C, tmp);
				SpeedFunc = 2;
			}
			else if(SpeedFunc == 2)
			{
				// LED1 on, LED2 off, LED3 off
				tmp = SX9513_Read(0x0C);
				tmp &= 0xF1;
				tmp |= 0x02;
				SX9513_Write(0x0C, tmp);
				SpeedFunc = 1;
			}
		}
		else if((TouchStatus & 0x08) == 0x08)
		{
			// BL3
			if(TimerFunc == 0)
			{
				// LED4 on, LED5 off, LED6 off, LED7 off
				tmp = SX9513_Read(0x0C);
				tmp &= 0x0F;
				tmp |= 0x10;
				SX9513_Write(0x0C, tmp);
				TimerFunc = 1;
			}
			else if(TimerFunc == 1)
			{
				// LED4 off, LED5 on, LED6 off, LED7 off
				tmp = SX9513_Read(0x0C);
				tmp &= 0x0F;
				tmp |= 0x20;
				SX9513_Write(0x0C, tmp);
				TimerFunc = 2;
			}
			else if(TimerFunc == 2)
			{
				// LED4 off, LED5 off, LED6 on, LED7 off
				tmp = SX9513_Read(0x0C);
				tmp &= 0x0F;
				tmp |= 0x40;
				SX9513_Write(0x0C, tmp);
				TimerFunc = 3;
			}
			else if(TimerFunc == 3)
			{
				// LED4 off, LED5 off, LED6 off, LED7 on
				tmp = SX9513_Read(0x0C);
				tmp &= 0x0F;
				tmp |= 0x80;
				SX9513_Write(0x0C, tmp);
				TimerFunc = 0;
			}
		}
                
                /* Trigger compensation on all channels */
                //tmp = SX9513_Read(0x00);
                //SX9513_Write(0x00, tmp | 0x04);
                
                Touch_Indicate();
	}
	else if((IrqSrc & 0x20) == 0x20)
	{
		// detect release event
		if(((TouchStatus & 0x01) == 0x00) && ((PreTouchStatus & 0x01) == 0x01) )
		{
			// BL0
			//STM_EVAL_LEDToggle(LED3);
			
		}
                
                //Touch_Indicate();
	}
	
	PreTouchStatus = TouchStatus;   
#endif
}
