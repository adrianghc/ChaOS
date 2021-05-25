
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Standard application library for miscellaneous functions.
 */


#include "lib/stdlib.h"


/* BEGIN Thread management functions */

/**
 * Exits the current thread.
 * 
 * @param status    The exit status
 */
__attribute__((section(".lib")))
void exit(int32_t status) {

    asm volatile(
        "mov r7, %[status] \n"
        "swi 0x21 \n"
        :
        : [status] "r" (status)
        : "r7"
    );

}

/**
 * Launches a new thread. All parameters are optional, depending the signature of @param text.
 * 
 * @param text      A pointer to the new thread's text segment
 * @param param1    The first parameter the new thread's function should be launched with
 * @param param2    The second parameter the new thread's function should be launched with
 * 
 * @return          The new thread's ID
 */
__attribute__((section(".lib")))
uint32_t launch(void* text, uint32_t param1, uint32_t param2) {

    uint32_t child_id;

    asm volatile(
        "mov r7, %[text] \n"
        "mov r8, #0 \n"
        "mov r9, %[param1] \n"
        "mov r10, %[param2] \n"
        "swi 0x22 \n"
        "mov %[child_id], r7"
        : [child_id] "=r" (child_id)
        : [text] "r" (text), [param1] "r" (param1), [param2] "r" (param2)
        : "r7", "r8", "r9", "r10"
    );

    return child_id;

}

/**
 * Launches a new task thread that shares the address space with the thread that called it.
 * 
 * @param text      A pointer to the new task thread's text segment
 * @param param1    The first parameter the new task thread's function should be launched with
 * @param param2    The second parameter the new task thread's function should be launched with
 * 
 * @return          The new task thread's ID
 */
__attribute__((section(".lib")))
uint32_t launch_task(void* text, uint32_t param1, uint32_t param2) {

    uint32_t child_id;

    asm volatile(
        "mov r7, %[text] \n"
        "mov r8, #1 \n"
        "mov r9, %[param1] \n"
        "mov r10, %[param2] \n"
        "swi 0x22 \n"
        "mov %[child_id], r7"
        : [child_id] "=r" (child_id)
        : [text] "r" (text), [param1] "r" (param1), [param2] "r" (param2)
        : "r7", "r8", "r9", "r10"
    );

    return child_id;

}

/**
 * Puts a thread to sleep for a given time.
 * 
 * @param ms        The minimum time in milliseconds the thread should sleep
 * 
 * @return          In case the thread was awoken due to a signal, the amount
 *                  of time the thread could not sleep
 */
__attribute__((section(".lib")))
uint32_t sleep(uint32_t ms) {

    uint32_t remaining;

    asm volatile(
        "mov r7, %[ms] \n"
        "swi 0x23 \n"
        "mov %[remaining], r7"
        : [remaining] "=r" (remaining)
        : [ms] "r" (ms)
        : "r7"
    );

    return remaining;

}

/* END Thread management functions */
