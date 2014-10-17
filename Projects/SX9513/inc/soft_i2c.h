
#ifndef SOFT_I2C_H
#define SOFT_I2C_H
#include "stm8s.h"
#include "sx9513.h"

#define I2C_PIN  PORT_SCL
#define I2C_SCL  PIN_SCL
#define I2C_SDA  PIN_SDA
#define Set_I2C_SCL_HIGHT    GPIO_WriteHigh(I2C_PIN, I2C_SCL)
#define Set_I2C_SCL_LOW      GPIO_WriteLow(I2C_PIN, I2C_SCL)
#define Set_I2C_SDA_HIGHT    GPIO_WriteHigh(I2C_PIN, I2C_SDA)
#define Set_I2C_SDA_LOW      GPIO_WriteLow(I2C_PIN, I2C_SDA)
#define Get_I2C_SDA          GPIO_ReadInputPin(I2C_PIN, I2C_SDA)
#define Set_I2C_SCL_Out      GPIO_Init(I2C_PIN, I2C_SCL, GPIO_MODE_OUT_PP_HIGH_FAST)
#define Set_I2C_SDA_Out      GPIO_Init(I2C_PIN, I2C_SDA, GPIO_MODE_OUT_PP_HIGH_FAST)
//#define Set_I2C_SDA_In       GPIO_Init(I2C_PIN, I2C_SDA, GPIO_MODE_IN_FL_NO_IT)
#define Set_I2C_SDA_In       GPIO_Init(I2C_PIN, I2C_SDA, GPIO_MODE_IN_PU_NO_IT)

void Soft_I2C_Init(void);
void Soft_I2C_Start(void);
void Soft_I2C_Stop(void);
void Soft_I2C_Ack(void);
void Soft_I2C_NAck(void);
void Soft_I2C_Send_Byte(u8 Byte);
u8 Soft_I2C_Read_Byte(void);
#endif