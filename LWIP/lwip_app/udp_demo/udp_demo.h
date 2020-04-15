#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "sys.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"	   
 
#define BIGBUFFER 8

#define L8720_PORT			1200	
#define XiaWeiJi_PORT			43981	//定义udp连接的远端端口 很重要。。。。。。
#define W5300_PORT 8084
#define PondBuffer 10 

/******************************************************************************************/
#define accuracy16  //accuracy16 accuracy18_20 

#ifdef accuracy16
#define daqu_size Receive_Sum_Count/2
#define Receive_Sum_Count 11200
#define daqu_dataPacket_size daqu_size/tx_buffer_interval
#define integer101 daqu_dataPacket_size+1
#define tx_size 14
#define tx_buffer_interval tx_size 
#endif

#ifdef accuracy18_20
#define daqu_size Receive_Sum_Count/2
#define Receive_Sum_Count 13000
#define daqu_dataPacket_size daqu_size/tx_buffer_interval
#define integer101 daqu_dataPacket_size+1
#define tx_size 26
#define tx_buffer_interval tx_size
#endif



  
		
		
void udp_XiaWeiJi_test(void);

void udp_XiaWeiJi_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);

void udp_XiaWeiJi_senddata(struct udp_pcb *upcb);
void udp_XiaWeiJi_connection_close(struct udp_pcb *upcb);

void network_init(void);
#endif

