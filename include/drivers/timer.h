
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions to interact with the System Timer (ST), e.g. to let a thread sleep for a given amount of time.
 */


#include "lib/inttypes.h"


#ifndef TIMER_H_
#define TIMER_H_


/* BEGIN Functions to interact with the hardware directly */

void timer_init_periodical(uint16_t slck_period);

void timer_init_real_time(uint16_t slck_period);

uint32_t timer_read_status(void);

uint32_t timer_read_PIT_status(void);

uint32_t timer_read_RTTINC_status(void);

/* END Functions to interact with the hardware directly */


/* BEGIN Functions abstracting direct hardware access */

void timer_clksleep(uint16_t ms);

/* END Functions abstracting direct hardware access */


#endif /* TIMER_H_ */
