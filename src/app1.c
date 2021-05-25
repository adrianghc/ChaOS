
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * An application to demonstrate address space separation, context switching,
 * system calls and process/thread creation.
 * 
 * An initial process waits for user input and creates a new process when a
 * character is entered (with that character as the process' input).
 * The new process stores the character in its address space, initializes a
 * counter at 0 and starts two new threads in its own address space.
 * All three threads enter a loop where they increase the shared counter and a
 * private counter as long as the former is under a predefined limit, print a
 * message, and sleep for a moment.
 * The printed message is formatted as follows:
 * <letter><thread number>: <global counter value> (<local counter value>)
 */


#include "lib/inttypes.h"
#include "lib/mem.h"
#include "lib/stdio.h"
#include "lib/stdlib.h"
#include "sys/thread.h"


#define APP_ADDR    0x20ADBEEF
#define MAX_PRINTS  16


__attribute__((section(".lib")))
void task(char c, char id) {
    uint16_t* global_counter = (uint16_t*) APP_ADDR;
    uint16_t local_counter = 0;

    while (*global_counter <= MAX_PRINTS) {
        local_counter++;
        (*global_counter)++;
        printf("%c%c: %x (%x)\n", c, id, *global_counter, local_counter);
        sleep(100);
    }
    exit(0);
}

__attribute__((section(".lib")))
void process(char c) {
    uint16_t local_counter = 0;
    uint16_t* global_counter;
    if (!mmap(APP_ADDR)) {
        printf("Error mmap");
    }
    global_counter = (uint16_t*) APP_ADDR;
    *global_counter = 0;

    launch_task(&task, c, '2');
    launch_task(&task, c, '3');

    while (*global_counter <= MAX_PRINTS) {
        local_counter++;
        (*global_counter)++;
        printf("%c1: %x (%x)\n", c, *global_counter, local_counter);
        sleep(100);
    }
    exit(0);
}

__attribute__((section(".lib")))
void main(void) {
    char letter;

    while (1){
        letter = getc();
        launch(&process, letter, 0);
    }

    exit(0);
}
