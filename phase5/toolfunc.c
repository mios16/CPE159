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

////////////////////////////////////////////phase 4

void MsgEnQ(msg_t *msg_ptr, msg_q_t *msg_q_ptr)
{
	if(msg_q_ptr->len == Q_LEN)	// show error msg and return if queue's already full
	{
		cons_printf("Queue is full!!\n");
		return;		
	}
	msg_q_ptr->msg_q[msg_q_ptr->tail] = *msg_ptr;
	msg_q_ptr->tail++;
	msg_q_ptr->len++;
	
	if(msg_q_ptr->tail == Q_LEN)
	{
		msg_q_ptr->tail = 0;	//wrap around 
	}
}

void *MsgDeQ(msg_q_t *msg_q_ptr)
{
	msg_t* ret_ptr;
	
	if(msg_q_ptr->len == 0)		// return -1 if q is empty
	{
		cons_printf("Message Queue is empty!!\n");
		return 0;		
	}
	ret_ptr = &(msg_q_ptr->msg_q[msg_q_ptr->head]);
	msg_q_ptr->len--;
	msg_q_ptr->head++;

	if(msg_q_ptr->head == Q_LEN)	//wrap around
	{
		msg_q_ptr->head = 0;
	}	
	return ret_ptr;
}

//phase 5
void MyStrcpy(char *dest, char *src)
{
	while(*dest = *src)
	{
		dest++;
		src++;
	}
}	









