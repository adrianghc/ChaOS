
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Input and Output service functions.
 */


#include "lib/inttypes.h"


#ifndef IO_H_
#define IO_H_


/**
 * Initializes buffers for IO via DBGU.
 */
void io_dbgu_init(void);

/**
 * Reads at most @param maxlen bytes from the DBGU input buffer into the given string buffer.
 * 
 * @param str       A pointer to the buffer to store the result in
 * @param maxlen    The number of bytes to be read from the DBGU input buffer
 * 
 * @return          The number of bytes read from the DBGU input buffer
 */
size_t io_dbgu_read_input_string(char* str, size_t maxlen);

/**
 * Flushes the input buffer.
 */
void io_dbgu_read_flush(void);

/**
 * Writes @param len bytes from the given string buffer into the DBGU output buffer.
 * 
 * @param str       A pointer to the string to be written into the DBGU output buffer
 * @param len       The number of bytes to be written into the DBGU output buffer
 * 
 * @return          The number of bytes read from the DBGU input buffer
 */
size_t io_dbgu_write_output_string(char* str, size_t len);

/**
 * Writes @param len bytes from the given string buffer into the DBGU output buffer.
 * Use only in Interupt Service Routines!
 * 
 * @param str       A pointer to the string to be written into the DBGU output buffer
 * @param len       The number of bytes to be written into the DBGU output buffer
 * 
 * @return          The number of bytes read from the DBGU input buffer
 */
size_t io_dbgu_write_output_string_isr(char* str, size_t len);

/**
 * Reads a character from the DBGU input buffer.
 * Use only in Interupt Service Routines!
 * 
 * @param c         A pointer to the char to store the result in
 * 
 * @return          Whether the read was successful
 */
uint8_t io_dbgu_read_output_char(char* c);

/**
 * Writes a character into the DBGU input buffer.
 * Use only in Interupt Service Routines!
 * 
 * @param c         The char to be written into the DBGU input buffer
 * 
 * @return          Whether the write was successful
 */
uint8_t io_dbgu_write_input_char(char c);


#endif /* IO_H_ */
