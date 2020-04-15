#include "sys.h"
#include "delay.h"
#include "led.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "timer.h"
#include "sram.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "w5300.h"
#include "processw5300.h"
#include "udp_demo.h"

uint8_t question;

uint8_t T_XiaWeiJi_A[daqu_size];
uint8_t T_XiaWeiJi_B[daqu_size];
uint8_t T_XiaWeiJi_C[daqu_size];
uint8_t T_XiaWeiJi_D[daqu_size];

uint8_t T_XiaWeiJi_A2[daqu_size];
uint8_t T_XiaWeiJi_B2[daqu_size];
uint8_t T_XiaWeiJi_C2[daqu_size];
uint8_t T_XiaWeiJi_D2[daqu_size];


uint8_t T_DATABUF[]={"1"};
int A_Remain=1,B_Remain=1,C_Remain=1,D_Remain=1;
int A2_Remain=1,B2_Remain=1,C2_Remain=1,D2_Remain=1;
int Tranmit_Flag;
unsigned short state;
extern struct udp_pcb *udp_XiaWeiJi_pcb;
u8 Lan8720speed;
extern u8 traning1,traning2;
long count1;
int tx_size_16;
int main(void)
{
	delay_init(168);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断分组配置
	LED_Init();  			//LED初始化
	
	reset5300();
	FSMC_SRAM_Init();		//5300 
	
  W5300_Config();	
	
	while(lwip_comm_init()) 			//lwip初始化 
	{
		delay_ms(1200);	  
	}
  TIM3_Int_Init(999,839); //100khz的频率,计数1000为10ms
  Socket0_UDP();//打开一个端口
	
	udp_XiaWeiJi_test();
 tx_size_16=((double)tx_size)/2+0.5;	
/*****************************************************************************/	
udp_XiaWeiJi_senddata(udp_XiaWeiJi_pcb); 
traning2=1;
while(!(B_Remain==integer101))
{
	delay_ms(10);
}

udp_XiaWeiJi_senddata(udp_XiaWeiJi_pcb);
traning1++;     //1
traning2=1;
while(!(D_Remain==integer101))
{
	delay_ms(10);
}

udp_XiaWeiJi_senddata(udp_XiaWeiJi_pcb);
traning1++;     //2
traning2=1;
while(!(B2_Remain==integer101))
{
	delay_ms(10);
}

udp_XiaWeiJi_senddata(udp_XiaWeiJi_pcb);
traning1++;   //3
traning2=1;
while(!(D2_Remain==integer101))
{
	delay_ms(10);
}
/*****************************************************************************/	
TIM5_Int_Init(200-1,84-1); //40khz的频率,计数25为25us   发送数据
TIM_Cmd(TIM5,ENABLE); //开中断



	while(1)
	{ 		
		state = Detect_State();
		switch(state){
			case SOCK_CLOSED:
				Socket0_UDP();
			break;
			case SOCK_UDP:
				
			break;
			default: ;
		}
			
		count1++;
		if(count1==1000)
		{
			question=0;
			count1=0;	
			LED1=1;
		}
		
		if(question)
			LED0=0;
		else
			LED0=1;
		
		//Lan8720speed=LAN8720_Get_Speed();//得到网速
	}
}



