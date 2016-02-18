// isr.c, 159

#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h"
#include "main.h"

void StartProcISR(int new_pid) 
{
	MyBzero((char *) &pcb[new_pid], sizeof(pcb_t));	//clear the PCB of the new pid
	pcb[new_pid].state = READY;		//set its state to READY
	if(new_pid != 0)			//if new pid is not 0 (IdleProc),
	{
	      EnQ(new_pid, &ready_q);	//then, enqueue this new pid into the ready queue
	}
}

void EndProcISR() 
{
   	if(running_pid == 0)		//if running PID is 0 (IdleProc should not let exit),
   	{
		return;   		//then, just return;
   	}
	pcb[running_pid].state = FREE;	//change state of running process to FREE
	EnQ(running_pid, &free_q);	//queue the running PID to free queue
	running_pid = -1;		//set running PID to -1 (now none)
}        

void TimerISR() 
{
   	if(running_pid == -1)	//just return if running PID is -1 (not any valid PID)
   	{
		cons_printf("Panic Message! PID = -1\n");
		return;	//(shouldn't happen, a Panic message can be considered)
   	}
	
	//in PCB, upcount both runtime and total_runtime of running process
	pcb[running_pid].runtime = pcb[running_pid].runtime + 1;	
   	pcb[running_pid].total_runtime = pcb[running_pid].total_runtime + 1;
	
   	if(pcb[running_pid].runtime == TIME_LIMIT)	//if the runtime has reached TIME_LIMIT:
   	{
		pcb[running_pid].runtime = 0;   	//reset its runtime
        	pcb[running_pid].state = READY;       //change its state to READY
        	EnQ(running_pid, &ready_q);		//queue it to ready queue
        	running_pid = -1;			//set running PID to -1
   	
		//(Scheduler() will get next PID from ready queue if any;
      			//if none, Scheduler will pick 0 as running PID)
	}
}
