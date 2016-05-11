// extern.h, 159

#ifndef _EXTERN_H_
#define _EXTERN_H_

#include "typedef.h"                // defines q_t, pcb_t, MAX_PROC_NUM, PROC_STACK_SIZE

extern int running_pid;             // PID of currently-running process, -1 means none
extern q_t ready_q, free_q;                        // ready to run, not used proc IDs
extern pcb_t pcb[MAX_PROC_NUM];                    // process table
extern char proc_stack[MAX_PROC_NUM][PROC_STACK_SIZE]; // process runtime stacks

//phase 2
extern int OS_clock;
extern q_t sleep_q;

//phase 3
extern sem_t sem[Q_LEN];
extern q_t sem_q;
//for testing in phase 3 only
//extern int product_sem_id, product_count;

//phase 4
extern msg_q_t msg_q[MAX_PROC_NUM];

//phase 5
extern int printing_semaphore;

//phase 6
extern port_data_t port_data;

#endif
