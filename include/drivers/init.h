
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions to initialize the system, i.e. stacks and the Interrupt Vector Table.
 */


#include "drivers/mc.h"
#include "drivers/interrupt.h"
#include "drivers/util.h"
#include "lib/inttypes.h"
#include "lib/mem.h"


#ifndef INIT_H_
#define INIT_H_


#define IVT_ADDR        INT_RAM
#define ISR_OFFSET      32
#define LOAD_PC         0xE59FF000 + ISR_OFFSET - 8

#define I_BIT           0x80
#define F_BIT           0x40

#define ARM_MODE_FIQ    0b10001 | I_BIT | F_BIT
#define ARM_MODE_IRQ    0b10010 | I_BIT | F_BIT
#define ARM_MODE_SVC    0b10011 | I_BIT | F_BIT
#define ARM_MODE_ABT    0b10111 | I_BIT | F_BIT
#define ARM_MODE_UND    0b11011 | I_BIT | F_BIT
#define ARM_MODE_SYS    0b11111 | I_BIT | F_BIT



/**
 * Creates Interrupt Vector Table.
 */
__attribute__((always_inline))
inline void init_ivt(void) {

    uint32_t* ivt = (uint32_t*) INT_RAM;
    *(ivt++)        = LOAD_PC;
    *(ivt++)        = LOAD_PC;
    *(ivt++)        = LOAD_PC;
    *(ivt++)        = LOAD_PC;
    *(ivt++)        = LOAD_PC;
    *(ivt++)        = LOAD_PC;
    *(ivt)          = LOAD_PC;

    uint32_t* ivt_big = (uint32_t*) (INT_RAM + ISR_OFFSET);
    *(ivt_big++)    = (uint32_t) &isr_reset;
    *(ivt_big++)    = (uint32_t) &isr_undefined;
    *(ivt_big++)    = (uint32_t) &isr_software_interrupt;
    *(ivt_big++)    = (uint32_t) &isr_prefetch_abort;
    *(ivt_big++)    = (uint32_t) &isr_data_abort;
    *(ivt_big++)    = (uint32_t) &isr_fast_interrupt_request;
    *(ivt_big)      = (uint32_t) &isr_interrupt_request;

    mc_toggle_remap();

}

/**
 * Initializes the stack pointer register for the given mode.
 * 
 * @param mode  The processor mode for which the stack pointer is to be initialized
 * @param ptr   The first address of the stack
 */
__attribute__((always_inline))
inline void init_stack_pointer(uint32_t mode, uint32_t ptr) {
    asm volatile (
        "mrs r3,   CPSR     \n\t"
        "msr CPSR, %[mode]  \n\t"
        "mov sp,   %[ptr]   \n\t"
        "msr CPSR, r3       \n\t"
        :
        : [mode] "r" (mode), [ptr] "r" (ptr)
        : "r3"
    );
}

/**
 * Initializes all stacks available on the system to addresses in the RAM.
 */
__attribute__((always_inline))
inline void init_stacks(void) {

    // Place all interrupt mode stacks at the end of the internal RAM.
    init_stack_pointer(ARM_MODE_FIQ, 0x00204000);
    init_stack_pointer(ARM_MODE_IRQ, 0x00204C00);
    init_stack_pointer(ARM_MODE_SVC, 0x00204800);
    init_stack_pointer(ARM_MODE_ABT, 0x00204400);
    init_stack_pointer(ARM_MODE_UND, 0x00203000);

    // Place the system mode stack at the end of the external RAM.
    init_stack_pointer(ARM_MODE_SYS, 0x24000000);

}


#endif /* INIT_H_ */
