
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Utility functions and defines that can be used for driver code.
 */


#include "lib/inttypes.h"


#ifndef UTIL_H_
#define UTIL_H_


#define UNUSED(x)           (void)(x)

#define KB                  1024
#define MB                  (1024 * KB)

/* BEGIN Memory organisation */

#define BOOT_MEM            0x00000000
#define INT_ROM             0x00100000
#define INT_ROM_LEN         (128 * KB)
#define INT_RAM             0x00200000
#define INT_RAM_LEN         (16 * KB)
#define USB_HOST_IFACE      0x00300000

#define ALLOC_TABLE         (INT_RAM + INT_RAM_LEN - 2*KB)
#define ALLOC_TABLE_LEN     (2 * 4)
#define ALLOC_TABLE_ENTRIES 2

#define TTB_FIRST_ADDR      (EXT_RAM + 512 * KB)

#define EXT_FLASH           0x10000000
#define EXT_FLASH_LEN       (16 * MB)
#define EXT_RAM             0x20000000
#define EXT_RAM_LEN         (64 * MB)

#define PAGE_SIZE           (1 * MB)

#define CS2                 0x30000000
#define CS3                 0x40000000
#define CS4                 0x50000000
#define CS5                 0x60000000
#define CS6                 0x70000000
#define CS7                 0x80000000

/* END Memory organisation */


/* BEGIN Functions to read and write to memory directly */

void write_u32(uint32_t base, uint32_t offset, uint32_t val);

uint32_t read_u32(uint32_t base, uint32_t offset);

void write_u16(uint32_t base, uint32_t offset, uint16_t val);

uint16_t read_u16(uint32_t base, uint32_t offset);

void write_u8(uint32_t base, uint32_t offset, uint8_t val);

uint8_t read_u8(uint32_t base, uint32_t offset);

/* END Functions to read and write to memory directly */


#endif /* UTIL_H_ */
