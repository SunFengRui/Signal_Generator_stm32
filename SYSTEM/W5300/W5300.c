/***********************************************************************************
成都浩然电子有限公司
电话：028-86127089，400-998-5300
传真：028-86127039
网址：http://www.hschip.com.cn
***********************************************************************************/
#include "sys.h"
#include "W5300.h"              /* STM32F10x库 */

/* W5300中断输入口定义 */
#define W5300_INT	GPIO_Pin_0

/* 对W5300复位信号输出口定义 */
#define W5300_RST	GPIO_Pin_1

/********************** PORTF口的定义 **********************/
/* 数字信号输入口定义 */
#define D_INPUT1	GPIO_Pin_6
#define D_INPUT2	GPIO_Pin_7

/* 数字信号输出口定义 */
#define D_OUTPUT1	GPIO_Pin_8
#define D_OUTPUT2	GPIO_Pin_9

/********************** PORTG口的定义 **********************/
/* 以太网连接状态输入口定义 */
#define L_LINK	GPIO_Pin_15

unsigned int Timer2_Counter;
unsigned int S0_Recv, S0_SendOK, S0_TimeOut;

extern void System_Initialization(void);

unsigned short S_Buffer[800];

unsigned short S0_Port=5000;

unsigned int Delay_Connect;

#define TRUE	0xffffffff
#define FALSE	0

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

int RCC_Configuration(void)
{
	ErrorStatus		HSEStartUpStatus;

  	/* RCC system reset(for debug purpose) */
  	RCC_DeInit();

  	/* Enable HSE */
  	RCC_HSEConfig(RCC_HSE_ON);

  	/* Wait till HSE is ready */
  	HSEStartUpStatus = RCC_WaitForHSEStartUp();

  	if(HSEStartUpStatus == SUCCESS)
  	{
	    /* HCLK = SYSCLK */
    	RCC_HCLKConfig(RCC_SYSCLK_Div1);

	    /* PCLK2 = HCLK */
    	RCC_PCLK2Config(RCC_HCLK_Div1);

	    /* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);

	    /* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);

	    /* Enable Prefetch Buffer */
    	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	    /* PLLCLK = 8MHz * 9 / 1 = 72 MHz */
   		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

	    /* Enable PLL */
    	RCC_PLLCmd(ENABLE);

	    /* Wait till PLL is ready */
    	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET){};

	    /* Select PLL as system clock source */
    	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

	    /* Wait till PLL is used as system clock source */
    	while(RCC_GetSYSCLKSource() != 0x08){};
  	}
	else
		return FALSE;

	/* Enable peripheral clocks --------------------------------------------------*/
  	/* Enable TIM2 clock */
 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  	/* Enable GPIOA/B/C/D/E/F/G and USART1 clock */
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
					| RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD
					| RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF
					| RCC_APB2Periph_GPIOG
					| RCC_APB2Periph_USART1
					| RCC_APB2Periph_AFIO, ENABLE);

	/* High Speed Peripheral */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC | RCC_AHBPeriph_SRAM, ENABLE);

	return TRUE;
}

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures the nested vectored interrupt controller.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
  	NVIC_InitTypeDef	NVIC_InitStructure;

  	/* Set the Vector Table base location at 0x08000000 */
  	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

 	/* Enable the TIM2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the EXTI0 Interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQChannel;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

 	/* Enable the USART1 Interrupt */
  	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
}

/*    USART Initialization    */
void UART_Configuration(void)
{
	USART_InitTypeDef	USART_InitStructure;
  	GPIO_InitTypeDef 	GPIO_InitStructure;

  	/* Configure USART1 Tx (PA9) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

  	/* Configure USART1 Rx (PA10) as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate=9600;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Tx|USART_Mode_Rx;

	USART_Init(USART1,&USART_InitStructure);

/* Enable the USART Transmoit interrupt: this interrupt is generated when the
   USART1 transmit data register is empty */
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);

/* Enable the USART Receive interrupt: this interrupt is generated when the
   USART1 receive data register is not empty */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART1,ENABLE);				//Enable USART1
}

/* Timer2 interrupt every 1ms */
void Timer_Configuration(void)
{
	/* Time base configuration */
	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;

	TIM_TimeBaseStructure.TIM_Period = 36000;
	TIM_TimeBaseStructure.TIM_Prescaler = 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* TIM enable counter */
	TIM_Cmd(TIM2, ENABLE);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE );
}

/* IO port Configuration */
void IO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
//	EXTI_InitTypeDef	EXTI_InitStructure;

	/* Define Reset W5300 output */
	GPIO_InitStructure.GPIO_Pin  = W5300_RST;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOE, W5300_RST);	/* Output W5300 Reset */

	/* define Digital Input Port */
	GPIO_InitStructure.GPIO_Pin  = (D_INPUT1|D_INPUT2);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* define Digital Output Port */
	GPIO_InitStructure.GPIO_Pin  = (D_OUTPUT1|D_OUTPUT2);
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* Turn off LED */
	GPIO_SetBits(GPIOF, (D_OUTPUT1|D_OUTPUT2));

	/* Define W5300 Ethernet Link state input */
	GPIO_InitStructure.GPIO_Pin  = L_LINK;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* Configure PE0 as input floating (EXTI Line0) */
//	GPIO_InitStructure.GPIO_Pin = W5300_INT;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Connect EXTI Line0 to PE0 */
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource0);

	/* PE0 as W5300 interrupt input */
//	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);
}

/*******************************************************************************
* Function Name  : FSMC_SRAM_Init
* Description    : Configures the FSMC and GPIOs to interface with the SRAM memory.
*                  This function must be called before any write/read operation
*                  on the SRAM.
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/
void FSMC_SRAM_Init(void)
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;
	GPIO_InitTypeDef GPIO_InitStructure; 

	/*-- GPIO Configuration ------------------------------------------------------*/
  	/* SRAM Data lines configuration */
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 |
									GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
									GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
									GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
 
	/* SRAM Address lines configuration */
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
									GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 | 
									GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
									GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* NOE and NWE configuration */  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 |GPIO_Pin_5;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* NE3 configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/*-- FSMC Configuration ------------------------------------------------------*/
	p.FSMC_AddressSetupTime = 1;
	p.FSMC_AddressHoldTime = 1;
	p.FSMC_DataSetupTime = 2;
	p.FSMC_BusTurnAroundDuration = 0;
	p.FSMC_CLKDivision = 0;
	p.FSMC_DataLatency = 0;
	p.FSMC_AccessMode = FSMC_AccessMode_A;

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsyncWait = FSMC_AsyncWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

	/* Enable FSMC Bank1_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);  
}

/******************** STM32F10x Configuration ***********************/
void System_Initialization(void)
{
 	/* System Clocks Configuration */
  	while(RCC_Configuration()==FALSE);

  	/* NVIC configuration */
  	NVIC_Configuration();

	/* IO Configuration */
	IO_Configuration();

	/* FSMC Configuration */
	FSMC_SRAM_Init();

	/* UART Configuration */
	UART_Configuration();

	/* Timer configuration */
	Timer_Configuration();
}
