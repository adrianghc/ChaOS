
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for system calls via SWI.
 */


#include "lib/inttypes.h"
#include "sys/thread.h"


#ifndef SWI_H_
#define SWI_H_


#define SWI_STR_WRITE       0x10
#define SWI_STR_READ        0x11
#define SWI_STR_READ_FLUSH  0x12
#define SWI_GETC            0x1A

#define SWI_THREAD_YIELD    0x20
#define SWI_THREAD_EXIT     0x21
#define SWI_THREAD_CREATE   0x22
#define SWI_THREAD_SLEEP    0x23

#define SWI_MEM_MAP         0x30


/* BEGIN System call functions */

/* BEGIN I/O system calls */

void swi_str_write(struct thread_tcb*);

void swi_str_read(struct thread_tcb*);

void swi_str_read_resume(struct thread_tcb*);

void swi_str_read_flush(struct thread_tcb* tcb);

void swi_getc(struct thread_tcb* tcb);

void swi_getc_resume(struct thread_tcb* tcb, char c);

/* END I/O system calls */


/* BEGIN Thread management system calls */

void swi_thread_yield(struct thread_tcb* tcb);

void swi_thread_exit(struct thread_tcb*);

void swi_thread_create(struct thread_tcb*);

void swi_thread_sleep(struct thread_tcb*);

/* END Thread management system calls */


/* BEGIN Memory management system calls */

void swi_mem_map(struct thread_tcb*);

/* END Memory management system calls */


/* BEGIN System call management tables */

extern uint32_t swi_types[];

extern void* swi_functions[];

/* END System call management tables */


#endif /* SWI_H_ */
