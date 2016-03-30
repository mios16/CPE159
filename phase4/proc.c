// proc.c, 159
// processes are coded here

#include "spede.h"      	// for IO_DELAY() needed here below
#include "extern.h"     	// for currently-running pid needed below
#include "proc.h"       	// for prototypes of process functions
//Phase 2
#include "syscall.h"
#include "isr.h"

//wrap code inside IdleProc() and UserProc() with an infinite loop
void IdleProc() 
{
	while(1)
	{
   		int i;
		cons_printf("0 ");	//show msg on PC: "IdleProc (PID 0) runs.\n"
   		for(i = 0; i < 1666667; i++)			//loop 1666667 times calling IO_DELAY()
   		{
			IO_DELAY();
		}
	}	
}

///////////////////////////////////////////////Phase 4

void ProducerProc() 
{
	int i, my_pid;
	

	//////////////////phase 4
	static int count;		
	msg_t my_msg;	

	my_pid = GetPid();

	while(1)	//loop forever
	{	
		my_msg.recipient = 0;
		my_msg.data = my_pid * 100 + count++;
		cons_printf("\n++ Producer (%d) producing data %d...\n", my_pid, my_msg.data);
		for(i = 0; i < 3333333; i++)
		{
			IO_DELAY();	//busy loop for 2 seconds
		}
		MsgSnd(&my_msg);
	}
}

void ConsumerProc() 
{
	int i, my_pid;
	msg_t my_msg;
	my_pid = GetPid();
	

	while(1)	//loop forever
	{
		my_msg.recipient = 0;
		MsgRcv(&my_msg);
		cons_printf("\n-- Consumer (%d) consuming data %d...\n", my_pid, my_msg.data);
		for(i = 0; i < 3333333; i++)
		{
			IO_DELAY();	//busy loop for 2 seconds
		}
	}
}

//////////////////Phase 3

void InitProc(int product_sem_id) 
{
	char key;

	while(1)	//loops infinitely to poll for a key
	{
		Sleep(1);	//repeat to sleep for a second inside the infinite loop
		if(cons_kbhit())
		{
			key = cons_getchar();
			switch(key)
			{
				case 'p':
					StartProc(ProducerProc);//create a producer process
					break;
				case 'c':
					StartProc(ConsumerProc);//create a consumer process
					break;
				case 'b':
					breakpoint();	//breakpoint() to go into GDB
					break;
				case 'x':
					exit(0);	//exit(0) to quit MyOS.dli
					break;
				default :		// no keys were pressed
					break;
			}
		}
		
		
	}
}


