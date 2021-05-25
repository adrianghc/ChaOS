
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions to interact with the CP15 coprocessor, e.g. to enable the MMU.
 * 
 * Documentation source: doc/Atmel/AT91RM9200.pdf
 * CP15 description on pages 41 - 58
 */


#include "drivers/cp15.h"
#include "lib/inttypes.h"


/* BEGIN Functions for MMU, domain access and TTB management */

/**
 * Enables the MMU.
 */
void cp15_mmu_enable(void) {

    asm volatile (
        "mrc p15, 0, r7, c1, c0, 0 \n"
        "orr r7, #1 \n"
        "mcr p15, 0, r7, c1, c0, 0 \n"
        : : : "r7"
    );

}

/**
 * Disables the MMU.
 */
void cp15_mmu_disable(void) {

    asm volatile (
        "mrc p15, 0, r7, c1, c0, 0 \n"
        "and r7, #0xFFFFFFFE \n"
        "mcr p15, 0, r7, c1, c0, 0 \n"
        : : : "r7"
    );

}

/**
 * Initializes the Domain Access Control Register.
 */
void cp15_init_domains(void) {

    /*
     * The CP 15 Register 3, or Domain Access Control Register, defines the domain's access
     * permission.
     * MMU accesses are controlled through the use of 16 domains.
     * Each field of register 3 is associated with one domain.
     * 
     * The 2-bit field value allows domain access as described in the table below.
     * 
     * Value    Access      Description
     * --------------------------------------------------------
     * 00       No access   Any access generates a domain fault
     * 01       Client      Accesses are checked against the access permission 
     *                      bits in the section or page descriptor
     * 10       Reserved    Reserved. Currently behaves like the no access mode
     * 11       Manager     Accesses are not checked against the access permission
     *                      bits so a permission fault cannot be generated
     */

    // Currently we are not interested in a case where we wouldn't want to check accesses
    // against the access permission, so we define one single domain where this is the case.
    asm volatile (
        "mov r1, #1 \n"
        "mcr p15, 0, r1, c3, c0, 0 \n"
        : : : "r1"
    );

}

/**
 * Writes the address of the translation table base to the MMU.
 * 
 * @param ptr       The address of the TTB
 */
void cp15_write_translation_table_base(uint32_t* ptr) {

    ptr = (uint32_t*) ((uint32_t)ptr & 0xFFFFC000);
    asm volatile (
        "mov r7, %[ptr] \n"
        "mcr p15, 0, r7, c2, c0, 0 \n"
        :
        : [ptr] "r" (ptr)
        : "r7"
    );

}

/* END Functions for MMU, domain access and TTB management */


/* BEGIN Functions for cache management */

/**
 * Enables the data cache.
 */
void cp15_enable_dcache(void) {

    asm volatile (
        "mrc p15, 0, r7, c1, c0, 0 \n"
        "orr r7, #4 \n"
        "mcr p15, 0, r7, c1, c0, 0 \n"
        : : : "r7"
    );

}

/**
 * Disables the data cache.
 */
void cp15_disable_dcache(void) {

    asm volatile (
        "mrc p15, 0, r7, c1, c0, 0 \n"
        "and r7, #0xFFFFFFFB \n"
        "mcr p15, 0, r7, c1, c0, 0 \n"
        : : : "r7"
    );

}

/**
 * Enables the instruction cache.
 */
void cp15_enable_icache(void) {

    asm volatile (
        "mrc p15, 0, r7, c1, c0, 0 \n"
        "orr r7, #0x1000 \n"
        "mcr p15, 0, r7, c1, c0, 0 \n"
        : : : "r7"
    );

}

/**
 * Disables the instruction cache.
 */
void cp15_disable_icache(void) {

    asm volatile (
        "mrc p15, 0, r7, c1, c0, 0 \n"
        "and r7, #0xFFFFEFFF \n"
        "mcr p15, 0, r7, c1, c0, 0 \n"
        : : : "r7"
    );

}

/**
 * Invalidates the data cache.
 */
void cp15_invalidate_dcache(void) {

    asm volatile (
        "mcr p15, 0, r12, c7, c6, 0 \n"
        : : :
    );

}

/**
 * Invalidates the instruction cache.
 */
void cp15_invalidate_icache(void) {

    asm volatile (
        "mcr p15, 0, r12, c7, c5, 0 \n"
        : : :
    );

}

/**
 * Invalidates both caches.
 */
void cp15_invalidate_caches(void) {

    asm volatile (
        "mcr p15, 0, r12, c7, c7, 0 \n"
        : : :
    );

}

/* END Functions for cache management */


/* BEGIN Functions for TLB management */

/**
 * Invalidates the data Translation Lookaside Buffer.
 */
void cp15_invalidate_dtlb(void) {

    asm volatile (
        "mcr p15, 0, r12, c8, c6, 0 \n"
        : : :
    );

}

/**
 * Invalidates the instruction Translation Lookaside Buffer.
 */
void cp15_invalidate_itlb(void) {

    asm volatile (
        "mcr p15, 0, r12, c8, c5, 0 \n"
        : : :
    );

}

/**
 * Invalidates both Translation Lookaside Buffers.
 */
void cp15_invalidate_tlb(void) {

    asm volatile (
        "mcr p15, 0, r12, c8, c7, 0 \n"
        : : :
    );

}

/* END Functions for TLB management */


/* BEGIN Functions for fault management */

/**
 * Returns the virtual address of the access which was attempted when fault occurred.
 */
uint32_t cp15_read_fault_address(void) {

    uint32_t ret;
    asm volatile (
        "mrc p15, 0, r7, c6, c0, 0 \n"
        "mov %[ret], r7"
        : [ret] "=r" (ret)
        :
        : "r7"
    );
    return ret;

}

/* END Functions for fault management */
