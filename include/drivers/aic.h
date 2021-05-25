
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions to interact with the Advanced Interrupt Controller (AIC), e.g. to initialize it.
 */


#ifndef AIC_H_
#define AIC_H_


/* BEGIN Functions to interact with the hardware directly */

void aic_enable_system_peripherals(void);

void aic_clear_system_peripherals(void);

void aic_read_ivr(void);

void aic_end_of_interrupt(void);

/* END Functions to interact with the hardware directly */


#endif /* AIC_H_ */
