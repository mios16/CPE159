// toolfunc.c, 159

#include "spede.h"
#include "typedef.h"
#include "extern.h"

void MyBzero(char *p, int byte_size) 	// clears queue
{	
	while(byte_size--)
	{
		*p++ = 0;
	}
}

void EnQ(int pid, q_t *p) 
{
	if(p->len == Q_LEN)	// show error msg and return if queue's already full
	{
		cons_printf("Queue is full!!\n");
		return;		
	}

	p->q[p->tail]= pid;
	p->tail++;
	p->len++;

	if(p->tail == Q_LEN)
	{
		p->tail = 0;	//wrap around 
	}
}

int DeQ(q_t *p) 
{ 	
//If the length is already 0 for that q struct
	
	int data;
	if(p->len == 0)		// return -1 if q is empty
	{
		return -1;		
	}
		
	data = p->q[p->head];	//store value pointer is pointing to in data
	p->head++;
	p->len--;
	if(p->head == Q_LEN)	//wrap around
	{
		p->head = 0;
	}
	return data;
}




