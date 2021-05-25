
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions to interact with the debug unit (DBGU), e.g. to send or receive characters via
 * serial communication.
 */


#include "lib/inttypes.h"


#ifndef DBGU_H_
#define DBGU_H_


/* BEGIN Functions to interact with the hardware directly */

void dbgu_rx_enable(void);

void dbgu_rx_disable(void);

void dbgu_rx_reset(void);

void dbgu_tx_enable(void);

void dbgu_tx_disable(void);

void dbgu_tx_reset(void);

void dbgu_enable(void);

void dbgu_disable(void);

void dbgu_reset(void);

void dbgu_rxrdy_interrupt_enable(void);

void dbgu_rxrdy_interrupt_disable(void);

void dbgu_txrdy_interrupt_enable(void);

void dbgu_txrdy_interrupt_disable(void);

/* END Functions to interact with the hardware directly */


/* BEGIN Functions abstracting direct hardware access */

/**
 * Writes a string into the DBGU by writing single characters into the Transmit Holding Register
 * until all characters have been sent.
 * 
 * @param string    A pointer to a buffer containing the string to be written
 */
void dbgu_write_string(char* string);

/**
 * Reads a single character from the DBGU that is being held in the Receive Holding Register.
 * This function uses polling to determine whether there is a character to be read.
 * 
 * @return          The character that has been read
 */
char dbgu_read_char_poll();

/**
 * Reads a single character from the DBGU that is being held in the Receive Holding Register.
 * 
 * @return          The character that has been read
 */
char dbgu_read_char();

/**
 * Writes a single character into the DBGU's Transmit Holding Register.
 * 
 * @param c         The character to be written
 */
void dbgu_write_char(char c);

/**
 * Returns whether a character is available to be read.
 * 
 * @return          0 = No character has been received since the last read of
 *                      the DBGU_RHR or the receiver is disabled.
 *                  1 = At least one complete character has been received,
 *                      transferred to DBGU_RHR and not yet read.
 */
uint8_t dbgu_char_readable(void);

/**
 * Returns whether a character can be written.
 * 
 * @return          0 = A character has been written to DBGU_THR and not yet
 *                      transferred to the Shift Register, or the transmitter
 *                      is disabled.
 *                  1 = There is no character written to DBGU_THR and not yet
 *                      transferred to the Shift Register.
 */
uint8_t dbgu_char_writable(void);

/* END Functions abstracting direct hardware access */


#endif /* DBGU_H_ */
