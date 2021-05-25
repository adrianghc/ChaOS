
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Math utility functions.
 * 
 * Some nice tricks at https://graphics.stanford.edu/~seander/bithacks.html.
 */


#include "lib/math.h"
#include "lib/inttypes.h"


/**
 * Modulo function.
 * 
 * @param dividend  The dividend
 * @param divisor   The divisor
 * 
 * @return          Dividend modulo divisor
 */
__attribute__((always_inline))
__attribute__((section(".lib")))
inline uint32_t math_mod(uint32_t dividend, uint32_t divisor) {

    // fast path
    uint32_t mask = divisor - 1;
    if (divisor && !(divisor & mask)) {
        return dividend & mask;
    }

    while (dividend >= divisor) {
        dividend -= divisor;
    }
    return dividend;

}

/**
 * Division function.
 * 
 * @param dividend  The dividend
 * @param divisor   The divisor
 * 
 * @return          Dividend / divisor
 */
__attribute__((always_inline))
__attribute__((section(".lib")))
inline uint32_t math_div(uint32_t dividend, uint32_t divisor) {

    uint32_t i = 0;

    // fast path
    uint32_t mask = divisor - 1;
    if (divisor && !(divisor & mask)) {
        return dividend >> math_log2(divisor);
    }

    while (dividend >= divisor) {
        i++;
        dividend -= divisor;
    }

    return i;

}

/**
 * The logarithm to base 2.
 * Only use if v is a power of 2.
 * 
 * @param v         The number to get the 2-logarithm of
 * 
 * @return          log2(v)
 */
__attribute__((always_inline))
__attribute__((section(".lib")))
inline uint32_t math_log2(uint32_t v) {

    uint32_t r = (v & 0xAAAAAAAA) != 0;
    r |= ((v & 0xFFFF0000) != 0) << 4;
    r |= ((v & 0xFF00FF00) != 0) << 3;
    r |= ((v & 0xF0F0F0F0) != 0) << 2;
    r |= ((v & 0xCCCCCCCC) != 0) << 1;

    return r;

}
