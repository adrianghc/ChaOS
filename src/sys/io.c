
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Input and Output service functions.
 */


#include "sys/io.h"
#include "drivers/dbgu.h"
#include "drivers/interrupt.h"
#include "lib/inttypes.h"
#include "lib/buffer.h"


#define IO_DBGU_INPUT_BUFFER    512
#define IO_DBGU_OUTPUT_BUFFER   4096


struct ring_buffer io_dbgu_input_buffer;
struct ring_buffer io_dbgu_output_buffer;
char io_dbgu_input_rawbuffer[IO_DBGU_INPUT_BUFFER];
char io_dbgu_output_rawbuffer[IO_DBGU_OUTPUT_BUFFER];


/**
 * Initializes buffers for IO via DBGU.
 */
void io_dbgu_init(void) {

    ring_init(&io_dbgu_input_buffer, io_dbgu_input_rawbuffer, IO_DBGU_INPUT_BUFFER);
    ring_init(&io_dbgu_output_buffer, io_dbgu_output_rawbuffer, IO_DBGU_OUTPUT_BUFFER);

}

/**
 * Reads at most @param maxlen bytes from the DBGU input buffer into the given string buffer.
 * 
 * @param str       A pointer to the buffer to store the result in
 * @param maxlen    The number of bytes to be read from the DBGU input buffer
 * 
 * @return          The number of bytes read from the DBGU input buffer
 */
size_t io_dbgu_read_input_string(char* str, size_t maxlen) {
    return ring_read(&io_dbgu_input_buffer, str, maxlen);
}

/**
 * Flushes the input buffer.
 */
void io_dbgu_read_flush(void) {
    ring_flush(&io_dbgu_input_buffer);
}

/**
 * Writes @param len bytes from the given string buffer into the DBGU output buffer.
 * 
 * @param str       A pointer to the string to be written into the DBGU output buffer
 * @param len       The number of bytes to be written into the DBGU output buffer
 * 
 * @return          The number of bytes written into the DBGU input buffer
 */
size_t io_dbgu_write_output_string(char* str, size_t len) {

    size_t result;

    result = ring_write(&io_dbgu_output_buffer, str, len);

    // Make sure the output interrupt is enabled
    dbgu_txrdy_interrupt_enable();

    return result;

}

/**
 * Reads a character from the DBGU input buffer.
 * Use only in Interrupt Service Routines!
 * 
 * @param c         A pointer to the char to store the result in
 * 
 * @return          Whether the read was successful
 */
uint8_t io_dbgu_read_output_char(char* c) {
    return (uint8_t)ring_read(&io_dbgu_output_buffer, c, 1);
}

/**
 * Writes a character into the DBGU input buffer.
 * Use only in Interrupt Service Routines!
 * 
 * @param c         The char to be written into the DBGU input buffer
 * 
 * @return          Whether the write was successful
 */
uint8_t io_dbgu_write_input_char(char c) {
    return (uint8_t)ring_write(&io_dbgu_input_buffer, &c, 1);
}
