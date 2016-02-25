// proc.c, 159
// processes are coded here

#include "spede.h"      	// for IO_DELAY() needed here below
#include "extern.h"     	// for currently-running pid needed below
#include "proc.h"       	// for prototypes of process functions
//Phase 2
#include "syscall.h"

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

void UserProc() 
{
	while(1)
	{
		int current_pid, calculation;
		current_pid = GetPid();	// Call GetPid(0 to get its own PID for printing 
		cons_printf("%d ", current_pid);	//show msg on target
		
		//Phase 2
		calculation = (current_pid % 3) + 1; //(mod 3, plus 1); 	//  for calculation of its sleep seconds (mod 3, plus 1)
		Sleep(calculation);			//call Sleep() with the calculated sleep time		
	}
}
