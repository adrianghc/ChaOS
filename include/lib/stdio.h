
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Standard application library for input and output.
 */


#include "lib/inttypes.h"


#ifndef STDIO_H_
#define STDIO_H_


/**
 * Prints a string given as a format string with any number of arguments.
 * 
 * @param format    Pointer to the format string
 * @param ...       Format string arguments
 * 
 * 
 * @return          The number of characters printed, or -1 if there was an error
 */
__attribute__((format(printf, 1, 2)))
int printf(const char* format, ...);

/**
 * Writes a given number of bytes into the standard output.
 * 
 * @param source    Pointer to the buffer to write bytes from
 * @param size      The number of bytes to write
 * 
 * @return          The number of bytes written
 */
size_t read_string(char*, size_t);

/**
 * Flushes the input buffer.
 */
void read_flush(void);

/**
 * Outputs next keyboard input.
 * 
 * @return          Next keyinput as char
 */
char getc(void);

/**
 * Reads a given number of bytes from the standard input.
 * 
 * @param target    Pointer to the buffer to read bytes to
 * @param size      The number of bytes to read
 * 
 * @return          The number of bytes read
 */
size_t write_string(char*, size_t);

#endif /* STDIO_H_ */
