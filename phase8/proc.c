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
//phase 7
#include "FileService.h"


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

void InitProc() 
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
	//ShellProc, PID 3, is coded with:
	msg_t my_msg;
	//phase 7
	char login[101], password[101];				// handy strings
	//char a_string[101];					// a handy string
	int BAUD_RATE, divisor, my_pid, FileServicePid;
	int size;
	attr_t *p;

	//phase 8
	int child_pid, exit_code;

	//initialize the interface data structure:
	MyBzero((char *)&port_data, sizeof(port_data_t));	//first clear (bzero) port data	
	port_data.TX_semaphore = SemGet(Q_LEN);	// request for a semaphore as TX_semaphore, limit is max queue length
	port_data.RX_semaphore = SemGet(0);	// request for a semaphore as RX_semaphore, limit is 0       

	//phase 7
	my_pid = GetPid(); 	// get my PID  
	FileServicePid = 2;
      
	port_data.echo_mode = 1;	//default echo back to terminal
	port_data.TXRDY = 1;			//missed 1st TXRDY event
	port_data.stdin_pid = my_pid + 1;		
	port_data.stdout_pid = my_pid + 2;		

	//reset the serial port:
   	BAUD_RATE = 9600;              // Mr. Baud invented this
  	divisor = 115200 / BAUD_RATE;  // time period of each baud
  	outportb(COM2_IOBASE+CFCR, CFCR_DLAB);          // CFCR_DLAB 0x80
  	outportb(COM2_IOBASE+BAUDLO, LOBYTE(divisor));
  	outportb(COM2_IOBASE+BAUDHI, HIBYTE(divisor));
  	// B. set CFCR: 7-E-1 (7 data bits, even parity, 1 stop bit)
  	outportb(COM2_IOBASE+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
  	outportb(COM2_IOBASE+IER, 0);
  	// C. raise DTR, RTS of the serial port to start read/write
  	outportb(COM2_IOBASE+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
   	IO_DELAY();
  	outportb(COM2_IOBASE+IER, IER_ERXRDY|IER_ETXRDY); // enable TX, RX events
  	IO_DELAY();	

					

	MyStrcpy(my_msg.data, "\n\n\nHello, World! Team MIOS\n\0"); 
	
	my_msg.recipient = port_data.stdout_pid;  // set recipient of my_msg to StdoutProc 
	MsgSnd(&my_msg);		//send my_msg
	MsgRcv(&my_msg); // receive my_msg ( a reply, content don't care)

	while(1)	//forever loop:
	{
		while(1)
		{
			MyStrcpy(my_msg.data, "\nTeam MIOS login > \0");
			my_msg.recipient = port_data.stdout_pid; 
			MsgSnd(&my_msg);			
			MsgRcv(&my_msg);
				
			// get whats entered login
			my_msg.recipient = port_data.stdin_pid; 	
			MsgSnd(&my_msg);			
			MsgRcv(&my_msg);		
			MyStrcpy(login, my_msg.data);	
		//prompt:
			MyStrcpy(my_msg.data, "Team MIOS password > ");
			my_msg.recipient = port_data.stdout_pid;
			MsgSnd(&my_msg);			
			MsgRcv(&my_msg);
	
			//turn echo off			
			port_data.echo_mode = 0;

		// get whats entered password
			my_msg.recipient = port_data.stdin_pid; 	
			MsgSnd(&my_msg);			
			MsgRcv(&my_msg);		
			MyStrcpy(password, my_msg.data);
		//turn echo on		
			port_data.echo_mode = 1;		
			
			size = MyStrlen(login);

			if(MyStrcmp(login, password, size) == 1)
			{
				MyStrcpy(my_msg.data, "Login successfull. Welcome!\n\0");
				my_msg.recipient = port_data.stdout_pid;
				MsgSnd(&my_msg);			
				MsgRcv(&my_msg);
				break;
			}
			else
			{
				MyStrcpy(my_msg.data, "Invalid Password!\n \0");
				my_msg.recipient = port_data.stdout_pid;
				MsgSnd(&my_msg);			
				MsgRcv(&my_msg);
			}
		}
		while(1)
		{
			MyStrcpy(my_msg.data, "Team MIOS shell command > \0");
			my_msg.recipient = port_data.stdout_pid; 
			MsgSnd(&my_msg);			
			MsgRcv(&my_msg);

			// get whats entered 
			my_msg.recipient = port_data.stdin_pid; 	
			MsgSnd(&my_msg);			
			MsgRcv(&my_msg);		

			if(my_msg.data[0] == '\0')
			{
				continue;
			}
			else if(MyStrcmp(my_msg.data,"end", MyStrlen("end")) || MyStrcmp(my_msg.data, "000", MyStrlen("000")))
			{
				MyStrcpy(my_msg.data, "Thank you for playing. Come again!\n \0");
				my_msg.recipient = port_data.stdout_pid;
				MsgSnd(&my_msg);			
				MsgRcv(&my_msg);
				break;
			}
			else if(MyStrcmp(my_msg.data,"dir", MyStrlen("dir")) || MyStrcmp(my_msg.data, "111", MyStrlen("111")))
			{
				DirSub(my_msg.data, FileServicePid);
				continue;
			}
			else if(MyStrcmp(my_msg.data,"cat", MyStrlen("cat")) || MyStrcmp(my_msg.data, "222", MyStrlen("222")))
			{
				CatSub(my_msg.data, FileServicePid);
				continue;
			}
			//phase 8
			else
			{
				my_msg.recipient = FileServicePid;
				my_msg.code[0] = CHK_OBJ;
   				MsgSnd(&my_msg);
   				MsgRcv(&my_msg);	
				p = (attr_t *) my_msg.data;

  				if((my_msg.code[0] != GOOD) || (p->mode != MODE_EXEC)) 
				{
      					MyStrcpy(my_msg.data, "Illegal Message!!!\n");
      					my_msg.recipient = port_data.stdout_pid;
      					MsgSnd(&my_msg);
      					MsgRcv(&my_msg);
					continue;				
				}
				child_pid = Fork(p->data, p->size);
				
				sprintf(my_msg.data, "\nForked, Child PID %d...\n", child_pid);	
				my_msg.recipient = port_data.stdout_pid;
				MsgSnd(&my_msg);
      				MsgRcv(&my_msg);
				if(child_pid == -1)
				{				
					continue;
				}	
				child_pid = Wait(&exit_code);
				
				
				sprintf(my_msg.data, "\nChild PID %d exits, exit code is %d.\n", child_pid, exit_code);	
				my_msg.recipient = port_data.stdout_pid;
				MsgSnd(&my_msg);
      				MsgRcv(&my_msg);	
			}		
		}
	}
}

void StdinProc()
{
	msg_t my_msg;
	char *p, ch;
	
	while(1)// forever loop
	{
	MsgRcv(&my_msg);			//receive my_msg
	p = my_msg.data;			//char ptr p points to my_msg.data

		while(1)
		{
			SemWait(port_data.RX_semaphore); // semaphore wait on RX_semaphore of port data	
			ch = DeQ(&port_data.RX_buffer);	// ch = dequeue from RX_buffer of port data
			if (ch == '\r')			// if ch is '\r', break; // delimier encountered
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
		MsgRcv(&my_msg);		// receive my_msg (from user shell)
		p = my_msg.data;
			
		while(*p != '\0')	
		{
			SemWait(port_data.TX_semaphore);// semaphore wait on TX_semaphore of port data	
			EnQ(*p, &port_data.TX_buffer);//enqueue what p points to TX_buffer of port data
			if( *p == '\n')
			{
				SemWait(port_data.TX_semaphore); 	// semaphore wait on TX_semaphore of port data
				EnQ('\r', &port_data.TX_buffer);	// enqueue '/r' to TX_buffer of port data
			}
			TipIRQ3(); 	//issue syscall "TipIRQ3();" to manually start IRQ-3 event
			p++;
		}
		my_msg.recipient = my_msg.sender;
		MsgSnd(&my_msg);
	}
}

//////////////////////////////////////Phase 7

void DirStr(attr_t *p, char *str)		
{	
	//my_msg.data has 2 parts: attr_t and targe, p+1 points to target
	char *target = (char *)(p + 1);

	//build str from attr_t p poins to
	sprintf(str, " - - - -  size =%6d    %s\n", p->size, target);
	if ( A_ISDIR(p->mode) ) str[1] = 'D';         // mode is directory
   	if ( QBIT_ON(p->mode, A_ROTH) ) str[3] = 'R'; // mode is readable
   	if ( QBIT_ON(p->mode, A_WOTH) ) str[5] = 'W'; // mode is writable
   	if ( QBIT_ON(p->mode, A_XOTH) ) str[7] = 'X'; // mode is executable
}

void DirSub(char *cmd_str, int FileServicePid)
{
// make sure cmd_str ends with \0: "dir\0" or "dir obj...\0"
	char str[101];
	msg_t my_msg;
	attr_t *p;

	// if cmd_str is "dir" assume "dir /\0" (on root dir)
	if(cmd_str[3] == ' ')
	{
		cmd_str += 4;		//skip 1st 4 leters "dir " and get teh rest: obj...
	}
	// else, assume user specified an target after first 4 letters "dir "
	else
	{
		cmd_str[0] = '/';
		cmd_str[1] = '\0';	// null-erminate the target
	}

	//apply standard "check target" protocol
	my_msg.code[0] = CHK_OBJ;	
	MyStrcpy(my_msg.data, cmd_str);

	my_msg.recipient = FileServicePid;
	MsgSnd(&my_msg);			//send my_msg to FileServicePid
	MsgRcv(&my_msg);		//receive reply

	if(my_msg.code[0] != GOOD)	//chk result code
	{
		MyStrcpy(my_msg.data, "DirSub: CHK_OJB reurn code is not GOOD!\n\0");	

		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		MsgRcv(&my_msg);

		return;
	}

	p = (attr_t *)my_msg.data;	//otherwise, code[0] good, my_msg has "attr_t,"

	if(! A_ISDIR(p->mode))			//if it's file, "dir" it
	{
		DirStr(p, str);			//str will be built and returned
		MyStrcpy(my_msg.data, str); 	//go about since p pointed to my_msg.data
     		my_msg.recipient = port_data.stdout_pid;
     		MsgSnd(&my_msg);
      		MsgRcv(&my_msg);

		return;
	}

// otherwise, it's a DIRECTORY! -- list each entry in it in loop.
// 1st request to open it, then issue reads in loop


// apply standard "open target" protocol
	my_msg.code[0] = OPEN_OBJ;
	MyStrcpy(my_msg.data, cmd_str);
	my_msg.recipient = FileServicePid;
	MsgSnd(&my_msg);
	MsgRcv(&my_msg);

   	while(1)                     		//apply standard "read obj" protocol
	{   		
		my_msg.code[0] = READ_OBJ;
     	 	my_msg.recipient = FileServicePid;
      		MsgSnd(&my_msg);
      		MsgRcv(&my_msg);
		
		if(my_msg.code[0] != GOOD)
		{
			break;			//EOF
		}

// do same thing to show it via STANDOUT
	
		p = (attr_t *)my_msg.data;
      		DirStr(p, str);                //str will be built and returned
      		MyStrcpy(my_msg.data, str);
      		my_msg.recipient = port_data.stdout_pid;
      		MsgSnd(&my_msg);  	      //show str onto terminal
      		MsgRcv(&my_msg);
  	}

//apply "close obj" protocol with FileServicePid
//if response is not good, display an error my_msg via por_data.stdout_pid ...
 
  		my_msg.code[0] = CLOSE_OBJ;
   		my_msg.recipient = FileServicePid;
   		MsgSnd(&my_msg);
   		MsgRcv(&my_msg);	

  		if(my_msg.code[0] != GOOD) 
		{
      			MyStrcpy(my_msg.data, "Dir: CLOSE_OBJ returns NOT GOOD!\n\0");
      			my_msg.recipient = port_data.stdout_pid;
      			MsgSnd(&my_msg);
      			MsgRcv(&my_msg);
		}
}


// "cat" command, ShellProc talks to FileServicePid and port_data.stdout_pid
// make sure cmd_str ends with \0: "cat file\0"
void CatSub(char *cmd_str, int FileServicePid) 
{
	msg_t my_msg;
	attr_t *p;
	
	cmd_str += 4; 		//skip 1st 4 letters "cat " and get the rest

	//apply standard "check target" protocol
	my_msg.code[0] = CHK_OBJ;
	MyStrcpy(my_msg.data, cmd_str);
	
	my_msg.recipient = FileServicePid;
	MsgSnd(&my_msg);			//send my_msg to FileServicePID
	MsgRcv(&my_msg);			//receive reply

	p = (attr_t *)my_msg.data;		//otherwise, code[0] good, chck attr_t

	if(my_msg.code[0] != GOOD || A_ISDIR(p->mode) )	 // if directory
	{
		MyStrcpy(my_msg.data, "Usage: cat [path]filename\n\0");
		my_msg.recipient = port_data.stdout_pid;
		MsgSnd(&my_msg);
		MsgRcv(&my_msg);
      
		return;
	}

//1st request to open it, then issue reads in loop

//apply standard "open obj: protocol
	my_msg.code[0] = OPEN_OBJ;
   	MyStrcpy(my_msg.data, cmd_str);
   	my_msg.recipient = FileServicePid;
   	MsgSnd(&my_msg);
   	MsgRcv(&my_msg);

	while(1)
	{
		// apply standard "read target" protocol
		my_msg.code[0] = READ_OBJ;
      		my_msg.recipient = FileServicePid;
      		MsgSnd(&my_msg);
      		MsgRcv(&my_msg);

		// did it read OK? if not, then break loop ...
		if(my_msg.code[0] != GOOD)
		{
			break;
		}

		// otherwise, show file content via port_data.stdout_pid
      		my_msg.recipient = port_data.stdout_pid;
      		MsgSnd(&my_msg);  // show str onto terminal
      		MsgRcv(&my_msg);
	}

		// apply standard "close target" protocol with FileServicePid
     		my_msg.code[0] = CLOSE_OBJ;
   		my_msg.recipient = FileServicePid;
   		MsgSnd(&my_msg);
   		MsgRcv(&my_msg);

   		// if return code is not good, show error msg onto terminal
		if(my_msg.code[0] != GOOD)
		{	
			MyStrcpy(my_msg.data, "Dir: CLOSE_OBJ returns NOT GOOD!\n\0");
      			my_msg.recipient = port_data.stdout_pid;
      			MsgSnd(&my_msg);
      			MsgRcv(&my_msg);
		}
}
		




