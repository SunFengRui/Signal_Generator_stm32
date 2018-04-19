#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "usmart.h"
#include "timer.h"
#include "sram.h"
#include "malloc.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "udp_demo.h"
#include "spi.h"
#include "socket.h"	
#include "stm32f4xx_conf.h"
extern uint8_t Matlab_IP[];
extern wiz_NetInfo gWIZNETINFO;
extern uint16_t Matlab_PORT;
extern int R_COUNT;
extern int count,i,j;
uint8_t T_XiaWeiJi_A[150][13];
uint8_t T_XiaWeiJi_B[150][13];
// Default Network Configuration
int32_t ret = 0;	
//speed=LAN8720_Get_Speed();//�õ�����
uint8_t T_DATABUF[]={"1"};
int A_Remain=1;
int B_Remain=1;
int Tranmit_Flag=1;
int Flag;
int Test;
int main(void)
{
	uint8_t tmp;
	uint8_t memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};

	SPI_Configuration();            
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);	//ע���ٽ�������
#if   _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);//ע��SPIƬѡ�źź���
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  // CS must be tried with LOW.
#else
   #if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
      #error "Unknown _WIZCHIP_IO_MODE_"
   #else
      reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
   #endif
#endif
	reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);	//ע���д����

	/* WIZCHIP SOCKET Buffer initialize */
	if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1)
		{
		 while(1);
	  }

	/* PHY link status check */
	do{
		 if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
			 {
				printf("Unknown PHY Link stauts.\r\n");
		   }
	}while(tmp == PHY_LINK_OFF);
	network_init();	
	delay_init(168);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϷ�������
	uart_init(115200);   	//���ڲ���������
	usmart_dev.init(84); 	//��ʼ��USMART
	LED_Init();  			//LED��ʼ��
	KEY_Init();  			//������ʼ��
	FSMC_SRAM_Init();		//��ʼ���ⲿSRAM  	
	mymem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	mymem_init(SRAMEX);		//��ʼ���ⲿ�ڴ��
	mymem_init(SRAMCCM);	//��ʼ��CCM�ڴ��	
	
	while(lwip_comm_init()) 			//lwip��ʼ�� 
	{
		Flag=lwip_comm_init();
		delay_ms(1200);	  
	}
	delay_ms(500);			//��ʱ1s
	
	while(Flag==0)
	{
	Flag=getSn_SR(SOCK_UDPS);
	ret = socket(SOCK_UDPS,Sn_MR_UDP,W5500_PORT,0);//��socket0��һ���˿�    �ڶ���������UDP 5000Ϊ�˿ں�	
	}
	udp_XiaWeiJi_test();		
	delay_ms(500);			//��ʱ1s	
	
	while (A_Remain<151)
	{
		sendto(SOCK_UDPS,T_DATABUF,1,Matlab_IP,Matlab_PORT);//W5200���ؽ��յ������� 
		
		for(int i=0;i<150;i++)
		  { 		
				while(Test!=13)
				Test=recvfrom(SOCK_UDPS,T_XiaWeiJi_A[R_COUNT],13,Matlab_IP,&Matlab_PORT);	//W5200��������UDP������	
				
				Test=0;
        //if(T_XiaWeiJi_A[R_COUNT][12]!=T_XiaWeiJi_A[R_COUNT-1][12])				
		    A_Remain++;	
					//delay_us(5);
	    }
	}
		delay_ms(50);
		while (B_Remain<151)
	{
		sendto(SOCK_UDPS,T_DATABUF,1,Matlab_IP,Matlab_PORT);//W5200���ؽ��յ������� 
		for(int j=0;j<150;j++)
		  {
				while(Test!=13)
			 Test=recvfrom(SOCK_UDPS,T_XiaWeiJi_B[R_COUNT],13,Matlab_IP,&Matlab_PORT);	//W5200��������UDP������
				Test=0;
				 //if(T_XiaWeiJi_B[R_COUNT][12]!=T_XiaWeiJi_B[R_COUNT-1][12])				
		   B_Remain++;
				//delay_us(5);
		  }
		}
	  TIM3_Int_Init(999,839); //100khz��Ƶ��,����1000Ϊ10ms
		TIM5_Int_Init(50-1,84-1); //40khz��Ƶ��,����25Ϊ25us
		TIM7_Int_Init(1000-1,84-1); 
		TIM_Cmd(TIM5,ENABLE); //���ж�

	while(1)
	{ 
	   //lwip_periodic_handle();	
		 switch(getSn_SR(SOCK_UDPS))	//��ȡsocket0��״̬
		{
			//socket�ȴ��ر�״̬
			case SOCK_CLOSE_WAIT:
         if((ret=disconnect(SOCK_UDPS)) != SOCK_OK)
					 {
					 break;
				   }
         break;
			//socket�ر�
			case SOCK_CLOSED:	
			ret = socket(SOCK_UDPS,Sn_MR_UDP,W5500_PORT,0);//��socket0��һ���˿�    �ڶ���������UDP 5000Ϊ�˿ں�
			if(ret != SOCK_UDPS)  //0:Socket Error
					{				
					while(1);
				  }
			else       //Socket OK
					{
				  }
				break;
     }	

		
	}
}








