
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for memory management. 
 */


#include "lib/inttypes.h"


#ifndef MEM_H_
#define MEM_H_


/**
 * Copies a given amount of memory from a source to a destination location.
 * 
 * @param dst   The destination
 * @param src   The source
 * @param size  The number of bytes to be copied
 */
void memcopy(uint8_t* dst, uint8_t* src, size_t size);

/**
 * Fills a given amount of memory in a given location with zeros.
 * 
 * @param dst   The first location to be filled with zeros
 * @param size  The number of bytes to be written with zeros
 */
void memzero(uint8_t* dst, size_t size);

/**
 * Maps a memory segment to the given address.
 * 
 * @param addr  The address to be mapped
 * 
 * @return      Whether the mapping was successful
 */
uint32_t mmap(uint32_t addr);

#endif /* MEM_H_ */
