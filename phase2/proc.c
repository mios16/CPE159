// proc.c, 159
// processes are coded here

#include "spede.h"      	// for IO_DELAY() needed here below
#include "extern.h"     	// for currently-running pid needed below
#include "proc.h"       	// for prototypes of process functions

//wrap code inside IdleProc() and UserProc() with an infinite loop
void IdleProc() 
{
	while(1)
	{
   		int i;
		cons_printf("IdleProc (PID 0) runs. \n");	//show msg on PC: "IdleProc (PID 0) runs.\n"
   		for(i = 0; i <= 1666666; i++)			//loop 1666667 times calling IO_DELAY()
   		{
			IO_DELAY();
		}
	}	
}

void UserProc() 
{
	while(1)
	{
		int i;
		cons_printf("UserProc (PID %d) runs. \n", running_pid);	//show msg on target PC:cons_printf("UserProc (PID %d) runs.\n", running_pid);
		for(i = 0; i <= 1666666; i++)			//loop 1666667 times calling IO_DELAY()
   		{
			IO_DELAY();
   		}
		int current_pid = GetPid();	// Call GetPid(0 to get its own PID for printing 
*******		int calculation = current_pid(mod 3, plus 1); 	//  for calculation of its sleep seconds (mod 3, plus 1)
						//call Sleep() with the calculated sleep time		
	}
}
