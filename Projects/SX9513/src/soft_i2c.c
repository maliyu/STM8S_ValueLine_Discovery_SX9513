/******************** (C) COPYRIGHT  风驰iCreate嵌入式开发工作室 **************
 * 文件名  ：soft_i2c.c
 * 描述    ：模拟I2C函数库   
 * 实验平台：风驰STM8开发板
 * 库版本  ：V2.1.0
 * 作者    ：风驰  QQ：779814207
 * 博客    ：
 * 淘宝    ：http://shop71177993.taobao.com/
 *修改时间 ：2012-12-9

*******************************************************************************/


#include "soft_i2c.h"
#include "task.h"

#ifdef SOFT_I2C
void Soft_I2C_Init(void)
{
  Set_I2C_SCL_Out;
  Set_I2C_SDA_Out;
  Set_I2C_SCL_HIGHT;
  Set_I2C_SDA_HIGHT;
}
/*********************************************************
 * 名称: Soft_I2C_Start
 * 功能: 模拟I2C的起始条件
 * 形参: 无        
 * 返回: 无
 * 说明: 标准的I2C协议I2C起始条件：SCL为高时，SDA由高变低
               
 *********************************************************/
void Soft_I2C_Start(void)
{
  Set_I2C_SDA_Out;
  Set_I2C_SCL_HIGHT;
  Set_I2C_SDA_HIGHT;
  delay_us(4);
  Set_I2C_SDA_LOW;
  //标准的I2C协议I2C起始条件：SCL为高时，SDA由高变低
  delay_us(4);
  Set_I2C_SCL_LOW;//钳住I2C总线，准备发送或接收数据 
}
/*********************************************************
 * 名称: Soft_I2C_Stop
 * 功能: 模拟I2C的停止条件
 * 形参: 无        
 * 返回: 无
 * 说明: 标准的I2C协议I2C停止条件：SCL为高时，SDA由低变高
               
 *********************************************************/
void Soft_I2C_Stop(void)
{
  Set_I2C_SDA_Out;
  Set_I2C_SCL_HIGHT;
  Set_I2C_SDA_LOW;
  delay_us(4);
  Set_I2C_SDA_HIGHT;
  //标准的I2C协议I2C停止条件：SCL为高时，SDA由低变高
  delay_us(4);

}
/*********************************************************
 * 名称: Soft_I2C_Ack
 * 功能: 模拟I2C的应答
 * 形参: 无        
 * 返回: 无
 * 说明: 根据标准的I2C协议，从I2C从器件读1个byte后
         也就是在第9个CLK的时候，I2C主设备的SDA要变低
         表示I2C主器件已经接收完一个字节byte，I2C从器件
         继续发送数据
               
 *********************************************************/
void Soft_I2C_Ack(void)
{
  Set_I2C_SCL_LOW;
  Set_I2C_SDA_Out;
  Set_I2C_SDA_LOW;
  delay_us(2);
  Set_I2C_SCL_HIGHT;
  delay_us(2);
  Set_I2C_SCL_LOW;

}
/*********************************************************
 * 名称: Soft_I2C_NAck
 * 功能: 模拟I2C的不应答
 * 形参: 无        
 * 返回: 无
 * 说明: 
               
 *********************************************************/
void Soft_I2C_NAck(void)
{
  Set_I2C_SCL_LOW;
  Set_I2C_SDA_Out;
  Set_I2C_SDA_HIGHT;
  delay_us(2);
  Set_I2C_SCL_HIGHT;
  delay_us(2);
  Set_I2C_SCL_LOW;
}
/*********************************************************
 * 名称: Soft_I2C_Send_Byte
 * 功能: 模拟I2C主器件向从器件发送一个字节
 * 形参: Byte        
 * 返回: 无
 * 说明: 
               
 *********************************************************/
void Soft_I2C_Send_Byte(u8 Byte)
{
  u8 bit;
  Set_I2C_SDA_Out;
  Set_I2C_SCL_LOW;
  for(bit=0;bit<8;bit++)
  {
    if((Byte&0x80)>>7)Set_I2C_SDA_HIGHT;
    else Set_I2C_SDA_LOW;
    Byte<<=1;
    delay_us(2);
    Set_I2C_SCL_HIGHT;
    delay_us(2);
    Set_I2C_SCL_LOW;
    delay_us(2);
    
  }
  
}
/************************************************
 * 名称: Soft_I2C_Read_Byte
 * 功能: 模拟I2C主器件向从器件发送一个字节
 * 形参: 无       
 * 返回: 一个字节
 * 说明: 主器件从从器件里读一个字节
               
 **************************************************/
u8 Soft_I2C_Read_Byte(void)
{
  u8 bit,Receive_Byte=0;
  Set_I2C_SDA_In;
  for(bit=0;bit<8;bit++)
  {
    Set_I2C_SCL_LOW;
    delay_us(2);
    Set_I2C_SCL_HIGHT;
    Receive_Byte<<=1;//把上一时刻的值左移一位
    if(Get_I2C_SDA)Receive_Byte|=0x01;
    delay_us(1);
  }
  return Receive_Byte;
}
#endif
