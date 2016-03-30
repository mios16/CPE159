// isr.c, 159

#include "spede.h"
#include "typedef.h"
#include "isr.h"
#include "toolfunc.h"
#include "extern.h"
#include "proc.h"
#include "TF.h"
#include "main.h"
#include "entry.h"
#include "syscall.h"

void StartProcISR(int new_pid, int func_addr) 
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

	pcb[new_pid].TF_ptr->eip = func_addr;
}
        

void TimerISR() 
{
   	if(running_pid < 1)	//just return if running PID is -1 (not any valid PID)
   	{
		//cons_printf("Panic Message! PID = -1\n");
		return;	//(shouldn't happen, a Panic message can be considered)
   	}
	//in PCB, upcount both runtime and total_runtime of running process
	pcb[running_pid].runtime = pcb[running_pid].runtime + 1;	
   	pcb[running_pid].total_runtime = pcb[running_pid].total_runtime + 1;	
   	if(pcb[running_pid].runtime == TIME_LIMIT)	//runtime has reached TIME_LIMIT:
   	{
		pcb[running_pid].runtime = 0;   	//reset its runtime
        	pcb[running_pid].state = READY;         //change its state to READY
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

///////////////////////////////Phase 3
void SemGetISR(int limit)
{
	int sem_id;
	sem_id = DeQ(&sem_q);
	
	if(sem_id == -1)
	{		
		pcb[running_pid].TF_ptr->ebx = -1;
	}
	else
	{
		MyBzero((char *) &sem[sem_id], sizeof(sem_t));  //clear sem[sem_id] 
		sem[sem_id].limit = limit;
		pcb[running_pid].TF_ptr->ebx = sem_id;		//put semid into ebx of TF of running process
	}
}


void SemPostISR(int sem_id)
{
	int temp;
	if(sem[sem_id].wait_q.len == 0)	//check if any awaits
	{
		sem[sem_id].limit++;

	}
	else
	{
		temp = DeQ(&(sem[sem_id].wait_q));
		EnQ(temp, &ready_q);
		pcb[running_pid].state = READY;
		cons_printf("\nSemPostISR(): freeing proc %d <---!", running_pid);
	}
}

void SemWaitISR(int sem_id)
{
	if(sem[sem_id].limit > 0)
	{
		sem[sem_id].limit--;
	}
	else	
	{	
		cons_printf("\nSemWaitISR(): blocking proc %d <---!", running_pid);
		EnQ(running_pid, &(sem[sem_id].wait_q));
		pcb[running_pid].state = WAIT;		
		running_pid = -1;		
	}
}

////////////phase 4
void MsgSndISR(int msg_addr)
{
	int msg_q_id;
	int freed_pid;
	msg_t *incoming_msg_ptr, *dst_msg_ptr; 
	
	incoming_msg_ptr = (msg_t *) msg_addr; 
	msg_q_id = incoming_msg_ptr->recipient;
	incoming_msg_ptr->OS_clock = OS_clock;
	incoming_msg_ptr->sender = running_pid;

	if(msg_q[msg_q_id].wait_q.len == 0)
	{
		MsgEnQ(incoming_msg_ptr, &msg_q[msg_q_id]);
	}	
	else
	{
		freed_pid = DeQ(&(msg_q[msg_q_id].wait_q));
		EnQ(freed_pid, &ready_q);
		pcb[freed_pid].state = READY;
		dst_msg_ptr = (msg_t *) pcb[freed_pid].TF_ptr->eax;
		*dst_msg_ptr = *incoming_msg_ptr;
		cons_printf("MsgSndISR: FREEING process # %d !\n", freed_pid);
	}
			
}

void MsgRcvISR(int msg_addr)
{
	int msg_q_id;
	msg_t *receiving_msg_ptr, *queued_msg_ptr;

	receiving_msg_ptr = (msg_t *) msg_addr;	//set 1st abot to point to msg space
	msg_q_id = receiving_msg_ptr->recipient; // *************
	if(msg_q[msg_q_id].len > 0)
	{	 
		queued_msg_ptr = MsgDeQ(&(msg_q[msg_q_id]));
		*receiving_msg_ptr = *queued_msg_ptr;
		return;
	}
	else
	{
		cons_printf("MsgRcvISR: BLOCKING process # %d !\n", running_pid);
		EnQ(running_pid, &msg_q[msg_q_id].wait_q);
		pcb[running_pid].state = WAIT;	
		running_pid = -1;
	}
}	




