
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Math utility functions.
 * 
 * Some nice tricks at https://graphics.stanford.edu/~seander/bithacks.html.
 */


#include "lib/inttypes.h"


#ifndef MATH_H_
#define MATH_H_


/**
 * Modulo function.
 * 
 * @param dividend  The dividend
 * @param divisor   The divisor
 * 
 * @return          Dividend modulo divisor
 */
uint32_t math_mod(uint32_t dividend, uint32_t divisor);

/**
 * Division function.
 * 
 * @param dividend  The dividend
 * @param divisor   The divisor
 * 
 * @return          Dividend / divisor
 */
uint32_t math_div(uint32_t dividend, uint32_t divisor);

/**
 * The logarithm to base 2.
 * Only use if v is a power of 2.
 * 
 * @param v         The number to get the logarithm of
 * 
 * @return          log2(v)
 */
uint32_t math_log2(uint32_t v);


#endif /* MATH_H_ */
