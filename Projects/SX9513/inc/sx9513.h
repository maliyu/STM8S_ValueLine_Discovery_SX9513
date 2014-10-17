/******************************************************************************
 * Project        : STM8Sxxx-SX9513
 * File           : SX9513.c
 * Copyright      : 2014 Yosun Singapore Pte Ltd
 ******************************************************************************
  Change History:

    Version 1.0.0 - Aug 2014
    > Initial revision

******************************************************************************/
#ifndef _SX9513_H_
#define _SX9513_H_

#define PORT_NIRQ       GPIOD
#define PIN_NIRQ        GPIO_PIN_7
#define PORT_SCL        GPIOB
#define PIN_SCL         GPIO_PIN_2
#define PORT_SDA        GPIOB
#define PIN_SDA         GPIO_PIN_3

typedef enum{
  STANDARDSPEED=100000,
  FASTSPEED=400000
}I2C_SpeedMode_TypeDef;

void SX9513_Init(void);
uint8_t SX9513_Read(uint8_t reg);
void SX9513_Write(uint8_t reg, uint8_t data);
#endif
