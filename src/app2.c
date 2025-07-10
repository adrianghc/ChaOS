
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * An application to demonstrate protection against various forbidden actions:
 * 
 * 0 - Accessing a NULL pointer.
 * 1 - Reading kernel data.
 * 2 - Writing into program text in memory.
 * 3 - Overflowing the stack.
 * 4 - Reading from an unmapped address.
 * 5 - Reading from an address that would normally be unmapped.
 */


#include "lib/inttypes.h"
#include "lib/stdio.h"
#include "lib/stdlib.h"
#include "sys/thread.h"


__attribute__((section(".lib")))
void read_null_pointer(void) {
    printf("Show what is at NULL: %x\n", *((uint32_t*)0));
}

__attribute__((section(".lib")))
void read_kernel_data(void) {
    printf("Show what is at _start: %x\n", *((uint32_t*)0x20000000));
}

__attribute__((section(".lib")))
void write_own_text(void) {
    *((uint32_t*)&write_own_text) = 10; 
}

#pragma GCC diagnostic push
// GCC reports an infinite recursion here, this is by design to cause a stack overflow.
#pragma GCC diagnostic ignored "-Winfinite-recursion"
__attribute__((section(".lib")))
void stack_overflow(void) {
    stack_overflow();
}
#pragma GCC diagnostic pop

__attribute__((section(".lib")))
void read_unmapped_address(void) {
    printf("Show what is at someplace unmapped: %x\n", *((uint32_t*)0x1FFFFFFF));
}

__attribute__((section(".lib")))
void read_non_1_to_1_mapped(void) {

    int i = 42;
    printf("Our stack is mapped into normally undefined areas of the address space: %p\n", &i);

}

__attribute__((section(".lib")))
void main(void) {

    printf("Please select the action you want to perform:\n");
    printf("0 - Access a NULL pointer\n");
    printf("1 - Read kernel data\n");
    printf("2 - Write into this program text in memory\n");
    printf("3 - Overflow the stack\n");
    printf("4 - Read from unmapped address\n");
    printf("5 - Read from address that would normally be unmapped\n");
    char c = getc();

    switch (c) {
    case '0':
    default:
        printf("Attempting to read from a NULL pointer.\n");
        read_null_pointer();
        break;

    case '1':
        printf("Attempting to read kernel data pointer.\n");
        read_kernel_data();
        break;

    case '2':
        printf("Attempting to write into own code.\n");
        write_own_text();
        break;

    case '3':
        printf("Overflowing the stack.\n");
        stack_overflow();
        break;

    case '4':
        printf("Attempting to read from an unmapped address.\n");
        read_unmapped_address();
        break;

    case '5':
        printf("Reading and writing to not 1-1 mapped memory.\n");
        read_non_1_to_1_mapped();
        break;
    }

    printf("SUCCESS (unless you chose 5, this should not happen).\n");
    exit(0);

}
