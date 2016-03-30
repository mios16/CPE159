// main.c, 159
// Semaphore/Mutex

// Team Name: mios (Members: Roberto Centeno and Rohit Pandit)

#include "spede.h"      // spede stuff
#include "main.h"       // main stuff
#include "isr.h"        // ISR's
#include "toolfunc.h"   // handy functions for Kernel
#include "proc.h"       // processes such as IdleProc()
#include "typedef.h"    // data types
#include "entry.h"
#include "TF.h"
#include "syscall.h"

// kernel data stuff:
int running_pid, OS_clock;            // currently-running PID, if -1, none running
q_t ready_q, free_q, sleep_q;        // processes ready to run and ID's un-used
pcb_t pcb[MAX_PROC_NUM];    // process table
char proc_stack[MAX_PROC_NUM][PROC_STACK_SIZE]; // runtime stacks of processes

//phase 3 stuff
sem_t sem[Q_LEN];
q_t sem_q;

//phase 4
msg_q_t msg_q[MAX_PROC_NUM];

//phase 2 int OS_clock & q_t sleep_q

struct i386_gate *IDT_ptr;

void SetEntry(int entry_num, func_ptr_t func_ptr)
{
	struct i386_gate *gateptr = &IDT_ptr[entry_num];
	fill_gate(gateptr, (int)func_ptr, get_cs(), ACC_INTR_GATE, 0);
}


int main() 
{
   	int pid;

	InitKernelData();	//call InitKernelData()  to set kernel data
	InitKernelControl();	//call InitKernelControl() (see below)	
	
	pid = DeQ(&free_q);	
	StartProcISR(pid, IdleProc);
	
	//phase 3
	pid = DeQ(&free_q);	
	StartProcISR(pid, InitProc);

	LoadRun(pcb[0].TF_ptr);	//call LoadRun() to load/run IdleProc	
	return 0;		//this will never be executed
}


void InitKernelData() 
{
   	int i;
	MyBzero((char *)&ready_q, sizeof(q_t));
	MyBzero((char *)&free_q, sizeof(q_t));
	
	//phase 3
	MyBzero((char *)&sem_q, sizeof(q_t)); //clear the available semaphore queue 

   	for(i = 0; i < Q_LEN; i++)	// loop number i from 0 to 19
	{
      		EnQ(i, &free_q);	//call EnQ() to enqueue i to free_q
		//phase 3
		EnQ(i, &sem_q);	//fill it with available semaphore ID's (from 0 to Q_SIZE-1).
      		MyBzero((char *) &pcb[i], sizeof(pcb_t));
	}
   	running_pid = 0;//set running_pid to 0;  none initially, need to chose by Scheduler()

	//Phase 2
	OS_clock = 0;		//reset OS_clock to 0	
	MyBzero((char *)&sleep_q, sizeof(q_t));	//reset sleep_q

	//phase 4
	MyBzero((char *)&msg_q, sizeof(msg_q_t)); 		
}

void InitKernelControl() 
{ 				// learned from timer lab, remember to modify main.h
   	IDT_ptr = get_idt_base();	//locate IDT 1st
   	SetEntry(TIMER_INTR, TimerEntry);	//call SetEntry() to plant TimerEntry jump point
   	outportb(0x21, ~1);		//program the mask of PIC(PIC mask == 0x21 and ~1 == mask)
	
	//Phase 2	
	SetEntry(GETPID_INTR, GetPidEntry);	//add 2 new IDT entries.
	SetEntry(SLEEP_INTR, SleepEntry);	//add 2 new IDT entries.

	//phase 3 fill out IDT with new entries
	SetEntry(STARTPROC_INTR, StartProcEntry);	
	SetEntry(SEMGET_INTR, SemGetEntry);
	SetEntry(SEMWAIT_INTR, SemWaitEntry);	
	SetEntry(SEMPOST_INTR, SemPostEntry);

	//phase 4
	SetEntry(MSGSND_INTR, MsgSndEntry);	
	SetEntry(MSGRCV_INTR, MsgRcvEntry);

	
}

void Scheduler() {  // to choose running PID

	if(running_pid > 0)	//simply return if running_pid is greater than 0 (0 or less/-1 continues)
	{
		return;
	}
	if(running_pid == 0)
	{
		pcb[running_pid].state = READY;	//if running process ID is 0 (IdleProc), change its state to READY (from RUN)		
	}

	running_pid = DeQ(&ready_q);	//set running process ID = dequeue ready_q
	
	if(running_pid == -1)		//if it's -1 (didn't get one, ready_q was empty) 
	{
		running_pid = 0;		
	}
	pcb[running_pid].state = RUN;	//whoever's now selected as running process, set its state to RUN
}

void KernelMain(TF_t *TF_ptr) 
{
	int new_pid, i, pid, len;	

	pcb[running_pid].TF_ptr = TF_ptr;	//save TF_ptr to PCB of running process
	
	switch(TF_ptr->intr_id)			//switch on TF_ptr->intr_id
	{
		case TIMER_INTR:
				TimerISR();	//call TimerISR()
				outportb(0x20, 0x60);	//dismiss timer event: send PIC with a code
				OS_clock++;
				len = sleep_q.len;

				for(i = 0; i < len; i++)
				{
					pid = DeQ(&sleep_q);
					if(OS_clock == pcb[pid].wake_time)
					{
						EnQ(pid, &ready_q);
						pcb[pid].state = READY;
					}
					else
					{
						EnQ(pid, &sleep_q);
					}	
				}
				break;
		case GETPID_INTR:
				GetPidISR();
				break;
		case SLEEP_INTR:
				SleepISR();
				break;
		//phase 3 new switch() cases		
		case STARTPROC_INTR:
				new_pid = DeQ(&free_q);	//dequeue free_q for a new pid
				if (new_pid == -1)	//indicates no ID left
				{				
					cons_printf("Panic: no more available process ID left!\n");	//show msg on target PC			
				}
				else
				{
					StartProcISR(new_pid, TF_ptr->eax);//create new proc
				}
				break;
		case SEMGET_INTR:
				SemGetISR(TF_ptr->eax);
				break;
		case SEMWAIT_INTR:
				SemWaitISR(TF_ptr->eax);
				break;
		case SEMPOST_INTR:
				SemPostISR(TF_ptr->eax);
				break;
		//phase 4
		case MSGSND_INTR:
				MsgSndISR(TF_ptr->eax);	//Check if parameter needed*********
				break;
		case MSGRCV_INTR:
				MsgRcvISR(TF_ptr->eax);
				break;
		default:
				cons_printf("Panic: unknown intr ID (%d)!\n", TF_ptr->intr_id);
				breakpoint();		//fallback to GDB
	}

	Scheduler();				//call scheduler() to process to load/run if needed
	LoadRun(pcb[running_pid].TF_ptr);	//call LoadRun(pcb[running_pid].TF_ptr) to load/run selected proc
}
