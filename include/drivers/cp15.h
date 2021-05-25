
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions to interact with the CP15 coprocessor, e.g. to enable the MMU.
 * 
 * Documentation source: doc/Atmel/AT91RM9200.pdf
 * CP15 description on pages 41 - 58
 */


#include "lib/inttypes.h"


#ifndef CP15_H_
#define CP15_H_


/* BEGIN Functions for MMU, domain access and TTB management */

/**
 * Enables the MMU.
 */
void cp15_mmu_enable(void);

/**
 * Disables the MMU.
 */
void cp15_mmu_disable(void);

/**
 * Initializes the Domain Access Control Register.
 */
void cp15_init_domains(void);

/**
 * Writes the address of the translation table base to the MMU.
 * 
 * @param ptr       The address of the TTB
 */
void cp15_write_translation_table_base(uint32_t* ptr);

/* END Functions for MMU, domain access and TTB management */


/* BEGIN Functions for cache management */

/**
 * Enables the data cache.
 */
void cp15_enable_dcache(void);

/**
 * Disables the data cache.
 */
void cp15_disable_dcache(void);

/**
 * Enables the instruction cache.
 */
void cp15_enable_icache(void);

/**
 * Disables the instruction cache.
 */
void cp15_disable_icache(void);

/**
 * Invalidates the data cache.
 */
void cp15_invalidate_dcache(void);

/**
 * Invalidates the instruction cache.
 */
void cp15_invalidate_icache(void);

/**
 * Invalidates both caches.
 */
void cp15_invalidate_caches(void);

/* END Functions for cache management */


/* BEGIN Functions for TLB management */

/**
 * Invalidates the data Translation Lookaside Buffer.
 */
void cp15_invalidate_dtlb(void);

/**
 * Invalidates the instruction Translation Lookaside Buffer.
 */
void cp15_invalidate_itlb(void);

/**
 * Invalidates both Translation Lookaside Buffers.
 */
void cp15_invalidate_tlb(void);

/* END Functions for TLB management */


/* BEGIN Functions for fault management */

/**
 * Returns the virtual address of the access which was attempted when fault occurred.
 */
uint32_t cp15_read_fault_address(void);

/* END Functions for fault management */


#endif /* CP15_H_ */
