
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for string operations, e.g. printing to the console or interpolating with arguments.
 */


#include "sys/sysio.h"
#include "drivers/dbgu.h"
#include "lib/inttypes.h"
#include "lib/stdio.h"
#include "lib/string.h"
#include "sys/io.h"


/**
 * Prints a string given as a format string with any number of arguments.
 * Use only in Interupt Service Routines!
 * 
 * @param format    Pointer to the format string
 * @param ...       Format string arguments
 */
__attribute__((format(printf, 1, 2)))
void printf_isr(const char* format, ...) {

    /* Retrieve the number of arguments by parsing the format string for % chars */
    char target[512]; // This is the target buffer for our final interpolated string
    interpolate_core(target, 512, format, &format + 1);
    target[511] = 0; // Ensure there is a 0 terminator

    io_dbgu_write_output_string(target, strnlen(target, 512));

}
