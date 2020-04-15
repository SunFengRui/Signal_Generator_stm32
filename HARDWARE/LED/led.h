#ifndef _LED_H
#define _LED_H
#include "sys.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//LED�˿ڶ���
#define BEEP PFout(8)
#define LED0 PFout(9)
#define LED1 PFout(10)
#define TEST_IO PEout(0)

void LED_Init(void); //��ʼ��
void Set_Led(u8 LED,u8 state);
#endif
