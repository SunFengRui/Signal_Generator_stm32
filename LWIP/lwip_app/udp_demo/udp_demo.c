#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"  
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "socket.h"	


/*
              IP                    PORT
Matlab      Matlab_IP           Matlab_PORT
STM32      192.168.1.10    W5500_PORT(W5500)  L8720_PORT(L8720)
XiaWeiJi   192.168.1.16        XiaWeiJi_PORT
*/
uint16_t Matlab_PORT=1401;	//����udp���ӵ�Զ�˶˿� ����Ҫ������������
uint8_t Matlab_IP[]={192,168,1,108};  //������λ��Զ��IP��ַ  ����Ҫ������������������
extern uint8_t T_XiaWeiJi_A[150][13];
extern uint8_t T_XiaWeiJi_B[150][13];
extern int T_COUNT;
void udp_set_XiaWeiji_remoteip(void)  //������λ��Զ��IP��ַ  ����Ҫ������������������
{
	u8 *tbuf;
	tbuf=mymalloc(SRAMIN,100);	//�����ڴ�
	if(tbuf==NULL)return; 	
  lwipdev.remoteip[0]=192;
  lwipdev.remoteip[1]=168;
  lwipdev.remoteip[2]=1;
	lwipdev.remoteip[3]=16;  //�Լ�����Զ��IP
	myfree(SRAMIN,tbuf); 
}

//W5500��������
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd},
                            .ip = {192, 168, 1, 10},      //����������������
                            .sn = {255,255,255,0},
                            .gw = {192, 168, 1, 1},
                            .dns = {0,0,0,0},
                            .dhcp = NETINFO_STATIC };  //ʹ�þ�̬IP
char udp_matlab_recvbuf[UDP_DEMO_RX_BUFSIZE];	//Matlab�������ݻ��� ���յ���Ҫ�����ݽ��д���

//UDP ����ȫ��״̬��Ǳ���
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û��������;1,��������.
u8 udp_xiaweiji_flag;



//��λ��UDP����
int XiaWeiJi_Flag;
struct udp_pcb *udp_XiaWeiJi_pcb;  	//����һ��UDP���������ƿ�
void udp_XiaWeiJi_test(void)
{
 	err_t err;
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ 	
	u8 *tbuf;
	u8 res=0;		 	
	udp_set_XiaWeiji_remoteip();//������Զ��IP
	tbuf=mymalloc(SRAMIN,200);	//�����ڴ�
	if(tbuf==NULL)
		return;		//�ڴ�����ʧ����,ֱ���˳�
	udp_XiaWeiJi_pcb=udp_new();
	if(udp_XiaWeiJi_pcb)//�����ɹ�
	{ 
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);  //��Զ��IP�����32λ
		err=udp_connect(udp_XiaWeiJi_pcb,&rmtipaddr,XiaWeiJi_PORT);//UDP�ͻ������ӵ�ָ��IP��ַ�Ͷ˿ںŵķ�����  ����Զ��IP�Ͷ˿�
		if(err==ERR_OK)
		{
			err=udp_bind(udp_XiaWeiJi_pcb,IP_ADDR_ANY,L8720_PORT);//�󶨱���IP��ַ��˿ں�
			if(err==ERR_OK)	//�����
			{
				udp_recv(udp_XiaWeiJi_pcb,udp_XiaWeiJi_recv,NULL);//ע����ջص�����Ϊ udp_XiaWeiJi_recv()
				udp_xiaweiji_flag |= 1<<5;			//����Ѿ���������
			}else res=1;
		}else res=1;		
	}else res=1;
	if(res==0)    //   res==0ǰ�涼�ɹ���
   XiaWeiJi_Flag=1;
  else	
	XiaWeiJi_Flag=0;
		
	
//	udp_XiaWeiJi_connection_close(udp_XiaWeiJi_pcb);  //�Ͽ�����
//	myfree(SRAMIN,tbuf);
} 

//Matlab_UDP�������ݻص�����
void udp_XiaWeiJi_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//���յ���Ϊ�յ�����ʱ
	{
		memset(udp_matlab_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //���ݽ��ջ���������
		for(q=p;q!=NULL;q=q->next)  //����������pbuf����
		{
			//�ж�Ҫ������UDP_DEMO_RX_BUFSIZE�е������Ƿ����UDP_DEMO_RX_BUFSIZE��ʣ��ռ䣬�������
			//�Ļ���ֻ����UDP_DEMO_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) 
				memcpy(udp_matlab_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//��������
			else 
				memcpy(udp_matlab_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
		}
		upcb->remote_ip=*addr; 				//��¼Զ��������IP��ַ
		upcb->remote_port=port;  			//��¼Զ�������Ķ˿ں�
		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1 
		udp_xiaweiji_flag|=1<<6;	//��ǽ��յ�������
		pbuf_free(p);//�ͷ��ڴ�
	}else
	{
		udp_disconnect(upcb); 
		udp_xiaweiji_flag &= ~(1<<5);	//������ӶϿ�
	} 
} 
extern int A_Remain,B_Remain;
extern int Tranmit_Flag;
int Tranmit_Count=0;
//����λ��25us��������   �ж�����ִ��
void udp_XiaWeiJi_senddata(struct udp_pcb *upcb)     
{  
	struct pbuf *ptr;
	ptr=pbuf_alloc(PBUF_TRANSPORT,13,PBUF_POOL); //�����ڴ�  //��λΪ�ֽ�
	if(ptr)
	{ T_COUNT=Tranmit_Count%150;
		if(Tranmit_Count%150==0)
		{
			Tranmit_Flag=!Tranmit_Flag;
		}
		if(Tranmit_Flag)
		{
		ptr->payload=(void*)T_XiaWeiJi_B[T_COUNT]; //��tcp_matlab_sendbuf�е����ݴ����pbuf�ṹ��
		B_Remain--;
		}
		else
		{
		ptr->payload=(void*)T_XiaWeiJi_A[T_COUNT]; //��tcp_matlab_sendbuf�е����ݴ����pbuf�ṹ��
		A_Remain--;
		}
		Tranmit_Count++;
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

//W5500
void network_init(void)
{
  uint8_t tmpstr[6];
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
	ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
	// Display Network Information
	ctlwizchip(CW_GET_ID,(void*)tmpstr);
}





















