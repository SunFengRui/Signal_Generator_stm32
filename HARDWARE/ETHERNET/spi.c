/**
  ******************************************************************************
  * @file    spi.c
  * $Author: 飞鸿踏雪 $
  * $Revision: 17 $
  * $Date:: 2014-10-25 11:16:48 +0800 #$
  * @brief   SPI驱动函数实现.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, EmbedNet</center>
  *<center><a href="http:\\www.embed-net.com">http://www.embed-net.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_conf.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  使能SPI时钟
  * @retval None
  */
static void SPI_RCC_Configuration(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5,  GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
}
/**
  * @brief  配置指定SPI的引脚
  * @retval None
  */
static void SPI_GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	//PA4->CS,PA5->SCK,PA6->MISO,PA7->MOSI		 					 
	GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;  	
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_DOWN;  	
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	//初始化片选输出引脚
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}
/**
  * @brief  根据外部SPI设备配置SPI相关参数
  * @retval None
  */
void SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStruct;

	SPI_RCC_Configuration();
	SPI_GPIO_Configuration();
	
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStruct);
	
	SPI_Cmd(SPI1, ENABLE);
}
/**
  * @brief  写1字节数据到SPI总线
  * @param  TxData 写到总线的数据
  * @retval None
  */
void SPI_WriteByte(uint8_t TxData)
{				 
	while((SPI1->SR&SPI_I2S_FLAG_TXE)==0);	//等待发送区空		  
	SPI1->DR=TxData;	 	  									//发送一个byte 
	while((SPI1->SR&SPI_I2S_FLAG_RXNE)==0); //等待接收完一个byte  
	SPI1->DR;		
}
/**
  * @brief  从SPI总线读取1字节数据
  * @retval 读到的数据
  */
uint8_t SPI_ReadByte(void)
{			 
	while((SPI1->SR&SPI_I2S_FLAG_TXE)==0);	//等待发送区空			  
	SPI1->DR=0xFF;	 	  										//发送一个空数据产生输入数据的时钟 
	while((SPI1->SR&SPI_I2S_FLAG_RXNE)==0); //等待接收完一个byte  
	return SPI1->DR;  						    
}
/**
  * @brief  进入临界区
  * @retval None
  */
void SPI_CrisEnter(void)
{
	__set_PRIMASK(1);
}
/**
  * @brief  退出临界区
  * @retval None
  */
void SPI_CrisExit(void)
{
	__set_PRIMASK(0);
}

/**
  * @brief  片选信号输出低电平
  * @retval None
  */
void SPI_CS_Select(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
}
/**
  * @brief  片选信号输出高电平
  * @retval None
  */
void SPI_CS_Deselect(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}
/*********************************END OF FILE**********************************/

