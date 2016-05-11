// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

void StartProcISR(int, int);
void TimerISR();
//phase 2
void GetPidISR();
void SleepISR();

//phase 3
void SemGetISR(int);
void SemPostISR(int);
void SemWaitISR(int);

//phase 4
void MsgSndISR(int);
void MsgRcvISR(int);

//phase 6
void IRQ3ISR();
void TX();
void RX();

#endif
