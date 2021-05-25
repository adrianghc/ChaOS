
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for memory management. 
 */


#include "lib/mem.h"
#include "lib/inttypes.h"


/**
 * Copies a given amount of memory from a source to a destination location.
 * 
 * @param dst   The destination
 * @param src   The source
 * @param size  The number of bytes to be copied
 */
__attribute__((section(".lib")))
void memcopy(uint8_t* dst, uint8_t* src, size_t size) {

    for (; size > 0; size--) {
        dst[size] = src[size];
    }

}

/**
 * Fills a given amount of memory in a given location with zeros.
 * 
 * @param dst   The first location to be filled with zeros
 * @param size  The number of bytes to be written with zeros
 */
__attribute__((section(".lib")))
void memzero(uint8_t* dst, size_t size) {

    for (; size > 0; size--) {
        dst[size] = 0;
    }

}

/**
 * Maps a memory segment to the given address.
 * 
 * @param addr  The address to be mapped
 * 
 * @return      Whether the mapping was successful
 */
__attribute__((section(".lib")))
uint32_t mmap(uint32_t addr) {
    uint32_t result;

    asm volatile(
        "mov r7, %[addr] \n"
        "swi 0x30 \n"
        "mov %[result], r7"
        : [result] "=r" (result)
        : [addr] "r" (addr)
        : "r7"
    );
    return result;
}
