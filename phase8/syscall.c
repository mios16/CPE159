// syscall.c
// software interrupt/syscalls, i.e., kernel service API
#include "typedef.h"

int GetPid() {                   // no input, has return
   int pid;
   asm("int $48; movl %%eax, %0" // CPU inst
       : "=g" (pid)              // output from asm("...")
       :                         // no input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after

   return pid;
}

void Sleep(int seconds) {        // has input, no return

   asm("movl %0, %%eax; int $49" // CPU inst
       :                         // no output from asm("...")
       : "g" (seconds)           // input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after
}


/////////////////////phase 3 code new syscalls
void StartProc(func_ptr_t p) {        // has input, no return
   asm("movl %0, %%eax; int $50" // CPU inst
       :                         // no output from asm("...")
       : "g" ((int)p)           // input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after
}

int SemGet(int limit) {                   // no input, has return ************
   int sem_id;
   asm("movl %1, %%eax; int $51; movl %%ebx, %0" // CPU inst
       : "=g" (sem_id)              // output from asm("...")
       : "g"  (limit)                        // no input into asm("...")
       : "%eax", "%ebx");                // will get pushed before asm("..."), and popped after

   return sem_id;
}

void SemWait(int sem_id) {        // has input, no return

   asm("movl %0, %%eax; int $52" // CPU inst
       :                         // no output from asm("...")
       : "g" (sem_id)           // input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after
}

void SemPost(int sem_id) {        // has input, no return

   asm("movl %0, %%eax; int $53" // CPU inst
       :                         // no output from asm("...")
       : "g" (sem_id)           // input into asm("...")
       : "%eax");                // will get pushed before asm("..."), and popped after
}

//////////////////Phase 4

void MsgSnd(msg_t *msg_ptr){			// CPU inst
   asm("movl %0,  %%eax; int $54"
       :			// no output from asm("...")
       :"g"((int) msg_ptr)			// input into asm("...")
       :"%eax");		// will get pushed before asm("..."), and popped after*
}

void MsgRcv(msg_t *msg_ptr){			// CPU inst
   asm("movl %0,  %%eax; int $55"
       :			// no output from asm("...")
       :"g"((int) msg_ptr)			// input into asm("...")
       :"%eax");		// will get pushed before asm("..."), and popped after*
}


//Phase 6
void TipIRQ3(){		
   asm("int $35");
}

//Phase 8
int Fork(char *addr, int size){		//FileService finds these info
	int pid;	
	asm("movl %1, %%eax; movl %2, %%ebx; int $56; movl %%ecx, %0"
       		: "=g" (pid) 			
       		:"g"((int) addr), "g"(size)	
        	: "%eax", "%ebx", "%ecx");		
	return pid;
}
int Wait(int *exit_code){	//return = child PID      //arg = & exit #
	int pid;
	asm("movl %1,  %%eax; int $57; movl %%ebx, %0"
       		:"=g"(pid)
       		:"g"((int) exit_code)
       		:"%eax","%ebx");	
	return pid;
}

void Exit(int exit_code){		//exit #

       asm("movl %0,  %%eax; int $58"
		:		
		:"g"(exit_code)
       		:"%eax");
	
}



