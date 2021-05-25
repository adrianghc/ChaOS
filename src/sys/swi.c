
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for system calls via SWI.
 */


#include "sys/swi.h"
#include "drivers/util.h"
#include "lib/string.h"
#include "sys/io.h"
#include "sys/memmgmt.h"


/* BEGIN System call functions */

/* BEGIN I/O system calls and system call helper functions */

void swi_str_write(struct thread_tcb* tcb) {

    // Read the input parameters
    char* target = (char*)tcb->r[7];
    size_t length = (size_t)tcb->r[8];

    size_t size = io_dbgu_write_output_string(target, length);

    // Write the output parameters
    tcb->r[7] = (uint32_t)size;

}

void swi_str_read(struct thread_tcb* tcb) {

    // Read the input parameters
    char* target = (char*)tcb->r[7];
    size_t length = (size_t)tcb->r[8];
    size_t size;

    if (!length) {
        tcb->r[7] = 0;
        return;
    }

    size = io_dbgu_read_input_string(target, length);
    if (!size) {
        thread_block_for_input(tcb);
        thread_select();
        return;
    }

    // Write the output parameters
    tcb->r[7] = (uint32_t)size;

}

void swi_str_read_resume(struct thread_tcb* tcb) {

    // Read the input parameters
    char* target = (char*)tcb->r[7];
    size_t length = (size_t)tcb->r[8];

    // This call will always succeed because it's called from the interrupt handler
    size_t size = (uint32_t)io_dbgu_read_input_string(target, length);

    // Write the output parameters
    tcb->r[7] = (uint32_t)size;

}

void swi_str_read_flush(struct thread_tcb* tcb) {
    UNUSED(tcb);
    io_dbgu_read_flush();
}

void swi_getc(struct thread_tcb* tcb) {
    thread_block_for_char(tcb);
    thread_select();
}

void swi_getc_resume(struct thread_tcb* tcb, char c) {
    tcb->r[7] = c;
}

/* END I/O system calls and system call helper functions */


/* BEGIN Thread management system calls */

void swi_thread_yield(struct thread_tcb* tcb) {
    UNUSED(tcb);
    thread_select();
}

void swi_thread_exit(struct thread_tcb* tcb) {
    thread_exit(tcb, (int32_t)tcb->r[7]);
    thread_select();
}

void swi_thread_create(struct thread_tcb* tcb) {

    struct thread_tcb* child;
    uint8_t i;

    child = thread_create((void*)tcb->r[7], tcb->id, tcb->r[8], 0);

    // Pass starting parameters to child
    for (i = 0; i < 2; i++) {
        child->r[i] = tcb->r[9+i];
    }
    thread_activate(child->id);

    // Write the output parameters
    tcb->r[7] = child->id;

}

void swi_thread_sleep(struct thread_tcb* tcb) {
    thread_block_for_timer(tcb);
    thread_select();
}

/* END Thread management system calls */


/* BEGIN Memory management system calls */

void swi_mem_map(struct thread_tcb* tcb) {
    uint32_t from = tcb->r[7];
    if (from < EXT_RAM + 5*MB) {
        tcb->r[7] = 0;
        return;
    }

    tcb->r[7] = (uint32_t)memmgmt_map_any(tcb->ttb, tcb->r[7], 1, 1);
}

/* END Memory management system calls */


/* BEGIN System call management tables */

uint32_t swi_types[] = {
    SWI_STR_WRITE,
    SWI_STR_READ,
    SWI_STR_READ_FLUSH,
    SWI_GETC,
    SWI_THREAD_YIELD,
    SWI_THREAD_EXIT,
    SWI_THREAD_CREATE,
    SWI_THREAD_SLEEP,
    SWI_MEM_MAP,
    0x00
};

void* swi_functions[] = {
    &swi_str_write,
    &swi_str_read,
    &swi_str_read_flush,
    &swi_getc,
    &swi_thread_yield,
    &swi_thread_exit,
    &swi_thread_create,
    &swi_thread_sleep,
    &swi_mem_map
};

/* END System call management tables */


/* END System call functions */
