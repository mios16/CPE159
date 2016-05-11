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


////////////////////////////////Phase 6

/****************************************

Serial data communication vocabulary:
   // COM1_IOBASE ~ COM8_IOBASE: 0x3f8 0x2f8 0x3e8 0x2e8 0x2f0 0x3e0 0x2e0 0x260
   // transmit speed 9600 bauds, clear IER, start TXRDY and RXRDY
   // data communication acronyms:
   //    IIR Intr Indicator Reg
   //    IER Intr Enable Reg
   //    ETXRDY Enable Xmit Ready
   //    ERXRDY Enable Recv Ready
   //    MSR Modem Status Reg
   //    MCR Modem Control Reg
   //    LSR Line Status Reg
   //    CFCR Char Format Control Reg
   //    LSR_TSRE Line Status Reg, Xmit+Shift Regs Empty
****************************************/

void ShellProc()
{
	//ShellProc, PID 2, is coded with:
	msg_t my_msg;
	char a_string[101];					// a handy string
	int BAUD_RATE, divisor, my_pid;

	//initialize the interface data structure:
	MyBzero((char *)&port_data, sizeof(port_data_t));	//first clear (bzero) port data	
	TX_semaphore = msg_q[Q_LEN]				// request for a semaphore as TX_semaphore, limit is max queue length
	RX_semaphore = SemGet(0);				// request for a semaphore as RX_semaphore, limit is 0               
	echo_mode = 1;						// set echo_mode to 1 (default echo back to terminal)
	TXRDY = 1;						// set TXRDY to 1 (missed 1st TXRDY event) 
	stdin_pid = my_pid + 1;					// set stdin_pid to my_pid + 1
	stdout_pid = my_pid + 2;				// set stdout_pid to my_pid + 2

	//reset the serial port:
	BAUD_RATE = 9600; 					//set baud rate 9600, invented by Mr. Baud
	divisor = 115200 / BAUD_Rate;				// time period of each baud
	outportb(COM2_IOBASE+CFCR, CFCR_DLAB);			// CFCR_DLAB 0x80
	outportb(COM2_IOBASE+BAUDLO, LOBYTE(divisor));
	outportb(COM2_IOBASE+BAUDHI, HIBYTE(divisor));
	// set CFCR: 7-E-1 (7 data bits, even parity, 1 stop bit)
	outportb(COM2_IOBASE+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BIS);
	outportb(COM2_IOBASE+IER, 0);
	//raise DTR, RTS of the serial port to start read/write
	outportb(COM2_IOBASE+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
	IO_DELAY();
	outportb(COM2_IOBASE+_IER, IER_ERXRDY|IER_ETXRDY);	//enable TX, RX events
	IO_DELAY();	

	my_pid = GetPID(); 	// get my PID 				

	//MyStrcpy a hello-world string to my_msg.data
	char key;
	msg_t temp_msg;
	char hello[] = "Hello World\n";
	MyStrcpy((char *) &temp_msg.data, (char *) &hello);
	
	temp_msg.recipient = StdoutProc;		// set recipient of my_msg to StdoutProc (its PID set in port data)

	MsgSnd(&my_msg);		//send my_msg
	MsgRcv(&my_msg); // receive my_msg ( a reply, content don't care)
	p = temp_msg.data;
	cons_printf("Received my_msg for hello world"); 

	while(1)	//forever loop:
	{
		//prompt:
			char completion[] = "completion-- change according to demo\n";
			MyStrcpy((char *) &my_msg.data, (char *) &completion);// MyStrcpy what's to be prompted to my_msg.data--hello world team Mios
			temp_msg.recipient = port_data.stdout_pid;	// set recipient to StdoutProc
			MsgSnd(&my_msg);			// send my_msg
			MsgRcv(&my_msg);			//receive my_msg ( a reply, contetent don't care)
			p = temp_msg.data;
			cons_printf("a reply"); 

	
//prompt "just entered -> " onto terminal( same details as above prompting example)	
		//prompt:
			char entered[] = "just entered ->";	//prompt "just entered -> " "onto terminal
			MyStrcpy((char *) &my_msg.data, (char *) &entered);// MyStrcpy what's to be prompted to my_msg.data
			temp_msg.recipient = port_data.stdout_pid; 	// set recipient to StdoutProc
			MsgSnd(&my_msg);			// send my_msg
			MsgRcv(&my_msg);			//receive my_msg ( a reply, contetent don't care)

	
// show a_string onto terminal (same details as above promppting example)
		//prompt:
			//show a_string onto terminal 
			// MyStrcpy what's to be prompted to my_msg.data ********view demo***************
			temp_msg.recipient = port_data.stdout_pid;	// set recipient to StdoutProc
			MsgSnd(&my_msg);			// send my_msg
			MsgRcv(&my_msg);			//receive my_msg ( a reply, contetent don't care)

		// get whats entered:
			temp_msg.recipient = port_data.stdin_pid;		// set recipient to StdinProc
			MsgSnd(&my_msg);			// send my_msg
			MsgRcv(&my_msg);			// receive my_msg (completion timing reply, content don't care)
			MyStrcpy((char *) &my_msg.data, (char *) &completion);// copy my_msg.data to a string (save for later)
			p = temp_msg.data;
			cons_printf("completion timing reply"); 

	}
}



void StdinProc()
{
	msg_t my_msg;
	char *p;
	
	while(1)// forever loop
	{
	temp_msg.recipient = running_pid;	
	MsgRcv(&temp_msg);			//receive my_msg
	p = my_msg.data;			//char ptr p points to my_msg.data

		while(1)
		{
			SemWait(port_data.RX_semaphore); 		// semaphore wait on RX_semaphore of port data
			outportb(COM2_IOBASE+_IER, IER_ERXRDY|IER_ETXRDY);		
			ch = MsgDeQ(&port_data.RX_buffer);		// ch = dequeue from RX_buffer of port data
			if (ch = '/r')					// if ch is '\r', break; // delimier encountered
			{				
				break;
			}
			*p++ = ch;			
		}
		*p = '\0';				// add NULL to terminate msg.data
		my_msg.recipient = my_msg.sender;	//set my_msg.recipient to my_msg.sender
		MsgSnd(&my_msg);			//send my_msg back
	}
}

void StdoutProc()
{
	msg_t my_msg;
	char *p;

	while(1)	// forever loop:
	{
		MsgRcv(&my_msg)		// receive my_msg (from user shell)
		char *p;
		p = my_msg.data;
			
			while(p !='\0')
			{
				SemWait(port_data.TX_semaphore);// semaphore wait on TX_semaphore of port data
				outportb(COM2_IOBASE+_IER, IER_ERXRDY|IER_ETXRDY);				
				MsgEnQ(*p, &portdata.TX_buffer);	//enqueue what p points to TX_buffer of port data
				if( p = '\n')
				{
					SemWait(port_data.TX_semaphore); 	// semaphore wait on TX_semaphore of port data
					MsgEnq(*p, &port_data.TX_buffer);	// enqueue '/r' to TX_buffer of port data
				}
				TipIRQ3(); 	//issue syscall "TipIRQ3();" to manually start IRQ-3 event
				p++;
}





