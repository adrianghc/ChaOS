
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

#define I_BIT 0x80
#define F_BIT 0x40

#define ARM_MODE_FIQ 0b10001 | I_BIT | F_BIT
#define ARM_MODE_IRQ 0b10010 | I_BIT | F_BIT
#define ARM_MODE_SVC 0b10011 | I_BIT | F_BIT
#define ARM_MODE_ABT 0b10111 | I_BIT | F_BIT
#define ARM_MODE_UND 0b11011 | I_BIT | F_BIT
#define ARM_MODE_SYS 0b11111 | I_BIT | F_BIT



/**
 * Creates Interrupt Vector Table.
 */
void init_ivt(void);

/**
 * Initializes the stack pointer register for the given mode.
 * 
 * @param mode  The processor mode for which the stack pointer is to be initialized
 * @param ptr   The first address of the stack
 */
void init_stack_pointer(uint32_t mode, uint32_t ptr);

/**
 * Initializes all stacks available on the system to addresses in the RAM.
 */
void init_stacks(void);

/**
 * Initializes a memory segment for the page allocation table.
 */
void init_alloc_table(void);


#endif /* INIT_H_ */
