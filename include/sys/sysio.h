
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for string operations, e.g. printing to the console or interpolating with arguments.
 */


#include "lib/inttypes.h"


#ifndef SYSIO_H_
#define SYSIO_H_


/**
 * Prints a string given as a format string with any number of arguments.
 * Use only in Interupt Service Routines!
 * 
 * @param format    Pointer to the format string
 * @param ...       Format string arguments
 */
__attribute__((format(printf, 1, 2)))
void printf_isr(const char* format, ...);


#endif /* SYSIO_H_ */
