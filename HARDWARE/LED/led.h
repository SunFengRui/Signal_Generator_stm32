#ifndef _LED_H
#define _LED_H
#include "sys.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//LED端口定义
#define BEEP PFout(8)
#define LED0 PFout(9)
#define LED1 PFout(10)
#define TEST_IO PEout(0)

void LED_Init(void); //初始化
void Set_Led(u8 LED,u8 state);
#endif
