
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions and structs that implement special buffer types, e.g. a ring buffer
 */


#include "lib/inttypes.h"
#include "lib/math.h"


#ifndef BUFFER_H_
#define BUFFER_H_


/* BEGIN Ring buffer */

/**
 * The struct holding the ring buffer
 * 
 * @field buffer    A pointer to the ring buffer's content buffer
 * @field cap       The maximum size of the ring buffer's content buffer
 * @field len       The length of the current ring buffer's content
 * @field r         The index of the next byte to read from the ring buffer
 */
struct ring_buffer {
    int8_t* buffer;
    size_t cap;
    size_t len;
    size_t r;
};


/**
 * Initializes a given ring buffer struct.
 * 
 * @param rb        A pointer to the ring buffer struct to be initialized
 * @param buffer    A pointer to the ring buffer's content buffer
 * @param cap       The maximum size of the ring buffer's content buffer
 */
void ring_init(struct ring_buffer* rb, int8_t* buffer, size_t cap);

/**
 * Checks whether the ring buffer is empty.
 * 
 * @param rb        A pointer to the ring buffer struct to be checked
 * 
 * @return          Whether the ring is empty
 */
int8_t ring_is_empty(struct ring_buffer* rb);

/**
 * Checks whether the ring buffer is full.
 * 
 * @param rb        A pointer to the ring buffer struct to be checked
 * 
 * @return          Whether the ring is full
 */
int8_t ring_is_full(struct ring_buffer* rb);

/**
 * Peeks into the ring, i.e. reads the next @param size bytes and saves them
 * into @param target WITHOUT deleting them from the ring buffer.
 * 
 * @param rb        A pointer to the ring buffer struct to be peeked from
 * @param target    The result buffer
 * @param size      The number of bytes to be peeked from the ring buffer
 * 
 * @return          The number of bytes peeked
 */
size_t ring_peek(struct ring_buffer* rb, int8_t* target, size_t size);

/**
 * Reads from the ring, i.e. reads the next @param size bytes and saves them
 * into @param target, deleting them from the ring buffer.
 * 
 * @param rb        A pointer to the ring buffer struct to be read from
 * @param target    The result buffer
 * @param size      The number of bytes to be read from the ring buffer
 * 
 * @return          The number of bytes read
 */
size_t ring_read(struct ring_buffer* rb, int8_t* target, size_t size);

/**
 * Writes into the ring, i.e. writes @param size bytes from @param source
 * into the ring buffer.
 * 
 * @param rb        A pointer to the ring buffer struct to be written to
 * @param source    The source buffer
 * @param size      The number of bytes to be written into the ring buffer
 * 
 * @return          The number of bytes written
 */
size_t ring_write(struct ring_buffer* rb, int8_t* source, size_t size);

/**
 * Flushes the ring buffer.
 * 
 * @param rb        A pointer to the ring buffer struct to be flushed
 */
void ring_flush(struct ring_buffer* rb);

/* END Ring buffer */

#endif /* BUFFER_H_ */
