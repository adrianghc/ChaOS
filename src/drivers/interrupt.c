
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Interrupt Service Routines.
 */


#include "drivers/interrupt.h"
#include "drivers/aic.h"
#include "drivers/cp15.h"
#include "drivers/dbgu.h"
#include "drivers/mc.h"
#include "drivers/timer.h"
#include "lib/inttypes.h"
#include "sys/io.h"
#include "sys/swi.h"
#include "sys/sysio.h"
#include "sys/thread.h"


typedef void (*func)(struct thread_tcb*);


/**
 * Reads the content of the Link Register and returns it as a void*.
 * 
 * @return The content of the LR
 */
__attribute__((always_inline))
inline void* read_link_register(void) {

    void* lr;
    asm volatile (
        "mov %[lr], lr \n\t"
        : [lr] "=r" (lr)
    );
    return lr;

}


/* BEGIN Interrupt Service Routines */

/**
 * Reset
 */
void isr_reset(void) {
    // When an interrupt reaches this instruction without user interaction, something is seriously broken.
    printf_isr("Reset detected.\n");
    while(1);
}

/**
 * Undefined instruction
 */
__attribute__ ((interrupt ("UNDEF")))
void isr_undefined(void) {
    void* iptr = read_link_register() - 4;
    uint32_t inst = *(uint32_t*)iptr;

    printf_isr("Undefined instruction 0x%x detected at address 0x%p.\n", inst, iptr);
}

/**
 * Software Interrupt (SWI)
 */
__attribute__ ((interrupt ("SWI")))
void isr_software_interrupt(void) {

    void* iptr = read_link_register() - 4;
    uint32_t inst = *(uint32_t*)iptr & 0xFF;
    uint8_t i = 0;

    struct thread_tcb* tcb = thread_get_current();
    thread_save_context(tcb);

    while (swi_types[i++]) {
        if (inst == swi_types[i-1]) {
            ((func)swi_functions[i-1])(tcb);

            tcb = thread_get_current();
            tcb->status = THREAD_STATUS_RUNNING;
            thread_restore_context(tcb);

            return;
        }
    }

    printf_isr("Unknown software interrupt 0x%x detected at address 0x%p.\n", inst, iptr);

}

/**
 * Prefetch Abort
 */
__attribute__ ((interrupt ("ABORT")))
void isr_prefetch_abort(void) {
    void* iptr = read_link_register() - 4;

    printf_isr("Prefetch abort detected at address 0x%p.\n", iptr);
}

/**
 * Data Abort
 */
__attribute__ ((interrupt ("ABORT")))
void isr_data_abort(void) {
    void* iptr = read_link_register() - 8;
    void* addr = (void*) cp15_read_fault_address(); // TODO distinguish abort sources
    struct thread_tcb* tcb = thread_get_current();

    thread_save_context(tcb);

    printf_isr("Data abort by thread %x for attempted access of 0x%p detected at address 0x%p.\n",
            tcb->id, addr, iptr);
    thread_print_info(tcb);

    thread_exit(tcb, THREAD_DESTROY_CODE);
    thread_switch();
}

/**
 * Fast Interrupt Request (FIQ)
 */
__attribute__ ((interrupt ("FIQ")))
void isr_fast_interrupt_request(void) {
    void* iptr = read_link_register() - 8;

    printf_isr("Fast Interrupt request detected during execution at address 0x%p.\n", iptr);
}

/**
 * Interrupt Request (IRQ)
 */
__attribute__ ((interrupt ("IRQ")))
void isr_interrupt_request(void) {
    char c;
    struct thread_tcb* thread;

    // Interrupt from the Period Interval Timer
    if (timer_read_PIT_status()) {
        thread_unblock_for_timer();
        thread_switch();
        return;
    }

    // Read an input character
    if (dbgu_char_readable()) {
        c = dbgu_read_char();
        io_dbgu_write_input_char(c);

        thread = thread_unblock_for_input();
        if (thread) {
            swi_str_read_resume(thread);
        }

        for (thread = thread_unblock_for_char(); thread != 0; thread = thread_unblock_for_char()) {
            swi_getc_resume(thread, c);
        }
    }

    // Write an output character
    if (dbgu_char_writable()) {
        if (!io_dbgu_read_output_char(&c)) {
            dbgu_txrdy_interrupt_disable();
        } else {
            dbgu_write_char(c);
        }
    }
}

/* END Interrupt Service Routines */


/* BEGIN Functions for specific interrupts */

/**
 * Enables the IRQ signal.
 */
void interrupt_enable_irq(void) {

    asm volatile (
        "mrs r3, CPSR \n\t"
        "and r3, r3, #0xFFFFFF7F \n\t"
        "msr CPSR, r3 \n\t"
    );

}

/**
 * Enables the FIQ signal.
 */
void interrupt_enable_fiq(void) {

    asm volatile (
        "mrs r3, CPSR \n\t"
        "and r3, r3, #0xFFFFFFBF \n\t"
        "msr CPSR, r3 \n\t"
    );

}

/**
 * Disables the IRQ signal.
 */
void interrupt_disable_irq(void) {

    asm volatile (
        "mrs r3, CPSR \n\t"
        "orr r3, r3, #0x80 \n\t"
        "msr CPSR, r3 \n\t"
    );

}

/**
 * Disables the FIQ signal.
 */
void interrupt_disable_fiq(void) {

    asm volatile (
        "mrs r3, CPSR \n\t"
        "orr r3, r3, #0x40 \n\t"
        "msr CPSR, r3 \n\t"
    );

}

/**
 * Enables all interrupt signals.
 */
void interrupt_enable(void) {
    interrupt_enable_fiq();
    interrupt_enable_irq();
}

/**
 * Disables all interrupt signals.
 */
void interrupt_disable(void) {
    interrupt_disable_fiq();
    interrupt_disable_irq();
}

/* END Functions for specific interrupts */
