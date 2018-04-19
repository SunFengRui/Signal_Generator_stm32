#ifndef __UDP_DEMO_H
#define __UDP_DEMO_H
#include "sys.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"	   
 
#define SOCK_UDPS        0
#define T_DATA_BUF_SIZE   6

#define UDP_DEMO_RX_BUFSIZE		1	//����udp���������ݳ��� 

#define L8720_PORT			1200	//����udp���ӵ�Զ�˶˿� ����Ҫ������������
#define XiaWeiJi_PORT			1400	//����udp���ӵ�Զ�˶˿� ����Ҫ������������
#define W5500_PORT 8084

 
void udp_XiaWeiJi_test(void);

void udp_XiaWeiJi_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);

void udp_XiaWeiJi_senddata(struct udp_pcb *upcb);

void udp_XiaWeiJi_connection_close(struct udp_pcb *upcb);

void network_init(void);
#endif

