// main.c, 159
// the kernel is simulated, not yet real
//
// Team Name: ??????? (Members: ?????? and ??????)

#include "spede.h"      // spede stuff
#include "main.h"       // main stuff
#include "isr.h"        // ISR's
#include "toolfunc.h"   // handy functions for Kernel
#include "proc.h"       // processes such as IdleProc()
#include "typedef.h"    // data types

// kernel data stuff:
int running_pid;            // currently-running PID, if -1, none running
q_t ready_q, free_q;        // processes ready to run and ID's un-used
pcb_t pcb[MAX_PROC_NUM];    // process table
char proc_stack[MAX_PROC_NUM][PROC_STACK_SIZE]; // runtime stacks of processes

int main() {
   int pid;

   call InitKernelData()  to set kernel data

   call DeQ() to dequeue free_q to get pid
   call StartProcISR(pid) to create IdleProc

   infinite loop to alternate 2 things below:
      call LoadRun() to load/run the chosen process
      call KernelMain() to run kernel periodically to control things

   return 0;   // not reached, but compiler needs it for syntax
}

void InitKernelData() {
   int i;

   call MyBzero() to clear queues (which is to be coded in toolfunc.h/.c)

   loop number i from 0 to 19:
      call EnQ() to enqueue i to free_q
      call MyBzero() to clear pcb[i]

   set running_pid to 0;  none initially, need to chose by Scheduler()
}

void Scheduler() {  // to choose running PID
   simply return if running_pid is greater than 0 (0 or less/-1 continues)

   if running process ID is 0 (IdleProc), change its state to READY (from RUN)

   set running process ID = dequeue ready_q
   if it's -1 (didn't get one, ready_q was empty)
      set running process ID = 0 (fall back to IdleProc)

   whoever's now selected as running process, set its state to RUN
}

void KernelMain() {
   int new_pid;
   char key;

   call TimerISR() to service timer interrupt as if it just occurred

   if a key has been pressed on PC {
      read the key with cons_getchar()
      switch(key) {
         case 's'
            dequeue free_q for a new pid
            if the new pid (is -1) indicates no ID left
               show msg on target PC: "Panic: no more available process ID left!\n"
            else
               call StartProcISR(new pid) to create new proc
            break;

         case 'e'
            call EndProcISR() to handle this event
            break;

         case 'b'
            call breakpoint(); to go into GDB
            break;

         case 'x'
            just call exit(0) to quit MyOS.dli
     }
   }
   call Scheduler() to choose next running process if needed
}

