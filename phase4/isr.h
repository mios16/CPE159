// isr.h, 159

#ifndef _ISR_H_
#define _ISR_H_

void StartProcISR();
void TimerISR();
//phase 2
void GetPidISR();
void SleepISR();

//phase 3
void SemGetISR();
void SemPostISR();
void SemWaitISR();

//phase 4
void MsgSndISR();
void MsgRcvISR();

#endif
