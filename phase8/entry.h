// entry.h, 159

#ifndef _ENTRY_H_
#define _ENTRY_H_

#include <spede/machine/pic.h>

#define TIMER_INTR 32

//phase 2	add 2 new constants
#define GETPID_INTR 48		// phase 2 constants
#define SLEEP_INTR 49		// phase 2 constants

//phase 3
#define STARTPROC_INTR 50	// phase 3 constants
#define SEMGET_INTR 51		// phase 3 constants
#define SEMWAIT_INTR 52		// phase 3 constants
#define SEMPOST_INTR 53		// phase 3 constants

//phase 4
#define MSGSND_INTR 54		// phase 4 constants
#define MSGRCV_INTR 55		// phase 4 constants

//phase 5
#define IRQ7_INTR 39		// phase 5 constants

//phase 6
#define IRQ3_INTR 35		// phase 6 constants

//phase 8
#define FORK_INTR 56		// phase 8 constants
#define WAIT_INTR 57		// phase 8 constants
#define EXIT_INTR 58		// phase 8 constants

#define KERNEL_CODE 0x08         // kernel's code segment
#define KERNEL_DATA 0x10         // kernel's data segment
#define KERNEL_STACK_SIZE 32768  // kernel's stack byte size




// ISR Entries
#ifndef ASSEMBLER

__BEGIN_DECLS

extern void LoadRun();           // code defined in entry.S
extern void TimerEntry();        // code defined in entry.S
//phase 2 	add 2 new prototypes
extern void GetPidEntry();	// code defined in entry.S Phase 2 prototype
extern void SleepEntry();	// code defined in entry.S Phase 2 prototype

//phase 3 new protoypes
extern void StartProcEntry();	// code defined in entry.S Phase 3 prototype
extern void SemGetEntry();	// code defined in entry.S Phase 3 prototype
extern void SemWaitEntry();	// code defined in entry.S Phase 3 prototype
extern void SemPostEntry();	// code defined in entry.S Phase 3 prototype

//phase 4
extern void MsgSndEntry();	
extern void MsgRcvEntry();

//phase 5
extern void IRQ7Entry();

//phase 6
extern void IRQ3Entry();

//phase 8
extern void ForkEntry();
extern void WaitEntry();
extern void ExitEntry();

__END_DECLS

#endif

#endif
