
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Standard application library for miscellaneous functions.
 */


#include "lib/inttypes.h"


#ifndef STDLIB_H_
#define STDLIB_H_


/* BEGIN Thread management functions */

/**
 * Exits the current thread.
 * 
 * @param status    The exit status
 */
void exit(int32_t status);

/**
 * Launches a new thread. All parameters are optional, depending the signature of `text`.
 * 
 * @param text      A pointer to the new thread's text segment
 * @param param1    The first parameter the new thread's function should be launched with
 * @param param2    The second parameter the new thread's function should be launched with
 * 
 * @return          The new thread's ID
 */
uint32_t launch(void* text, uint32_t param1, uint32_t param2);

/**
 * Launches a new task thread that shares the address space with the thread that called it.
 * 
 * @param text      A pointer to the new task thread's text segment
 * @param param1    The first parameter the new task thread's function should be launched with
 * @param param2    The second parameter the new task thread's function should be launched with
 * 
 * @return          The new task thread's ID
 */
uint32_t launch_task(void* text, uint32_t param1, uint32_t param2);

/**
 * Puts a thread to sleep for a given time.
 * 
 * @param ms        The minimum time in milliseconds the thread should sleep
 * 
 * @return          In case the thread was awoken due to a signal, the amount
 *                  of time the thread could not sleep
 */
uint32_t sleep(uint32_t ms);

/* END Thread management functions */


#endif /* STDLIB_H_ */
