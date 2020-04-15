#include "sram.h"
#include "usart.h"
#include "delay.h"
//LAN8720
//使用NOR/SRAM的 Bank1.sector3,地址位HADDR[27,26]=10 
//对IS61LV25616/IS62WV25616,地址线范围为A0~A17 
//对IS61LV51216/IS62WV51216,地址线范围为A0~A18

//W5300
//使用NOR/SRAM的 Bank1.sector1,地址位HADDR[27,26]=00

#define Bank1_SRAM3_ADDR		(u32)(0x68000000)

/*
 +-------------------+--------------------+------------------+------------------+
 | PD0  <-> FSMC_D2  | PE0  <-> FSMC_NBL0 | PF0 <-> FSMC_A0  | PG0 <-> FSMC_A10 |
 | PD1  <-> FSMC_D3  | PE1  <-> FSMC_NBL1 | PF1 <-> FSMC_A1  | PG1 <-> FSMC_A11 |
 | PD4  <-> FSMC_NOE | PE2  <-> FSMC_A23  | PF2 <-> FSMC_A2  | PG2 <-> FSMC_A12 |
 | PD5  <-> FSMC_NWE | PE3  <-> FSMC_A19  | PF3 <-> FSMC_A3  | PG3 <-> FSMC_A13 |
 | PD8  <-> FSMC_D13 | PE4  <-> FSMC_A20  | PF4 <-> FSMC_A4  | PG4 <-> FSMC_A14 |
 | PD9  <-> FSMC_D14 | PE5  <-> FSMC_A21  | PF5 <-> FSMC_A5  | PG5 <-> FSMC_A15 |
 | PD10 <-> FSMC_D15 | PE6  <-> FSMC_A22  | PF12 <-> FSMC_A6 | PG9 <-> FSMC_NE2 |
 | PD11 <-> FSMC_A16 | PE7  <-> FSMC_D4   | PF13 <-> FSMC_A7 |------------------+
 | PD12 <-> FSMC_A17 | PE8  <-> FSMC_D5   | PF14 <-> FSMC_A8 |
 | PD13 <-> FSMC_A18 | PE9  <-> FSMC_D6   | PF15 <-> FSMC_A9 |
 | PD14 <-> FSMC_D0  | PE10 <-> FSMC_D7   |------------------+
 | PD15 <-> FSMC_D1  | PE11 <-> FSMC_D8   |
 +-------------------| PE12 <-> FSMC_D9   |
                     | PE13 <-> FSMC_D10  |
                     | PE14 <-> FSMC_D11  |
                     | PE15 <-> FSMC_D12  |
                     +--------------------+
*/
//初始化外部SRAM
void FSMC_SRAM_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	NVIC_InitTypeDef NVIC_InitStructure;
	FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure_5300;
	FSMC_NORSRAMTimingInitTypeDef FSMC_ReadTimingStructure_5300;
	FSMC_NORSRAMTimingInitTypeDef FSMC_WRITETimingStructure_5300;
	
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG,ENABLE); //使能PD,PE,PF,PG时钟
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);//使能FSMC时钟 

  
	
	NVIC_InitStructure.NVIC_IRQChannel=FSMC_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//FSMC_ITConfig(FSMC_Bank1_NORSRAM1, FSMC_IT_RisingEdge | FSMC_IT_FallingEdge | FSMC_IT_Level, ENABLE);
	FSMC_ITConfig(FSMC_Bank1_NORSRAM1, FSMC_IT_RisingEdge, ENABLE);

	/* SRAM Data lines configuration */
	
	GPIO_InitStructure.GPIO_Pin =( GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10 \
																|GPIO_Pin_14|GPIO_Pin_15); //PD0,1,8,9,10,14,15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //复用模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //100M时钟
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  //推完模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOD,&GPIO_InitStructure);
		
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11 \
																|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15); //PE0,1,7,8,9,10,11,12,13,14,15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用输出
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
  /* SRAM Address lines configuration */
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//推挽输出
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_12 \
																|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15); //PF0,1,2,3,4,5,12,13,14,15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  //推完模式	
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7 ; //PG0,1,2,3,4,5,6,7
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Pin =( GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13); //PD0,1,4,5,8,9,10,11,12,13,14,15
//	GPIO_Init(GPIOD,&GPIO_InitStructure);
	
	/* NOE and NWE configuration */  
	GPIO_InitStructure.GPIO_Pin =( GPIO_Pin_4|GPIO_Pin_5); //PD0,1,4,5,8,9,10,11,12,13,14,15
	GPIO_Init(GPIOD,&GPIO_InitStructure);
	
	/* NE3 configuration */
	//1区   该线连接LAN8720片选
//	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_10 ; //PG0,1,2,3,4,5,10
//	GPIO_Init(GPIOG,&GPIO_InitStructure);
	/* NE1 configuration */
	//1区   该线连接W5300片选
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;   //FSMC_NE1
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/*****************/
//	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_0|GPIO_Pin_1); //PE0,1,7,8,9,10,11,12,13,14,15
//	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	//GPIOD复用配置
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);  //PD0
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);  //PD1
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);  //PD4
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC);  //PD5
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource7,GPIO_AF_FSMC);  //PD7
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC);  //PD8
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);  //PD9
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC); //PD10
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_FSMC); //PD11
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_FSMC); //PD12
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_FSMC); //PD13
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC); //PD14
	GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC); //PD15
	
	//GPIOE复用配置
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource0,GPIO_AF_FSMC);  //PE0
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource1,GPIO_AF_FSMC);  //PE1
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);  //PE7
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);  //PE8
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);  //PE9
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC); //PE10
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC); //PE11
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC); //PE12
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC); //PE13
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC); //PE14
	GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC); //PE15
	
	//GPIOF复用配置
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource0,GPIO_AF_FSMC);  //PF0
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource1,GPIO_AF_FSMC);  //PF1
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource2,GPIO_AF_FSMC);  //PF2
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource3,GPIO_AF_FSMC);  //PF3
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource4,GPIO_AF_FSMC);  //PF4
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource5,GPIO_AF_FSMC);  //PF5
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource12,GPIO_AF_FSMC); //PF12
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource13,GPIO_AF_FSMC); //PF13
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource14,GPIO_AF_FSMC); //PF14
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource15,GPIO_AF_FSMC); //PF15
	
	//GPIOG复用配置
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource0,GPIO_AF_FSMC);  //PG0
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource1,GPIO_AF_FSMC);  //PG1
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource2,GPIO_AF_FSMC);  //PG2
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource3,GPIO_AF_FSMC);  //PG3
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource4,GPIO_AF_FSMC);  //PG4
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource5,GPIO_AF_FSMC);  //PG5
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource6,GPIO_AF_FSMC);  //PG6
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource7,GPIO_AF_FSMC);  //PG7
	GPIO_PinAFConfig(GPIOG,GPIO_PinSource10,GPIO_AF_FSMC); //PG10
	
	
	FSMC_ReadTimingStructure_5300.FSMC_AddressSetupTime = 0X0f; //地址建立时间0个HCLK 0ns
	FSMC_ReadTimingStructure_5300.FSMC_AddressHoldTime = 0x00;  //地址保持时间,模式A未用到
	FSMC_ReadTimingStructure_5300.FSMC_DataSetupTime = 0x08;    //数据保持时间,9个HCLK，6*9=54ns
	FSMC_ReadTimingStructure_5300.FSMC_BusTurnAroundDuration = 0x00;
	FSMC_ReadTimingStructure_5300.FSMC_CLKDivision = 0x00;
	FSMC_ReadTimingStructure_5300.FSMC_DataLatency = 0x00;
	FSMC_ReadTimingStructure_5300.FSMC_AccessMode = FSMC_AccessMode_A; //模式A
	
	FSMC_WRITETimingStructure_5300.FSMC_AddressSetupTime = 0X08; //地址建立时间0个HCLK 0ns
	FSMC_WRITETimingStructure_5300.FSMC_AddressHoldTime = 0x00;  //地址保持时间,模式A未用到
	FSMC_WRITETimingStructure_5300.FSMC_DataSetupTime = 0x08;    //数据保持时间,9个HCLK，6*9=54ns
	FSMC_WRITETimingStructure_5300.FSMC_BusTurnAroundDuration = 0x00;
	FSMC_WRITETimingStructure_5300.FSMC_CLKDivision = 0x00;
	FSMC_WRITETimingStructure_5300.FSMC_DataLatency = 0x00;
	FSMC_WRITETimingStructure_5300.FSMC_AccessMode = FSMC_AccessMode_A; //模式A
	
	
	
	FSMC_NORSRAMInitStructure_5300.FSMC_Bank = FSMC_Bank1_NORSRAM1;  //NOR/SRAM的Bank1
	FSMC_NORSRAMInitStructure_5300.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; //数据和地址线不复用
	FSMC_NORSRAMInitStructure_5300.FSMC_MemoryType = FSMC_MemoryType_SRAM; //SRAM存储模式
	FSMC_NORSRAMInitStructure_5300.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b; //16位数据宽度
	FSMC_NORSRAMInitStructure_5300.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable; //FLASH使用的,SRAM未使用
	FSMC_NORSRAMInitStructure_5300.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable; //是否使能同步传输模式下的等待信号,此处未用到
	FSMC_NORSRAMInitStructure_5300.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low; //等待信号的极性,仅在突发模式访问下有用
	FSMC_NORSRAMInitStructure_5300.FSMC_WrapMode = FSMC_WrapMode_Disable;  //是否使能环路突发模式,此处未用到
	FSMC_NORSRAMInitStructure_5300.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState; //存储器是在等待周期之前的一个时钟周期还是等待周期期间使能NWAIT
	FSMC_NORSRAMInitStructure_5300.FSMC_WriteOperation = FSMC_WriteOperation_Enable; //存储器写使能
	FSMC_NORSRAMInitStructure_5300.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   //等待使能位,此处未用到
	FSMC_NORSRAMInitStructure_5300.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; //读写使用相同的时序
	FSMC_NORSRAMInitStructure_5300.FSMC_WriteBurst = FSMC_WriteBurst_Disable;  //异步传输期间的等待信号
	FSMC_NORSRAMInitStructure_5300.FSMC_ReadWriteTimingStruct = &FSMC_ReadTimingStructure_5300;
	FSMC_NORSRAMInitStructure_5300.FSMC_WriteTimingStruct = &FSMC_WRITETimingStructure_5300;
	
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure_5300);  //FSMC_SRAM初始化
	
	//FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3,ENABLE);  //使能NOR/SRAM功能
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1,ENABLE);  //使能NOR/SRAM功能
}
u8 flag1,flag2,flag3;
void FSMC_IRQHandler(void)
{
	if (FSMC_GetITStatus(FSMC_Bank1_NORSRAM1, FSMC_IT_RisingEdge) == SET)
	{
		FSMC_ClearITPendingBit(FSMC_Bank1_NORSRAM1, FSMC_IT_RisingEdge);
		flag1=1;
	}
//	if (FSMC_GetITStatus(FSMC_Bank1_NORSRAM1, FSMC_IT_FallingEdge) == SET)
//	{
//		FSMC_ClearITPendingBit(FSMC_Bank1_NORSRAM1, FSMC_IT_FallingEdge);
//		flag2=1;
//	}
//	
//	if (FSMC_GetITStatus(FSMC_Bank1_NORSRAM1, FSMC_IT_Level) == SET)
//	{
//		FSMC_ClearITPendingBit(FSMC_Bank1_NORSRAM1, FSMC_IT_Level);
//		flag3=1;
//	}

}


