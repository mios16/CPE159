// proc.c, 159
// processes are coded here

#include "spede.h"      	// for IO_DELAY() needed here below
#include "extern.h"     	// for currently-running pid needed below
#include "proc.h"       	// for prototypes of process functions

void LoadRun(void) 			// this is not real
{         
   	if(running_pid == 0)		//if running_pid is 0:
   	{
   		IdleProc();		//call IdleProc()   // as if loads it and it runs
   	}   
	else				//else (all other processes)
   	{
		UserProc();		//call UserProc()   // as if loads it and it runs
	}
}

void IdleProc(void) 
{
   	int i;
	cons_printf("IdleProc (PID 0) runs. \n");	//show msg on PC: "IdleProc (PID 0) runs.\n"
   	for(i = 0; i <= 1666666; i++)			//loop 1666667 times calling IO_DELAY()
   	{
		IO_DELAY();
	}	
}

void UserProc() 
{
	int i;
	cons_printf("UserProc (PID %d) runs. \n", running_pid);	//show msg on target PC:cons_printf("UserProc (PID %d) runs.\n", running_pid);
	for(i = 0; i <= 1666666; i++)			//loop 1666667 times calling IO_DELAY()
   	{
		IO_DELAY();
   	}	
}
