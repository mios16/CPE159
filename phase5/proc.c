// proc.c, 159
// processes are coded here

#include "spede.h"      	// for IO_DELAY() needed here below
#include "extern.h"     	// for currently-running pid needed below
#include "proc.h"       	// for prototypes of process functions
//Phase 2
#include "syscall.h"
#include "isr.h"
//phase 5
#include "toolfunc.h"

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



//////////////////Phase 3

void InitProc(int product_sem_id) 
{
	char key;
	msg_t temp_msg;
	char greet[] = "Greetings from team MIOS!\n";
	MyStrcpy((char *) &temp_msg.data, (char *) &greet); //send a greetings message
	
	temp_msg.recipient = 2;	
	
	while(1)	//loops infinitely to poll for a key
	{
		Sleep(1);	//repeat to sleep for a second inside the infinite loop
		if(cons_kbhit())
		{
			key = cons_getchar();
			switch(key)
			{
				case 'p':
					MsgSnd(&temp_msg);
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

/////////////////////////////////////phase 5

void PrintDriver() 
{
	int i, code;
	char *p;
	msg_t temp_msg;
	

 	printing_semaphore = SemGet(0);	//request semaphore printing_semaphore, limit 0.

	// reset printer (check printer power, cable, and paper), it will jitter
   	outportb(LPT1_BASE+LPT_CONTROL, PC_SLCTIN);   // CONTROL reg, SeLeCT INterrupt
   	code = inportb(LPT1_BASE+LPT_STATUS);         // read STATUS
   	for(i = 0; i < 50; i++)	//loop 50 times of IO_DELAY(); 		
   	{
		IO_DELAY();
  	}	        
   	outportb(LPT1_BASE+LPT_CONTROL, PC_INIT|PC_SLCTIN|PC_IRQEN); // IRQ ENable
   	Sleep(1);	//Sleep for a second, needs time resetting

	while(1)//forever loop:
	{
		temp_msg.recipient = running_pid;	
		MsgRcv(&temp_msg);//receive a message, get if msg to print *******
		p = temp_msg.data;
		cons_printf("PrintDriver (PID %d) now prints...\n", GetPid()); 	//a notification msg (match how demo runs)

    		//set p to point to start of character string in message
      		while (*p != '\0')	//"what p points to" is not null/empty/(char)0, 
         	{	
			outportb(LPT1_BASE+LPT_DATA, *p);       // write char to DATA reg
         		code = inportb(LPT1_BASE+LPT_CONTROL);  // read CONTROL reg
         		outportb(LPT1_BASE+LPT_CONTROL, code|PC_STROBE); // write CONTROL, STROBE added
         		for(i = 0; i < 50; i++)	   //do 50 times of IO_DELAY		
   			{
				IO_DELAY();
  			}	       
         		outportb(LPT1_BASE+LPT_CONTROL, code);  // send back original CONTROL
         		SemWait(printing_semaphore);//semaphore-wait on the printing semaphore

         		p++;	//move p to next character to print
      		}	
   	}	
}	



