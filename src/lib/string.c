
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Standard application library for string operations.
 */


#include "lib/string.h"
#include "lib/inttypes.h"


/* Holds all hex characters in one place. */
const char* hex = "0123456789ABCDEF";


/**
 * Writes a given number as its hex representation into a given target buffer. 
 * 
 * @param target    Pointer to the target buffer for the hex representation
 * @param arg       The number to be converted to its hex representation
 */
__attribute__((section(".lib")))
void to_hex(char* target, uint32_t arg) {

    int i;
    for (i = 2 * sizeof(uint32_t) - 1; i >= 0; i--) {
        *(target++) = hex[(arg>>(4*i))&0xF];
    }

}

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
__attribute__((section(".lib")))
size_t interpolate_core(char* target, size_t cap, const char* format, void* cur_arg) {

    char cur_char;
    size_t target_idx = 0;
    int8_t is_special = 0;
    uint32_t i;
    uint32_t word;
    char* string_ptr;

    for (i = 0; format[i] && cap; i++) {
        cur_char = format[i];
        if (is_special) {
            /* Handle interpolations */
            switch (cur_char) {

            case '%': // user wanted a plain %, ez
                target[target_idx++] = '%';
                --cap;
                break;

            case 'c':
                word = *(int*)cur_arg;
                cur_arg += sizeof(int32_t);
                target[target_idx++] = (char)word;
                --cap;
                break;

            case 's':
                string_ptr = *(char**)cur_arg;
                cur_arg += sizeof(char*);
                while (*string_ptr && cap) {
                    target[target_idx++] = *(string_ptr++);
                    --cap;
                }
                break;

            case 'x':
            case 'p':
                word = *(int*)(cur_arg);
                cur_arg += sizeof(uint32_t);

                if (cap < sizeof(uint32_t)*2) {
                    // The HEX number will not fit into the buffer, abort!
                    cap = 0;
                    break;
                }

                to_hex(&target[target_idx], word);
                target_idx += sizeof(uint32_t) * 2; // two hex chars per byte
                break;

            default: // not an interpolation
                target[target_idx++] = '%';
                --cap;

                if (cap--) {
                    target[target_idx++] = cur_char;
                }
                break;
            }

            is_special = 0;
            continue;
        }

        if (cur_char == '%') {
            is_special = 1;
            continue;
        }

        if (cap--) {
            target[target_idx++] = cur_char;
        }

    }

    if (cap) {
        target[target_idx++] = 0; // 0 terminator
    }
    return target_idx; // return the length in the buffer

}

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
__attribute__((section(".lib")))
size_t interpolate(char* target, size_t cap, const char* format, ...) {
    return interpolate_core(target, cap, format, &format + 1);
}

/**
 * Calculates the length of a string as long as it's lower or equal to the given maximum length.
 * If the string is longer than @param maxlen, maxlen will be returned as the length.
 * 
 * @param str       Pointer to the string
 * @param maxlen    The maximum length of the string
 * 
 * @return          The length of the string or @param maxlen if the length exceeds it
 */
__attribute__((section(".lib")))
size_t strnlen(char* str, size_t maxlen) {
    size_t len = 0;
    while (len != maxlen && *(str++)) {
        len++;
    }
    return len;
}
