#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"  
#include <stdio.h>
#include <stdlib.h>
#include "processw5300.h"

/*
              IP                    PORT
Matlab      Matlab_IP  108           Matlab_PORT
STM32      192.168.1.10    W5300_PORT(W5300)  L8720_PORT(L8720)
XiaWeiJi   192.168.1.16        XiaWeiJi_PORT
*/
uint16_t Matlab_PORT=1401;	//����udp���ӵ�Զ�˶˿� ����Ҫ������������  matlab�����źŶ˿�
uint8_t Matlab_IP[]={192,168,1,108};  //������λ��Զ��IP��ַ  ����Ҫ������������������

//UDP ����ȫ��״̬��Ǳ���
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û��������;1,��������.
u8 udp_xiaweiji_flag;


//��λ��UDP����
struct udp_pcb *udp_XiaWeiJi_pcb;  	//����һ��UDP���������ƿ�
void udp_XiaWeiJi_test(void)
{
 	err_t err;
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ 	

	udp_XiaWeiJi_pcb=udp_new();
	if(udp_XiaWeiJi_pcb)//�����ɹ�
	{ 
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);  //��Զ��IP�����32λ
		err=udp_connect(udp_XiaWeiJi_pcb,&rmtipaddr,Matlab_PORT);//UDP�ͻ������ӵ�ָ��IP��ַ�Ͷ˿ںŵķ�����  ����Զ��IP�Ͷ˿�
		if(err==ERR_OK)
		{
			err=udp_bind(udp_XiaWeiJi_pcb,IP_ADDR_ANY,L8720_PORT);//�󶨱���IP��ַ��˿ں�
			if(err==ERR_OK)	//�����
			{
				udp_recv(udp_XiaWeiJi_pcb,udp_XiaWeiJi_recv,NULL);//ע����ջص�����Ϊ udp_XiaWeiJi_recv()
				udp_xiaweiji_flag |= 1<<5;			//����Ѿ���������
			}
		}		
	}
} 
extern int A_Remain,B_Remain,C_Remain,D_Remain;
extern int A2_Remain,B2_Remain,C2_Remain,D2_Remain;
extern uint8_t T_XiaWeiJi_A[daqu_size];
extern uint8_t T_XiaWeiJi_B[daqu_size];
extern uint8_t T_XiaWeiJi_C[daqu_size];
extern uint8_t T_XiaWeiJi_D[daqu_size];


extern uint8_t T_XiaWeiJi_A2[daqu_size];
extern uint8_t T_XiaWeiJi_B2[daqu_size];
extern uint8_t T_XiaWeiJi_C2[daqu_size];
extern uint8_t T_XiaWeiJi_D2[daqu_size];


extern u8 traning1,traning2;
int receive_length;
u32 data_len = 0;
u8 receivefault;
void udp_XiaWeiJi_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
struct pbuf *q;
    LED1=0;
	switch(traning1%4)
	{
		case 0:
				 for(q=p;q!=NULL;q=q->next)  //����������pbuf����
				 {	
						memcpy(T_XiaWeiJi_A+data_len,q->payload,q->len);
						data_len += q->len;  		
				 }
					receive_length=data_len;
					if(receive_length==Receive_Sum_Count)
					{
					 A_Remain+=daqu_dataPacket_size;
					 B_Remain+=daqu_dataPacket_size;
					
						data_len=0;
						traning2=0;
						receive_length=0;
						receivefault=0;
					}
					else
						receivefault=1;
		break;
		case 1:
				for(q=p;q!=NULL;q=q->next)  //����������pbuf����
					{				
							memcpy(T_XiaWeiJi_C+data_len,q->payload,q->len);
							data_len += q->len;  							
						}
			 receive_length=data_len;
				
					if(receive_length==Receive_Sum_Count)
					{ 
					  C_Remain+=daqu_dataPacket_size;
					 D_Remain+=daqu_dataPacket_size;
					 data_len=0;
						traning2=0; 
						receive_length=0;
						receivefault=0;
					}
					else
						receivefault=1;
		break;
		case 2:
				 for(q=p;q!=NULL;q=q->next)  //����������pbuf����
						{				
								memcpy(T_XiaWeiJi_A2+data_len,q->payload,q->len);
								data_len += q->len;  		
							}
						 receive_length=data_len;
							
								if(receive_length==Receive_Sum_Count)
								{
								 A2_Remain+=daqu_dataPacket_size;
								 B2_Remain+=daqu_dataPacket_size;
								
								 data_len=0;
									traning2=0; 
									receive_length=0;
									receivefault=0;
								}
								else
						receivefault=1;
		break;
		case 3:
				 for(q=p;q!=NULL;q=q->next)  //����������pbuf����
					{				
							memcpy(T_XiaWeiJi_C2+data_len,q->payload,q->len);
							data_len += q->len;  				
						}
					 receive_length=data_len;	
							if(receive_length==Receive_Sum_Count)
							{
							C2_Remain+=daqu_dataPacket_size;
							D2_Remain+=daqu_dataPacket_size;
							data_len=0;
							traning2=0; 
							receive_length=0;
							receivefault=0;
							}
							else
						receivefault=1;
		break;
		default:
		break;
	}
		pbuf_free(p);//�ͷ��ڴ�
} 

extern uint8_t T_DATABUF[];
void udp_XiaWeiJi_senddata(struct udp_pcb *upcb)     
{  
	struct pbuf *ptr;
	ptr=pbuf_alloc(PBUF_TRANSPORT,1,PBUF_POOL); //�����ڴ�  //��λΪ�ֽ�
	if(ptr)
	{	
		ptr->payload=(void*)(T_DATABUF); //��tcp_matlab_sendbuf�е����ݴ����pbuf�ṹ��
		udp_send(upcb,ptr);	//udp�������� 

		pbuf_free(ptr);//�ͷ��ڴ�
	} 
}

//�ر�Matlab_UDP����
void udp_XiaWeiJi_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);			//�Ͽ�UDP���� 
	udp_xiaweiji_flag &= ~(1<<5);	//������ӶϿ�	
}























