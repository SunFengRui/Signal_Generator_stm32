#ifndef _PROCESS5300_H
#define _PROCESS5300_H

void W5300_Config(void);
void Read_IR(void);
unsigned int Socket0_UDP(void);
unsigned short Detect_State(void);
void S0_tx(void);
unsigned short S0_rx(void);
void reset5300(void);
#endif
