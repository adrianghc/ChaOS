
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Standard application library for input and output.
 */


#include "lib/stdio.h"
#include "lib/string.h"


/**
 * Prints a string given as a format string with any number of arguments.
 * 
 * @param format    Pointer to the format string
 * @param ...       Format string arguments
 * 
 * @return          The number of characters printed, or -1 if there was an error
 */
__attribute__((format(printf, 1, 2)))
__attribute__((section(".lib")))
int printf(const char* format, ...) {

    #define MAXSIZE 512

    /* Retrieve the number of arguments by parsing the format string for % chars */
    char target[MAXSIZE]; // This is the target buffer for our final interpolated string
    size_t size; // This will be the number of bytes written

    size = interpolate_core(target, MAXSIZE, format, &format + 1);
    target[MAXSIZE - 1] = 0; // Ensure there is a 0 terminator

    if (write_string(target, size-1) != size-1) {
        return -1;
    }

    return size;

    #undef MAXSIZE

}

/**
 * Reads a given number of bytes from the standard input.
 * 
 * @param target    Pointer to the buffer to read bytes to
 * @param size      The number of bytes to read
 * 
 * @return          The number of bytes read
 */
__attribute__((section(".lib")))
size_t read_string(char* target, size_t size) {
    size_t c = 0;
    asm volatile(
        "mov r7, %[target] \n"
        "mov r8, %[size] \n"
        "swi 0x11 \n"
        "mov %[c], r7"
        : [c] "=r" (c)
        : [target] "r" (target), [size] "r" (size)
        : "r7", "r8"
    );
    return c;
}

/**
 * Flushes the input buffer.
 */
__attribute__((section(".lib")))
void read_flush(void) {
    asm volatile(
        "swi 0x12 \n"
    );
}

/**
 * Outputs next keyboard input.
 * 
 * @return          Next keyinput as char
 */
__attribute__((section(".lib")))
char getc(void) {
    uint32_t c = 0;
    asm volatile(
        "swi 0x1a \n"
        "mov %[c], r7"
        : [c] "=r" (c)
        :
        : "r7"
    );
    return (char) c;
}

/**
 * Writes a given number of bytes into the standard output.
 * 
 * @param source    Pointer to the buffer to write bytes from
 * @param size      The number of bytes to write
 * 
 * @return          The number of bytes written
 */
__attribute__((section(".lib")))
size_t write_string(char* source, size_t size) {
    size_t len;
    asm volatile(
        "mov r7, %[source] \n"
        "mov r8, %[size] \n"
        "swi 0x10 \n"
        "mov %[len], r7"
        : [len] "=r" (len)
        : [source] "r" (source), [size] "r" (size)
        : "r7", "r8"
    );
    return len;
}
