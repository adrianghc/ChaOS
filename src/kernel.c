
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * The kernel entry point.
 */


#include "drivers/aic.h"
#include "drivers/cp15.h"
#include "drivers/dbgu.h"
#include "drivers/init.h"
#include "drivers/timer.h"
#include "sys/io.h"
#include "sys/memmgmt.h"
#include "sys/sysio.h"
#include "sys/thread.h"


/**
 * The entry point for the first non-kernel thread to be started by ChaOS.
 */
__attribute__((section(".lib")))
void main(void);


__attribute__((naked, section(".init")))
void _start() {
    init_stacks();

    // Init the kernel memory management
    // kmem_init(KMEM_START, KMEM_SIZE);

    io_dbgu_init();

    interrupt_enable();

    dbgu_enable();
    printf_isr("DBGU has been enabled.\n");

    dbgu_rxrdy_interrupt_enable();
    printf_isr("DBGU RXRDY Interrupt has been enabled.\n");

    printf_isr("Create Interrupt Vector Table and initialize system.\n");
    init_ivt();

    printf_isr("Initializing Advanced Interrupt Controller.\n");
    aic_enable_system_peripherals();

    printf_isr("Initializing allocation table.\n");
    memmgmt_init_allocation_table();

    printf_isr("Initializing thread management.\n");
    thread_init_management();

    printf_isr("Initializing CP15 domains.\n");
    cp15_init_domains();

    printf_isr("Welcome to ChaOS.\n");

    struct thread_tcb* thread = thread_create(&main, 0, 0, 0);
    if (thread) {
        thread_activate(thread->id);
    }

    // timer_init_periodical(32768);
    timer_init_real_time(32);
    timer_init_periodical(32);
    // Nothing should be executed after this line

    while(1);
}
