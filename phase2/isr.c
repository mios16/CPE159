// isr.c, 159

#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h"
#include "main.h"
#include "entry.h"

void StartProcISR(int new_pid) 
{
	MyBzero((char *) &pcb[new_pid], sizeof(pcb_t));	//clear the PCB of the new pid
	pcb[new_pid].state = READY;		//set its state to READY
	if(new_pid != 0)			//if new pid is not 0 (IdleProc),
	{
	      EnQ(new_pid, &ready_q);	//then, enqueue this new pid into the ready queue
	}

	//build initial trapframe in proc stack
	MyBzero((char *)&proc_stack[new_pid], PROC_STACK_SIZE);	//call MyBzero() to clear the stack 1st

	pcb[new_pid].TF_ptr = (TF_t *) &proc_stack[new_pid][PROC_STACK_SIZE - sizeof(TF_t)];	//set TF_ptr of PCB to close to end (top) of stack, then fill out (against last byte of stack, has space for a trapframe o build)

	pcb[new_pid].TF_ptr->eflags = EF_DEFAULT_VALUE|EF_INTR;	//set INTR flag
	pcb[new_pid].TF_ptr->cs = get_cs();				//standard fair
	pcb[new_pid].TF_ptr->ds = get_ds();				//standard fair
	pcb[new_pid].TF_ptr->es = get_es();				//standard fair
	pcb[new_pid].TF_ptr->fs = get_fs();				//standard fair
	pcb[new_pid].TF_ptr->gs = get_gs();				//standard fair

	if(new_pid == 0)
	{
		pcb[new_pid].TF_ptr->eip = (unsigned int)IdleProc;
	}
	else
	{
		pcb[new_pid].TF_ptr->eip = (unsigned int)UserProc;
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
	}
}

void GetPidISR()
{
	pcb[running_pid].TF_ptr->eax = running_pid;	
}

void SleepISR()
{
	int secs = pcb[running_pid].TF_ptr->eax;
	pcb[running_pid].wake_time = OS_clock + (secs * 100);	//calculate the wake time for the calling process, mark the wake time in its PCB
	EnQ(running_pid, &sleep_q);		//queue its PID to the sleep queue
	pcb[running_pid].state = SLEEP;       	//change its state to SLEEP
	running_pid = -1;			//reset current running PID
}
