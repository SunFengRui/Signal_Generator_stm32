#include "processw5300.h"
#include "stm32f4xx_conf.h"
#include "w5300.h"
#include "delay.h"
#include "udp_demo.h" 
#include "led.h"
#include "malloc.h"
/*******************************************/
#define TRUE	0xffffffff
#define FALSE	0

/*W5300使用的变量定义*/
unsigned int S0_SendOK;
unsigned short UDP_Preamble[4];
uint16_t dest_port;
uint16_t pack_size;
/*【配置网络参数】 */
uint8_t SIP[4]         = {192, 168, 1, 10};   //Source      IP Address 
uint8_t Gateway[4]     = {192,168,1,1};			   //Gateway Address
uint8_t Subnet[4]      = {255,255,255,0};			 //Subnet Address
uint8_t MAC[6]         = {0x00,0x08,0xDC,0x11,0x22,0x86};	//Source MAC Address

uint16_t MaxSegSize    = 1500;


/****************************************/
/* W5300 configuration */
void W5300_Config(void)
{
	unsigned short *ptr;

	//W5300 software reset, only clear registers
	ptr=(unsigned short*)(W5300_ADDRESS | MR);    //0x60000000+0  bank1 1区 首地址
	*ptr = MR_RST;
	delay_ms(100);
//直接访问
	/* Set Gateway address as "192.168.0.1" */
	ptr=(unsigned short*)(W5300_ADDRESS | GAR<<1);						/* Set Gateway IP */
	*ptr = ((uint16_t)Gateway[0]<<8)+(uint16_t)Gateway[1];
	ptr += 2;
	*ptr = ((uint16_t)Gateway[2]<<8)+(uint16_t)Gateway[3];

	/* Set IP address as "192.168.0.20" */
	ptr=(unsigned short*)(W5300_ADDRESS | SIPR<<1);
	*ptr = ((uint16_t)SIP[0]<<8)+(uint16_t)SIP[1];
	ptr += 2;
	*ptr = ((uint16_t)SIP[2]<<8)+(uint16_t)SIP[3];

	/* Set Subnet mask as "255.255.255.0" */
	ptr=(unsigned short*)(W5300_ADDRESS | SUBR<<1);
	*ptr = ((uint16_t)Subnet[0]<<8)+(uint16_t)Subnet[1];
	ptr += 2;
	*ptr = ((uint16_t)Subnet[2]<<8)+(uint16_t)Subnet[3];
	
	/* Set MAC address as "48 53 00 31 30 33"*/
	ptr=(unsigned short*)(W5300_ADDRESS | SHAR<<1);
	*ptr = ((uint16_t)MAC[0]<<8)+(uint16_t)MAC[1];
	ptr += 2;
	*ptr = ((uint16_t)MAC[2]<<8)+(uint16_t)MAC[3];
	ptr += 2;
	*ptr = ((uint16_t)MAC[4]<<8)+(uint16_t)MAC[5];
	
	/* Set interrupt mask */
	ptr=(unsigned short*)(W5300_ADDRESS | IMR<<1);
	*ptr=(IR_CONFLICT|IR_SOCK(0));
}

/* Read Interrupt Register */
void Read_IR(void)
{
	unsigned short *ptr,i,j;

	ptr=(unsigned short*)(W5300_ADDRESS | IR<<1);
	i=*ptr;	 			/* Read IR Register */
	*ptr=i;		/* Clear IR Token */

	// Process IP Coflict 
	if((i & IR_CONFLICT) == IR_CONFLICT){}
	// In UDP mode, Destination IP can't be reached
	if((i & IR_UNREACH) == IR_UNREACH){}
	
	/* Sn_IR(0)判断 */
	if((i & IR_SOCK(0)) == IR_SOCK(0))
	{
		ptr=(unsigned short*)(W5300_ADDRESS | Sn_IR(0)<<1);
		j=*ptr;
		*ptr=j;
		
		/*接收数据，一般不用中断来处理接收事件，怕来不及，详见说明文档
		if(j&Sn_IR_RECV) S0_Recv=1;*/
		
		/*发送前必须进行上一次发送是否完成的判断*/
		if(j&Sn_IR_SEND_OK) S0_SendOK=1;
		/* Sn_IR_TIMEOUT
		if(j&Sn_IR_TIMEOUT){
			ptr=(unsigned short*)(W5300_ADDRESS | Sn_CR(0)<<1);
			*ptr=Sn_CR_CLOSE;
			S0_SendOK=0;S0_TimeOut=1;
		}*/
	}
}
/******************** Set Socket0 in UDP mode *********************/
unsigned int Socket0_UDP(void)
{
	unsigned short *ptr;
	unsigned short i;
	/* Set Socket Port Number as 5000*/
	ptr=(unsigned short*)(W5300_ADDRESS | Sn_PORTR(0)<<1);
	*ptr=W5300_PORT;   //8084  0x1f94

	
	/* Set Destination IP  as "192.168.1.16"*/ 
	ptr=(unsigned short*)(W5300_ADDRESS | Sn_DIPR(0)<<1);
	*ptr=0xc0a8;

	ptr += 2;
	*ptr=0x01ff;

	
	/* Set Destination Port number as 1401*/ 
	ptr=(unsigned short*)(W5300_ADDRESS | Sn_DPORTR(0)<<1);
	*ptr=XiaWeiJi_PORT;

	/* Set Max. segment size  as 1472 */
	ptr=(unsigned short*)(W5300_ADDRESS | Sn_MSSR(0)<<1);
	*ptr=MaxSegSize;
   	
	/* Set Socket0 in UDP mode */
	ptr=(unsigned short*)(W5300_ADDRESS | Sn_MR(0)<<1);
	*ptr=Sn_MR_UDP;

	/* Open Socket0 */
	ptr=(unsigned short*)(W5300_ADDRESS | Sn_CR(0)<<1);
	*ptr=Sn_CR_OPEN;
	
	delay_ms(100);
	ptr=(unsigned short*)(W5300_ADDRESS | Sn_SSR(0)<<1);
	i=*ptr;
	if((i&0x00ff)!=SOCK_UDP)		/* If failed to open Socket0, close Socket0 and reurn false */
	{
		ptr=(unsigned short*)(W5300_ADDRESS | Sn_CR(0)<<1);
		*ptr=Sn_CR_CLOSE;
		return FALSE;
	}
	
	S0_SendOK=1;
	return TRUE;
}



/*************** Detect W5300 State ******************/
unsigned short Detect_State(void)
{
	unsigned short *ptr,i;

	ptr=(unsigned short*)(W5300_ADDRESS | Sn_SSR(0)<<1);
	i=*ptr;
	i&=0x00ff;
	
	return i;
}


int R_COUNT;
/* Read data from RX buffer and dump them to S_buffer */
unsigned short rx_size;
unsigned short rx_size_real;
unsigned short receive[20];
unsigned short S0_rx(void)
{
	unsigned short *ptr;
	
	unsigned short i,j;
	/* Check Received data size */
	ptr=(unsigned short*)(W5300_ADDRESS | Sn_RX_RSR(0)<<1);
	rx_size=*ptr;    
	ptr += 2;
	rx_size=*ptr;     //rx_size 接收数据字节数
  
	if(rx_size==0) 		/* If no Received data, return 0 */
	{
		return 0;
	}
	
	rx_size-=8;  //前8字节为PACKET_INFO 包含IP port 和数据包长度
	if(rx_size&0x0001) //奇数
		i=(rx_size+1)/2;
	else               //偶数
		i=rx_size/2;

	ptr=(unsigned short*)(W5300_ADDRESS | Sn_RX_FIFOR(0)<<1);
	
	for(int n=0;n<4;n++)
	UDP_Preamble[n]=*ptr;

	for(j=0;j<i;j++)
		receive[j]=*ptr;  
	
  rx_size_real=UDP_Preamble[3];
	ptr=(unsigned short*)(W5300_ADDRESS | Sn_CR(0)<<1);		/* Set RECV command */
	*ptr=Sn_CR_RECV;   //Sn_CR sockte命令寄存器

	 R_COUNT++;
	 if(R_COUNT>=PondBuffer)
		R_COUNT=0; 
 
	return rx_size_real;
}

void reset5300(void)
{
		GPIO_ResetBits(GPIOA,GPIO_Pin_4); //GPIOF9,10高电平
		delay_ms(10);
		GPIO_SetBits(GPIOA,GPIO_Pin_4);
		delay_ms(1000); 
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

/* write S_Buffer data to TX buffer and send them */
int Tranmit_Count=0;
extern int Tranmit_Flag;
extern struct udp_pcb *udp_XiaWeiJi_pcb;
u8 traning1,traning2;
extern uint8_t question;
u8 sendyeah;
extern int tx_size_16;
void S0_tx(void)
{
	unsigned short *ptr;
	unsigned short j;
	
	
			//发送数据
		if(sendyeah)
		{
		ptr=(unsigned short*)(W5300_ADDRESS | Sn_TX_WRSR(0)<<1);
		*ptr=0;
		ptr += 2;
		*ptr=tx_size;   //将发送数据大小填入 Sn_TX_WRSR寄存器
		ptr=(unsigned short*)(W5300_ADDRESS | Sn_CR(0)<<1);		//Set SEND command
		*ptr=Sn_CR_SEND;  //开启发送	
		S0_SendOK = 0;//标记为当前正在发送
		}
		sendyeah=0;
		
		ptr=(unsigned short*)(W5300_ADDRESS | Sn_TX_FIFOR(0)<<1);
		switch(Tranmit_Flag%BIGBUFFER)
		{
			case 0:
						if(A_Remain>1)
					{
						sendyeah=1;
						for(j=0;j<tx_size_16;j++)
						{
							*(ptr)=((unsigned short)(T_XiaWeiJi_A[2*j+(integer101-A_Remain)*tx_buffer_interval]<<8)+T_XiaWeiJi_A[2*j+1+(integer101-A_Remain)*tx_buffer_interval]);  //填充Sn_TX_FIFOR寄存器  //tx_size位数组
//							T_XiaWeiJi_A[2*j+(integer101-A_Remain)*tx_size]=0;
//							T_XiaWeiJi_A[2*j+1+(integer101-A_Remain)*tx_buffer_interval]=0;
						}
						A_Remain--;
					Tranmit_Count++;
						if(Tranmit_Count==daqu_dataPacket_size)
						{
							Tranmit_Count=0;
							Tranmit_Flag++;
						}
					}
					else
						question=1;
			break;
			case 1:
						if(B_Remain>1)
						{		
							sendyeah=1;
						for(j=0;j<tx_size_16;j++)
					{
						*(ptr)=(unsigned short)(T_XiaWeiJi_B[2*j+(integer101-B_Remain)*tx_buffer_interval]<<8)+T_XiaWeiJi_B[2*j+1+(integer101-B_Remain)*tx_buffer_interval];  //填充Sn_TX_FIFOR寄存器  //tx_size位数组
//						T_XiaWeiJi_B[2*j+(integer101-B_Remain)*tx_buffer_interval]=0;
//						T_XiaWeiJi_B[2*j+1+(integer101-B_Remain)*tx_buffer_interval]=0;
					}
							B_Remain--;
							Tranmit_Count++;
					
							if(Tranmit_Count==daqu_dataPacket_size)
							{
								Tranmit_Count=0;
								Tranmit_Flag++;
							}
							}
						else
						question=1;
				break;
			case 2:
							if(C_Remain>1)
						{	
							sendyeah=1;
							for(j=0;j<tx_size_16;j++)
							{
								*(ptr)=((unsigned short)(T_XiaWeiJi_C[2*j+(integer101-C_Remain)*tx_buffer_interval]<<8)+T_XiaWeiJi_C[2*j+1+(integer101-C_Remain)*tx_buffer_interval]);  //填充Sn_TX_FIFOR寄存器  //tx_size位数组
//							T_XiaWeiJi_C[2*j+(integer101-C_Remain)*tx_buffer_interval]=0;
//							T_XiaWeiJi_C[2*j+1+(integer101-C_Remain)*tx_buffer_interval]=0;	
							}
						C_Remain--;
						Tranmit_Count++;
							if(Tranmit_Count==daqu_dataPacket_size)
							{
								Tranmit_Count=0;
								Tranmit_Flag++;
							}
							}
						else
						question=1;
				break;
			case 3:
								if((A2_Remain==integer101)&&(D_Remain>1))
								{
									sendyeah=1;
									for(j=0;j<tx_size_16;j++)
									{
										*(ptr)=((unsigned short)(T_XiaWeiJi_D[2*j+(integer101-D_Remain)*tx_buffer_interval]<<8)+T_XiaWeiJi_D[2*j+1+(integer101-D_Remain)*tx_buffer_interval]);  //填充Sn_TX_FIFOR寄存器  //tx_size位数组
//									T_XiaWeiJi_D[2*j+(integer101-D_Remain)*tx_buffer_interval]=0;
//										T_XiaWeiJi_D[2*j+1+(integer101-D_Remain)*tx_buffer_interval]=0;
									}
								D_Remain--;
								Tranmit_Count++;
									if(Tranmit_Count==daqu_dataPacket_size)
									{
										Tranmit_Count=0;
										Tranmit_Flag++;
									}
							}
								else
						     question=1;
				break;
			
			
			
			case 4:
							if(A2_Remain>1)
						{
							sendyeah=1;
							for(j=0;j<tx_size_16;j++)
						{
							*(ptr)=(unsigned short)(T_XiaWeiJi_A2[2*j+(integer101-A2_Remain)*tx_buffer_interval]<<8)+T_XiaWeiJi_A2[2*j+1+(integer101-A2_Remain)*tx_buffer_interval];  //填充Sn_TX_FIFOR寄存器  //tx_size位数组
//						T_XiaWeiJi_A2[2*j+(integer101-A2_Remain)*tx_buffer_interval]=0;
//							T_XiaWeiJi_A2[2*j+1+(integer101-A2_Remain)*tx_buffer_interval]=0;
						}
								A2_Remain--;
								Tranmit_Count++;
								if(Tranmit_Count==daqu_dataPacket_size)
								{
									Tranmit_Count=0;
									Tranmit_Flag++;
								}
								}
						else
						question=1;
				break;
			case 5:
							if(B2_Remain>1)
						{	
							sendyeah=1;
							for(j=0;j<tx_size_16;j++)
						{
							*(ptr)=(unsigned short)(T_XiaWeiJi_B2[2*j+(integer101-B2_Remain)*tx_buffer_interval]<<8)+T_XiaWeiJi_B2[2*j+1+(integer101-B2_Remain)*tx_buffer_interval];  //填充Sn_TX_FIFOR寄存器  //tx_size位数组
//						T_XiaWeiJi_B2[2*j+(integer101-B2_Remain)*tx_buffer_interval]=0;
//							T_XiaWeiJi_B2[2*j+1+(integer101-B2_Remain)*tx_buffer_interval]=0;
						}
								B2_Remain--;
								Tranmit_Count++;
								if(Tranmit_Count==daqu_dataPacket_size)
								{
									Tranmit_Count=0;
									Tranmit_Flag++;
								}
								}
						else
						question=1;
				break;
			case 6:
							if(C2_Remain>1)
						{	
							sendyeah=1;
							for(j=0;j<tx_size_16;j++)
						{
							*(ptr)=(unsigned short)(T_XiaWeiJi_C2[2*j+(integer101-C2_Remain)*tx_buffer_interval]<<8)+T_XiaWeiJi_C2[2*j+1+(integer101-C2_Remain)*tx_buffer_interval];  //填充Sn_TX_FIFOR寄存器  //tx_size位数组
//						T_XiaWeiJi_C2[2*j+(integer101-C2_Remain)*tx_buffer_interval]=0;
//							T_XiaWeiJi_C2[2*j+1+(integer101-C2_Remain)*tx_buffer_interval]=0;
						}
								C2_Remain--;
								Tranmit_Count++;
								if(Tranmit_Count==daqu_dataPacket_size)
								{
									Tranmit_Count=0;
									Tranmit_Flag++;
								}
							}
						else
						question=1;
				break;
			case 7:
						if((D2_Remain>1)&&(A_Remain==integer101))
					{
						sendyeah=1;
						for(j=0;j<tx_size_16;j++)
					{
						*(ptr)=(unsigned short)(T_XiaWeiJi_D2[2*j+(integer101-D2_Remain)*tx_buffer_interval]<<8)+T_XiaWeiJi_D2[2*j+1+(integer101-D2_Remain)*tx_buffer_interval];  //填充Sn_TX_FIFOR寄存器  //tx_size位数组
//					  T_XiaWeiJi_D2[2*j+(integer101-D2_Remain)*tx_buffer_interval]=0;
//						T_XiaWeiJi_D2[2*j+1+(integer101-D2_Remain)*tx_buffer_interval]=0;	
					}
							D2_Remain--;
							Tranmit_Count++;
							if(Tranmit_Count==daqu_dataPacket_size)
							{
								Tranmit_Count=0;
								Tranmit_Flag++;
							}
							}
					else
						question=1;
				break;
			

			default:
				break;
		}


    
		if((D_Remain==1)||(D2_Remain==1)||(B_Remain==1)||(B2_Remain==1))
		{
			if(traning2==0)
			{	
				udp_XiaWeiJi_senddata(udp_XiaWeiJi_pcb);
        traning1++;		
        traning2=1;								
			}
			
		}

}

