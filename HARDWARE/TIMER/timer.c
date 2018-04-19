#include "timer.h"
#include "led.h"
#include "lwip_comm.h"
#include "udp_demo.h" 
#include "socket.h"	


extern u32 lwip_localtime;	//lwip����ʱ�������,��λ:ms

void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///ʹ��TIM3ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=arr;   //�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}
void TIM5_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  ///ʹ��TIM3ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=arr;   //�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
  //TIM_Cmd(TIM5,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}
void TIM7_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);  ///ʹ��TIM3ʱ��
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=arr;   //�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM7,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE); //����ʱ��3�����ж�
  TIM_Cmd(TIM7,ENABLE); //ʹ�ܶ�ʱ��3
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM7_IRQn; //��ʱ��3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00; //��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01; //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}
//��ʱ��3�жϷ�����
int QQQ;
void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //����ж�
	{
		lwip_localtime +=10; //��10
	}
	QQQ++;
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //����жϱ�־λ
}
//��ʱ��4�жϷ�����
int T_COUNT=0;
int T_IRQ=-1;
int count=150;  //��Ҫ������������
extern struct udp_pcb *udp_XiaWeiJi_pcb;
int NNN;
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5,TIM_IT_Update)==SET) //����ж�
	{ 		
		udp_XiaWeiJi_senddata(udp_XiaWeiJi_pcb);   //�������� ����Ҫ������������
		T_COUNT++;  //�����㣬һֱ��
		if(T_COUNT>=149)
		T_COUNT=0;  
	}
	NNN++;
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);  //����жϱ�־λ
}
extern char T_DATABUF[];
extern uint16_t Matlab_PORT;
extern int R_COUNT;
extern uint8_t T_XiaWeiJi_A[150][13];
extern uint8_t T_XiaWeiJi_B[150][13];
extern uint8_t Matlab_IP[];
extern int A_Remain;
extern int B_Remain;
int i,j;
int MMM;
extern int Test;
void TIM7_IRQHandler(void)                           //��������
{ 
	if(TIM_GetITStatus(TIM7,TIM_IT_Update)==SET) 
	{ 		

		if(A_Remain==1)
		{	   
		sendto(SOCK_UDPS,T_DATABUF,1,Matlab_IP,Matlab_PORT);  
		for(i=0;i<150;i++)
		 {
			 while(Test!=13)
		  Test=recvfrom(SOCK_UDPS,T_XiaWeiJi_A[R_COUNT],13,Matlab_IP,&Matlab_PORT);	
			 		Test=0;
			  //if(T_XiaWeiJi_A[R_COUNT][12]!=T_XiaWeiJi_A[R_COUNT-1][12])				
			A_Remain++;
			// delay_us(5);
		 }
		}
		if(B_Remain==1)
		{     
		sendto(SOCK_UDPS,T_DATABUF,1,Matlab_IP,Matlab_PORT);
		for(j=0;j<150;j++)
		 {
			while(Test!=13)
		  Test=recvfrom(SOCK_UDPS,T_XiaWeiJi_B[R_COUNT],13,Matlab_IP,&Matlab_PORT);	
					Test=0;
			  //if(T_XiaWeiJi_B[R_COUNT][12]!=T_XiaWeiJi_B[R_COUNT-1][12])				
			B_Remain++;
			//delay_us(5);
		 }
		}
		MMM++;
	}
	TIM_ClearITPendingBit(TIM7,TIM_IT_Update);  //����жϱ�־λ
}
