
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Utility functions that can be used for driver code.
 */


#include "drivers/util.h"
#include "lib/inttypes.h"


/* BEGIN Functions to read and write to memory directly */

inline void write_u32(uint32_t base, uint32_t offset, uint32_t val) {
    *(volatile uint32_t *)(base + offset) = val;
}

inline uint32_t read_u32(uint32_t base, uint32_t offset) {
    return *(volatile uint32_t *)(base + offset);
}

inline void write_u16(uint32_t base, uint32_t offset, uint16_t val) {
    *(volatile uint16_t *)(base + offset) = val;
}

inline uint16_t read_u16(uint32_t base, uint32_t offset) {
    return *(volatile uint16_t *)(base + offset);
}

inline void write_u8(uint32_t base, uint32_t offset, uint8_t val) {
    *(volatile uint8_t *)(base + offset) = val;
}

inline uint8_t read_u8(uint32_t base, uint32_t offset) {
    return *(volatile uint8_t *)(base + offset);
}

/* END Functions to read and write to memory directly */
