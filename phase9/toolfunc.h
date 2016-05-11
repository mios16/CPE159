// toolfunc.h, 159

#ifndef _TOOLFUNC_H
#define _TOOLFUNC_H

#include "typedef.h" // q_t needs be defined in code below

void MyBzero(char *, int);
int DeQ(q_t *);
void EnQ(int, q_t *);

//////////////phase 4
void MsgEnQ(msg_t *, msg_q_t *);
void *MsgDeQ(msg_q_t *);

/////////////phase 5
void MyStrcpy(char *, char *);

////////////phase 7
void MyMemcpy(char *, char *, int);
int MyStrcmp(char *, char *, int);
int MyStrlen(char *);

#endif

