/******************************************************************************
 * Project        : STM8Sxxx-SX9513
 * File           : SX9513.c
 * Copyright      : 2014 Yosun Singapore Pte Ltd
 ******************************************************************************
  Change History:

    Version 1.0.0 - Aug 2014
    > Initial revision

******************************************************************************/
#include "stm8s.h"
#include "soft_i2c.h"
#include "task.h"

#define SX9513_I2C_ADDRESS      (0x2B<<1)

typedef struct 
{
  uint8_t IrqSrc;       // 0x00
  uint8_t TouchStatus;  // 0x01
  uint8_t ProxStatus;   // 0x02
  uint8_t CompStatus;   // 0x03
  uint8_t NVMCtrl;      // 0x04
  uint8_t Reserved1[2]; // 0x05~0x05
  uint8_t Spo2Mode;     // 0x07
  uint8_t PwrKey;       // 0x08
  uint8_t IrqMask;      // 0x09
  uint8_t Reserved2[2]; // 0x0A~0x0B
  uint8_t LEDMap1;      // 0x0C
  uint8_t LEDMap2;      // 0x0D
  uint8_t LEDPwmFreq;   // 0x0E
  uint8_t LEDMode;      // 0x0F
  uint8_t LEDIdle;      // 0x10
  uint8_t LEDOffDelay;  // 0x11
  uint8_t LED1On;       // 0x12
  uint8_t LED1Fade;     // 0x13
  uint8_t LED2On;       // 0x14
  uint8_t LED2Fade;     // 0x15
  uint8_t LEDPwrIdle;   // 0x16
  uint8_t LEDPwrOn;     // 0x17
  uint8_t LEDPwrOff;    // 0x18
  uint8_t LEDPwrFade;   // 0x19
  uint8_t LEDPwrOnPw;   // 0x1A
  uint8_t LEDPwrMode;   // 0x1B
  uint8_t Reserved3[2]; // 0x1C~0x1D
  uint8_t CapSenseEnable;       // 0x1E
  uint8_t CapSensRange0;        // 0x1F
  uint8_t CapSenseRange1;       // 0x20
  uint8_t CapSenseRange2;       // 0x21
  uint8_t CapSenseRange3;       // 0x22
  uint8_t CapSenseRange4;       // 0x23
  uint8_t CapSenseRange5;       // 0x24
  uint8_t CapSenseRange6;       // 0x25
  uint8_t CapSenseRange7;       // 0x26
  uint8_t CapSenseRangeAll;     // 0x27
  uint8_t CapSenseThresh0;      // 0x28
  uint8_t CapSenseThresh1;      // 0x29
  uint8_t CapSenseThresh2;      // 0x2A
  uint8_t CapSenseThresh3;      // 0x2B
  uint8_t CapSenseThresh4;      // 0x2C
  uint8_t CapSenseThresh5;      // 0x2D
  uint8_t CapSenseThresh6;      // 0x2E
  uint8_t CapSenseThresh7;      // 0x2F
  uint8_t CapSenseThreshComb;   // 0x30
  uint8_t CapSenseOp;   // 0x31
  uint8_t CapSenseMode; // 0x32
  uint8_t CapSenseDebounce;     // 0x33
  uint8_t CapSenseNegCompThresh;        // 0x34
  uint8_t CapSensePosCompThresh;        // 0x35
  uint8_t CapSensePosFilt;      // 0x36
  uint8_t CapSenseNegFilt;      // 0x37
  uint8_t CapSenseStuck;        // 0x38
  uint8_t CapSenseFrameSkip;    // 0x39
  uint8_t CapSenseMisc; // 0x3A
  uint8_t ProxCombChanMask;     // 0x3B
  uint8_t Reserved4[2]; // 0x3C~0x3D
  uint8_t SPOChanMap;   // 0x3E
  uint8_t SPOLevelBL0;  // 0x3F
  uint8_t SPOLevelBL1;  // 0x40
  uint8_t SPOLevelBL2;  // 0x41
  uint8_t SPOLevelBL3;  // 0x42
  uint8_t SPOLevelBL4;  // 0x43
  uint8_t SPOLevelBL5;  // 0x44
  uint8_t SPOLevelBL6;  // 0x45
  uint8_t SPOLevelBL7;  // 0x46
  uint8_t SPOLevelIdle; // 0x47
  uint8_t SPOLevelProx; // 0x48
  uint8_t Reserved5[2]; // 0x49~0x4A
  uint8_t BuzzerTrigger;        // 0x4B
  uint8_t BuzzerFreq;   // 0x4C
  uint8_t Reserved6[0x63-0x4D]; // 0x4D~0x62
  uint8_t CapSenseChanSelect;   // 0x63
  uint8_t CapSenseUsefulDataMsb;        // 0x64
  uint8_t CapSenseUsefulDataLsb;        // 0x65
  uint8_t CapSenseAverageDataMsb;       // 0x66
  uint8_t CapSenseAverageDataLsb;       // 0x67
  uint8_t CapSenseDiffDataMsb;  // 0x68
  uint8_t CapSenseDiffDataLsb;  // 0x69
  uint8_t CapSenseCompMsb;      // 0x6A
  uint8_t CapSenseCompLsb;      // 0x6B
  uint8_t Reserved[0xFF-0x6C];  // 0x6C~0xFE
  uint8_t I2CSoftReset; // 0xFF
}SX9513_Reg_Def;

#ifdef SOFT_I2C
uint8_t SX9513_Read(uint8_t reg)
{
  uint8_t rxData = 0;
  
  Soft_I2C_Start();/*I2C start*/
  Soft_I2C_Send_Byte(SX9513_I2C_ADDRESS);/*Send MMA7455 Address*/
  Soft_I2C_Ack();/*I2C_Ack*/
  Soft_I2C_Send_Byte(reg);/*Send MMA7455 Reg*/
  Soft_I2C_Ack();/*I2C_Ack*/
  Soft_I2C_Start();/*restart I2C*/
  Soft_I2C_Send_Byte(SX9513_I2C_ADDRESS|0x01);/*Send ReadCommand*/
  Soft_I2C_Ack();/*I2C_Ack*/
  rxData=Soft_I2C_Read_Byte();/*I2C ReadByte*/
  Soft_I2C_NAck();/*I2C_NAck*/
  Soft_I2C_Stop();/*I2C Stop*/
  
  return rxData;
}

void SX9513_Write(uint8_t reg, uint8_t data)
{
  Soft_I2C_Start();	/*I2C Start*/
  Soft_I2C_Send_Byte(SX9513_I2C_ADDRESS);	/*Send MMA7455 Driver  address*/
  Soft_I2C_Ack();	/*I2C_Ack*/
  Soft_I2C_Send_Byte(reg);	/*Send Write REG Address*/
  Soft_I2C_Ack();	/*I2C_Ack*/
  Soft_I2C_Send_Byte(data);	/*Send byte*/
  Soft_I2C_Ack();		/*I2C_Ack*/
  Soft_I2C_Stop();	/*I2C Stop*/
}
#else // STM8S Hard I2C
uint8_t SX9513_Read(uint8_t reg)
{
#if 0
  uint8_t rxData;
  
  /* wait for I2C bus to be free */
  while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY));
	
  /* I2C start */
  I2C_GenerateSTART(ENABLE);
	
  /* STM8S I2C EV5 */
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
  I2C_ClearFlag(I2C_FLAG_ACKNOWLEDGEFAILURE);
        
  /* Set slave adress and I2C master write mode */
  I2C_Send7bitAddress(SX9513_I2C_ADDRESS, I2C_DIRECTION_TX);

  /* STM8S EV6 */
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	
  I2C_Cmd(ENABLE);

  I2C_SendData(reg);
	
  /* STM8S EV8_2 */
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
  /* I2C start */ 
  I2C_GenerateSTART(ENABLE);
	
  /* STM8S EV5 */
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
	
  /* Set slave adress and I2C master read mode */
  I2C_Send7bitAddress(SX9513_I2C_ADDRESS, I2C_DIRECTION_RX);
	
  /* STM8S EV6 */
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	
  /* STM8S EV7 */
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED));
  rxData = I2C_ReceiveData();
	
  I2C_AcknowledgeConfig(I2C_ACK_NONE);
	
  /* I2C stop */
  I2C_GenerateSTOP(ENABLE);
  
  return rxData;
#else
  uint8_t rxData;
  
  /* check if I2C bus is busy */
  while(I2C->SR3 & 0x02);
  
  /* Generate start bit */
  I2C->CR2 |= 0x01;
  
  /* EV5¡GSB=1 */
  while(!(I2C->SR1 & 0x01));
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  
  /* Set slave adress and I2C master write mode */
  I2C->DR = SX9513_I2C_ADDRESS;
  
  /* wait for end of address transmission */
  while(!(I2C->SR1 & 0x02));
  
  I2C->SR1;
  I2C->SR3;
  
  /* Send register address */
  I2C->DR = reg;
  
  /* EV8_2 TxE=1 ¡ABTF=1 */
  while(!(I2C->SR1 & 0x84));
  
  /* Regenrate start bit */
  I2C->CR2 |= 0x01;
  
  /* EV5¡GSB=1 */
  while(!(I2C->SR1 & 0x01));
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  
  /* Set slave adress and I2C master read mode */
  I2C->DR = (SX9513_I2C_ADDRESS|0x01);
  
  /* wait for end of address transmission */
  while(!(I2C->SR1 & 0x02));
  
  I2C->SR1;
  I2C->SR3;
  
  /* Wait for RX data */
  while(!(I2C->SR1 & 0x40));
  rxData = I2C->DR;
  
  /* Disable ACK */
  I2C->CR2 &= ~0x04;
  
  /* Generate stop bit */
  I2C->CR2 |= 0x02;
  
  return rxData;
#endif
}

void SX9513_Write(uint8_t reg, uint8_t data)
{
  /* Wait for I2C bus to be free */
  while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY));
	
  /* I2C start */
  I2C_GenerateSTART(ENABLE);
	
  /* STM8S EV5 */ 
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT)); 
	
  /* Set slave address and I2C master write mode */
  I2C_Send7bitAddress(SX9513_I2C_ADDRESS, I2C_DIRECTION_TX);
	
  /* STM8S EV6 */
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	 
  /* send register address to slave */
  I2C_SendData(reg);
  /* STM8S EV8 */
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING));
	
  /* Send data to slave */
  I2C_SendData(data); 
	
  /* STM8S EV8 */
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 
	
  /* I2C stop */
  I2C_GenerateSTOP(ENABLE);
}
#endif

//static SX9513_Reg_Def sx9513_reg;
void SX9513_Init(void)
{ 
        SX9513_Read(0x00);
        SX9513_Write(0xFF, 0xDE);
        SX9513_Write(0xFF, 0x00);
        while(GPIO_ReadInputPin(PORT_NIRQ, PIN_NIRQ) == RESET);
	SX9513_Write(0x07,0x00);
	SX9513_Write(0x08,0x00);
	SX9513_Write(0x09, 0xE4);
	SX9513_Write(0x0C, 0x00);	// assign BL to LED engine 1
	SX9513_Write(0x0D, 0x00);	// not assign LED engine 2
	SX9513_Write(0x0E, 0x10);	// LED PWM Frequency 
	SX9513_Write(0x0F, 0x00);	
	SX9513_Write(0x10, 0x00);	// Set LED engine 1 and LED engine 2 idle intensity level
	SX9513_Write(0x11, 0x00);	// Set time delay from loss of touch/prox to start of fade out.
	SX9513_Write(0x12, 0x00);	// Set LED engine 1 on intensity level.
	SX9513_Write(0x13, 0x08);	// LED Engine 1 Fade In/Out Timing
	SX9513_Write(0x1F, 0x43);
	SX9513_Write(0x20, 0x43);
	SX9513_Write(0x21, 0x43);
	SX9513_Write(0x22, 0x43);
	SX9513_Write(0x23, 0x83);
	SX9513_Write(0x24, 0x83);
	SX9513_Write(0x25, 0x83);
	SX9513_Write(0x26, 0x83);
	SX9513_Write(0x27, 0x43);
	SX9513_Write(0x28, 0x01);
	SX9513_Write(0x29, 0x01);
	SX9513_Write(0x2A, 0x01);
	SX9513_Write(0x2B, 0x01);
	SX9513_Write(0x2C, 0x60);
	SX9513_Write(0x2D, 0x60);
	SX9513_Write(0x2E, 0x60);
	SX9513_Write(0x2F, 0x60);
	SX9513_Write(0x30, 0x10);
	SX9513_Write(0x31, 0x14);
        SX9513_Write(0x32, 0x14);
	SX9513_Write(0x33, 0x0B);
	SX9513_Write(0x34, 0xFF);
	SX9513_Write(0x35, 0xFF);
	SX9513_Write(0x36, 0x05);
	SX9513_Write(0x37, 0x02);
	SX9513_Write(0x38, 0x00);
	SX9513_Write(0x3B, 0x00);
	SX9513_Write(0x3E, 0xFF);
	
	SX9513_Write(0x1E, 0x0F);
}

