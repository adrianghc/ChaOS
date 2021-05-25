
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions to interact with the memory controller (mc), e.g. to toggle a memory remap.
 */


#include "lib/inttypes.h"


#ifndef MC_H_
#define MC_H_


/* BEGIN Functions to interact with the hardware directly */

void mc_toggle_remap(void);

uint32_t mc_read_abort_status(void);

void* mc_read_abort_address_status(void);

uint32_t mc_read_master_priority(void);

/* END Functions to interact with the hardware directly */


#endif /* MC_H_ */
