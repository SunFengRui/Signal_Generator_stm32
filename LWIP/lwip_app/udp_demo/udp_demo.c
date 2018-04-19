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
uint16_t Matlab_PORT=1401;	//定义udp连接的远端端口 很重要。。。。。。
uint8_t Matlab_IP[]={192,168,1,108};  //设置上位机远端IP地址  很重要。。。。。。。。。
extern uint8_t T_XiaWeiJi_A[150][13];
extern uint8_t T_XiaWeiJi_B[150][13];
extern int T_COUNT;
void udp_set_XiaWeiji_remoteip(void)  //设置下位机远端IP地址  很重要。。。。。。。。。
{
	u8 *tbuf;
	tbuf=mymalloc(SRAMIN,100);	//申请内存
	if(tbuf==NULL)return; 	
  lwipdev.remoteip[0]=192;
  lwipdev.remoteip[1]=168;
  lwipdev.remoteip[2]=1;
	lwipdev.remoteip[3]=16;  //自己设置远端IP
	myfree(SRAMIN,tbuf); 
}

//W5500网络设置
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc,0x00, 0xab, 0xcd},
                            .ip = {192, 168, 1, 10},      //。。。。。。。。
                            .sn = {255,255,255,0},
                            .gw = {192, 168, 1, 1},
                            .dns = {0,0,0,0},
                            .dhcp = NETINFO_STATIC };  //使用静态IP
char udp_matlab_recvbuf[UDP_DEMO_RX_BUFSIZE];	//Matlab接收数据缓冲 接收到后要对数据进行处理

//UDP 测试全局状态标记变量
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有连接上;1,连接上了.
u8 udp_xiaweiji_flag;



//下位机UDP测试
int XiaWeiJi_Flag;
struct udp_pcb *udp_XiaWeiJi_pcb;  	//定义一个UDP服务器控制块
void udp_XiaWeiJi_test(void)
{
 	err_t err;
	struct ip_addr rmtipaddr;  	//远端ip地址 	
	u8 *tbuf;
	u8 res=0;		 	
	udp_set_XiaWeiji_remoteip();//先设置远端IP
	tbuf=mymalloc(SRAMIN,200);	//申请内存
	if(tbuf==NULL)
		return;		//内存申请失败了,直接退出
	udp_XiaWeiJi_pcb=udp_new();
	if(udp_XiaWeiJi_pcb)//创建成功
	{ 
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);  //将远端IP打包成32位
		err=udp_connect(udp_XiaWeiJi_pcb,&rmtipaddr,XiaWeiJi_PORT);//UDP客户端连接到指定IP地址和端口号的服务器  设置远端IP和端口
		if(err==ERR_OK)
		{
			err=udp_bind(udp_XiaWeiJi_pcb,IP_ADDR_ANY,L8720_PORT);//绑定本地IP地址与端口号
			if(err==ERR_OK)	//绑定完成
			{
				udp_recv(udp_XiaWeiJi_pcb,udp_XiaWeiJi_recv,NULL);//注册接收回调函数为 udp_XiaWeiJi_recv()
				udp_xiaweiji_flag |= 1<<5;			//标记已经连接上了
			}else res=1;
		}else res=1;		
	}else res=1;
	if(res==0)    //   res==0前面都成功了
   XiaWeiJi_Flag=1;
  else	
	XiaWeiJi_Flag=0;
		
	
//	udp_XiaWeiJi_connection_close(udp_XiaWeiJi_pcb);  //断开连接
//	myfree(SRAMIN,tbuf);
} 

//Matlab_UDP接收数据回调函数
void udp_XiaWeiJi_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//接收到不为空的数据时
	{
		memset(udp_matlab_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //数据接收缓冲区清零
		for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
		{
			//判断要拷贝到UDP_DEMO_RX_BUFSIZE中的数据是否大于UDP_DEMO_RX_BUFSIZE的剩余空间，如果大于
			//的话就只拷贝UDP_DEMO_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) 
				memcpy(udp_matlab_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//拷贝数据
			else 
				memcpy(udp_matlab_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
		}
		upcb->remote_ip=*addr; 				//记录远程主机的IP地址
		upcb->remote_port=port;  			//记录远程主机的端口号
		lwipdev.remoteip[0]=upcb->remote_ip.addr&0xff; 		//IADDR4
		lwipdev.remoteip[1]=(upcb->remote_ip.addr>>8)&0xff; //IADDR3
		lwipdev.remoteip[2]=(upcb->remote_ip.addr>>16)&0xff;//IADDR2
		lwipdev.remoteip[3]=(upcb->remote_ip.addr>>24)&0xff;//IADDR1 
		udp_xiaweiji_flag|=1<<6;	//标记接收到数据了
		pbuf_free(p);//释放内存
	}else
	{
		udp_disconnect(upcb); 
		udp_xiaweiji_flag &= ~(1<<5);	//标记连接断开
	} 
} 
extern int A_Remain,B_Remain;
extern int Tranmit_Flag;
int Tranmit_Count=0;
//向下位机25us发送数据   中断里面执行
void udp_XiaWeiJi_senddata(struct udp_pcb *upcb)     
{  
	struct pbuf *ptr;
	ptr=pbuf_alloc(PBUF_TRANSPORT,13,PBUF_POOL); //申请内存  //单位为字节
	if(ptr)
	{ T_COUNT=Tranmit_Count%150;
		if(Tranmit_Count%150==0)
		{
			Tranmit_Flag=!Tranmit_Flag;
		}
		if(Tranmit_Flag)
		{
		ptr->payload=(void*)T_XiaWeiJi_B[T_COUNT]; //将tcp_matlab_sendbuf中的数据打包进pbuf结构中
		B_Remain--;
		}
		else
		{
		ptr->payload=(void*)T_XiaWeiJi_A[T_COUNT]; //将tcp_matlab_sendbuf中的数据打包进pbuf结构中
		A_Remain--;
		}
		Tranmit_Count++;
		udp_send(upcb,ptr);	//udp发送数据 
		pbuf_free(ptr);//释放内存
	} 
} 
//关闭Matlab_UDP连接
void udp_XiaWeiJi_connection_close(struct udp_pcb *upcb)
{
	udp_disconnect(upcb); 
	udp_remove(upcb);			//断开UDP连接 
	udp_xiaweiji_flag &= ~(1<<5);	//标记连接断开	
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





















