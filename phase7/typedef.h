// typedef.h, 159

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include "TF.h"              // trapframe type TF_t is defined here

#define TIME_LIMIT 10       // max timer count to run
#define MAX_PROC_NUM 20      // max number of processes
#define Q_LEN 20             // queuing capacity
#define PROC_STACK_SIZE 4096 // process runtime stack in bytes

// this is the same as constants defines: UNUSED=0, READY=1, etc.
typedef enum {FREE, READY, RUN, SLEEP, WAIT, ZOMBIE, FORKWAIT} state_t;

typedef struct {             // PCB describes proc image
   state_t state;            // state of process
   int runtime;              // runtime since loaded
   int total_runtime;        // total runtime since created
   TF_t *TF_ptr;             // points to trapframe of process

//////phase 2
   int wake_time;	     // add to pcb_t: int wake_time
} pcb_t;

typedef struct {             // proc queue type
   int head, tail, len;      // where head and tail are, and current length
   int q[Q_LEN];             // indices into q[] array to place or get element
} q_t;

///////////////////phase3
typedef struct {
	int limit;
	q_t wait_q;
} sem_t;

typedef void (*func_ptr_t)(); // void-return function pointer type

///////////////////////////////////////phase4
typedef struct {
	int sender;		// sender PID
	int recipient;		// recipient PID
	int OS_clock;		// send time stamp
	//phase 5
	char data[101];		//just this to pass among process for now
	//phase 7	
	int code[3];		
} msg_t;

typedef struct {
	int head, tail, len;
	msg_t msg_q[Q_LEN];
	q_t wait_q;
} msg_q_t;

////////////////////phase 6
typedef struct {
	q_t 	TX_buffer,		//transmit to terminal
		RX_buffer, 		//receive from terminal
		echo_buffer;		//echo back to terminal
	
	int 	TX_semaphore,		//transmit space count
		RX_semaphore,		//receive data count
		echo_mode,		//if 1, echo back to terminal
		TXRDY,			//if 1, TXRDY event occurred
		stdin_pid,		//StdinProc PID (who's upper half)
	  	stdout_pid;		//StdoutProc PID (who's upper half)
} port_data_t;

#endif
