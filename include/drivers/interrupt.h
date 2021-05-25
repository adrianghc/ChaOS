
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Interrupt Service Routines.
 */


#ifndef INTERRUPT_H_
#define INTERRUPT_H_


/* BEGIN Interrupt Service Routines */

/**
 * Reset
 */
void isr_reset(void);

/**
 * Undefined instruction
 */
__attribute__ ((interrupt ("UNDEF")))
void isr_undefined(void);

/**
 * Software Interrupt (SWI)
 */
__attribute__ ((interrupt ("SWI")))
void isr_software_interrupt(void);

/**
 * Prefetch Abort
 */
__attribute__ ((interrupt ("ABORT")))
void isr_prefetch_abort(void);

/**
 * Data Abort
 */
__attribute__ ((interrupt ("ABORT")))
void isr_data_abort(void);

/**
 * Fast Interrupt Request (FIQ)
 */
__attribute__ ((interrupt ("FIQ")))
void isr_fast_interrupt_request(void);

/**
 * Interrupt Request (IRQ)
 */
__attribute__ ((interrupt ("IRQ")))
void isr_interrupt_request(void);

/* END Interrupt Service Routines */


/* BEGIN Functions for specific interrupts */

/**
 * Enables the FIQ signal.
 */
void interrupt_enable_fiq(void);

/**
 * Enables the IRQ signal.
 */
void interrupt_enable_irq(void);

/**
 * Disables the FIQ signal.
 */
void interrupt_disable_fiq(void);

/**
 * Disables the IRQ signal.
 */
void interrupt_disable_irq(void);

/**
 * Enables all interrupt signals.
 */
void interrupt_enable(void);

/**
 * Disables all interrupt signals.
 */
void interrupt_disable(void);

/* END Functions for specific interrupts */


#endif /* INTERRUPT_H_ */
