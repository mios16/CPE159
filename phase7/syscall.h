// syscall.h


#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "typedef.h"

int GetPid(void);  // no input, 1 return
void Sleep(int);   // 1 input, no return

//phase 3
void StartProc(func_ptr_t);
int SemGet(int);
void SemWait(int);
void SemPost(int);

//phase 4
void MsgSnd(msg_t *);
void MsgRcv(msg_t *);

//phase 6
void TipIRQ3();


#endif

