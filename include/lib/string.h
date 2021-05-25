
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Standard application library for string operations.
 */


#include "lib/inttypes.h"


#ifndef STRING_H_
#define STRING_H_


/**
 * Writes a given number as its hex representation into a given target buffer. 
 * 
 * @param target    Pointer to the target buffer for the hex representation
 * @param arg       The number to be converted to its hex representation
 */
void to_hex(char* target, uint32_t arg);

/**
 * Interpolates a format string with given arguments and writes it into a given target buffer.
 * cur_arg points to the location of the first argument.
 * This function is not meant to be used on its own to avoid mistakes with the addresses.
 * Use a variate function that wraps around it instead, e.g. printf() or interpolate().
 * 
 * @param target    Pointer to the target buffer for the interpolated string
 * @param cap       Capacity of the target buffer
 * @param format    Pointer to the format string
 * @param cur_arg   Pointer to the location of the first argument.
 *                  All arguments must be in a consecutive memory area.
 * 
 * @return          The number of characters in the interpolated string (including null terminator)
 */
size_t interpolate_core(char* target, size_t cap, const char* format, void* cur_arg);

/**
 * Interpolates a format string with given arguments and writes it into a given target buffer.
 * This is a wrapper around interpolate_core() so we can call it as a variate function.
 * 
 * @param target    Pointer to the target buffer for the interpolated string
 * @param format    Pointer to the format string
 * @param ...       Format string arguments
 * 
 * @return          The number of characters in the interpolated string (including null terminator)
 */
__attribute__((format(printf, 3, 4)))
size_t interpolate(char* target, size_t cap, const char* format, ...);

/**
 * Calculates the length of a string as long as it's lower or equal to the given maximum length.
 * If the string is longer than @param maxlen, maxlen will be returned as the length.
 * 
 * @param str       Pointer to the string
 * @param maxlen    The maximum length of the string
 * 
 * @return          The length of the string or @param maxlen if the length exceeds it
 */
size_t strnlen(char* str, size_t maxlen);


#endif /* STRING_H_ */
