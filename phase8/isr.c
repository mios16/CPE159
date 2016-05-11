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
	//phase 5
	MyBzero((char *) &msg_q[new_pid], sizeof(msg_q_t));

	pcb[new_pid].state = READY;			//set its state to READY
	if(new_pid != 0)				//if new pid is not 0 (IdleProc),
	{
	      EnQ(new_pid, &ready_q);			//then, enqueue this new pid into the ready queue
	}

	//build initial trapframe in proc stack
	MyBzero((char *)&proc_stack[new_pid], PROC_STACK_SIZE);	//call MyBzero() to clear the stack 1st

	pcb[new_pid].TF_ptr = (TF_t *) &proc_stack[new_pid][PROC_STACK_SIZE - sizeof(TF_t)];	//set TF_ptr of PCB to close to end (top) of stack

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
	pcb[running_pid].wake_time = OS_clock + (secs * 100);	//calculate wake time for the calling process, mark the wake time in its PCB
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
	}
			
}

void MsgRcvISR(int msg_addr)
{
	int msg_q_id;
	msg_t *receiving_msg_ptr, *queued_msg_ptr;	

	receiving_msg_ptr = (msg_t *) msg_addr;	//set 1st abot to point to msg space
	msg_q_id = running_pid; 
	if(msg_q[msg_q_id].len > 0)
	{	 
		queued_msg_ptr = MsgDeQ(&(msg_q[msg_q_id]));
		*receiving_msg_ptr = *queued_msg_ptr;
		return;
	}
	else
	{
		EnQ(running_pid, &msg_q[msg_q_id].wait_q);
		pcb[running_pid].state = WAIT;	
		running_pid = -1;
	}
}	


//phase 6

void IRQ3ISR()
{
	//int dismiss; 		
	//dismiss = inportb(COM2_IOBASE+IIR);	// dismiss irq3
	switch(inportb(COM2_IOBASE+IIR))			//switch read result
	{
		// call TX() and break(send char to terminal)
		case IIR_TXRDY:
			TX();
			break;
		//call RX() and break(send char from terminal)
		case IIR_RXRDY:
			RX();
			break;
	}
	if(port_data.TXRDY == 1)
	{
		TX();		//call TX() (check if can use it now to trasmit out a char)
	}
}

//dequeue out_q to write to port
void TX()
{
	char ch = 0;	//Nul, '\0'
	
	if(port_data.echo_buffer.len != 0)			//if echo buffer of port data is not empty
	{
		ch = DeQ(&port_data.echo_buffer);		//ch = dequeue from echo buffer
	}
	else
	{
		if(port_data.TX_buffer.len != 0)		// TX buffer of port data is not empty
		{
			ch = DeQ(&port_data.TX_buffer);	
			SemPostISR(port_data.TX_semaphore);	//SemPostISR TX semaphore of port data;
		}
	}

	if(ch !=0)
	{
		outportb(COM2_IOBASE+DATA, ch);	//use outportb to write ch to COM2_IOBASE+DATA
		port_data.TXRDY = 0;	//clear TXRDY of port data
	}
	else
	{
		port_data.TXRDY = 1; //set TXRDY of port data to 1
	}
}


void RX() 
{
	// read char from port to queue in_q and echo_q
	char ch;
	
	//use 127 to mask and msb (rset 7 bits in ASCII range)
	ch = inportb(COM2_IOBASE+DATA) & 0x7F;	//mask 0111 1111

	EnQ(ch, &port_data.RX_buffer);			//enqueue ch to RX buffer of port data
	SemPostISR(port_data.RX_semaphore);			//SemPostISR RX_semaphore of port data

	if(ch == '\r')
	{
		//enqueue '\r' then '\n' to each buffer of port data
		EnQ('\r', &port_data.echo_buffer);		
		EnQ('\n', &port_data.echo_buffer);
	}
	else
	{
		if(port_data.echo_mode == 1)	//echo mode of port data is 1
		{
			EnQ(ch, &port_data.echo_buffer);
		}
	}

}

//////////////Phase 8
void ForkISR(int addr, int size)
{
	int new_pid;
	int i;
	char *p;
	p = (char *) addr;
	if(free_q.len == 0)
	{
		cons_printf("Panic: no free PID left!\n");
		pcb[running_pid].TF_ptr->ecx = -1;	
		return;
	}
	for(i = 0; i < 100; i++)
	{
		if(DRAM[i].owner == -1)
		{
			break;
		}
		else if(i == 99)
		{
			cons_printf("Panic: no free DRAM space left!\n");
			pcb[running_pid].TF_ptr->ecx = -1;
			return;
		}	
	}
	new_pid = DeQ(&free_q);
	pcb[running_pid].TF_ptr->ecx = new_pid;
	DRAM[i].owner = new_pid;
	EnQ(new_pid ,&ready_q);
	
	MyBzero((char *) &msg_q[new_pid], sizeof(msg_q_t));
	MyBzero((char *) &pcb[new_pid], sizeof(pcb_t));	//clear the PCB of the new pid
	pcb[new_pid].state = READY;			//set its state to READY
	pcb[new_pid].ppid = running_pid;

	MyMemcpy((char *) DRAM[i].addr, p, size);	
	pcb[new_pid].TF_ptr = (TF_t *) (DRAM[i].addr + PROC_STACK_SIZE - sizeof(TF_t));

	pcb[new_pid].TF_ptr->eflags = EF_DEFAULT_VALUE|EF_INTR;		//set INTR flag   
	pcb[new_pid].TF_ptr->cs = get_cs();				//standard fair
	pcb[new_pid].TF_ptr->ds = get_ds();				//standard fair
	pcb[new_pid].TF_ptr->es = get_es();				//standard fair
	pcb[new_pid].TF_ptr->fs = get_fs();				//standard fair
	pcb[new_pid].TF_ptr->gs = get_gs();				//standard fair

	pcb[new_pid].TF_ptr->eip = DRAM[i].addr + 128;		
}

void WaitISR(int exit_code)
{
	int i,j;
	int *parent;

	parent = (int *)exit_code;

	for(i = 0; i < MAX_PROC_NUM; i++)
	{
		if((pcb[i].ppid == running_pid) && (pcb[i].state == ZOMBIE))
		{
			pcb[running_pid].TF_ptr->ebx = i;
			//pcb[running_pid].TF_ptr->eax = pcb[i].TF_ptr->eax; //***
			*parent = pcb[i].TF_ptr->eax;

			for( j = 0; j < 100; j++)
			{
				if(DRAM[j].owner == i)
				{
					MyBzero((char *) DRAM[i].addr, PROC_STACK_SIZE);
					DRAM[i].owner = -1;
					break;
				}
				
			}
			EnQ(i ,&free_q);
			MyBzero((char *) &pcb[i], sizeof(pcb_t));
			return;
		}	
	}
	
	pcb[running_pid].state = FORKWAIT;
	running_pid = -1;
	return;
}	

void ExitISR(int exit_code)
{
	int parpid,j;
	int *parent, *par;
	
	parpid = pcb[running_pid].ppid;

	if(pcb[parpid].state != FORKWAIT)
	{
		pcb[parpid].state = ZOMBIE;
		running_pid = -1;
		return;
	}

	pcb[parpid].state = READY;
	EnQ(parpid, &ready_q);
	
	pcb[parpid].TF_ptr->ebx = running_pid;
	par = (int *) pcb[parpid].TF_ptr->eax;
	*par = exit_code;
	//(int *)pcb[parpid].TF_ptr->eax = pcb[running_pid].TF_ptr->eax;


	for( j = 0; j < 100; j++)
	{
		if(DRAM[j].owner == running_pid)
		{
			MyBzero((char *) DRAM[j].addr, PROC_STACK_SIZE);
			DRAM[j].owner = -1;
			break;
		}	
	}
	MyBzero((char *) &pcb[running_pid], sizeof(pcb_t));
	EnQ(running_pid, &free_q);
	running_pid = -1;
}






