
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions and structs that implement special buffer types, e.g. a ring buffer
 */


#include "lib/buffer.h"
#include "lib/inttypes.h"
#include "lib/math.h"


/* BEGIN Ring buffer */

/**
 * Initializes a given ring buffer struct.
 * 
 * @param rb        A pointer to the ring buffer struct to be initialized
 * @param buffer    A pointer to the ring buffer's content buffer
 * @param cap       The maximum size of the ring buffer's content buffer
 */
__attribute__((section(".lib")))
void ring_init(struct ring_buffer* rb, int8_t* buffer, size_t cap) {
    rb->buffer = buffer;
    rb->cap = cap;
    rb->len = 0;
    rb->r = 0;
}

/**
 * Checks whether the ring buffer is empty.
 * 
 * @param rb        A pointer to the ring buffer struct to be checked
 * 
 * @return          Whether the ring is empty
 */
__attribute__((section(".lib")))
int8_t ring_is_empty(struct ring_buffer* rb) {
    return rb->len == 0;
}

/**
 * Checks whether the ring buffer is full.
 * 
 * @param rb        A pointer to the ring buffer struct to be checked
 * 
 * @return          Whether the ring is full
 */
__attribute__((section(".lib")))
int8_t ring_is_full(struct ring_buffer* rb) {
    return rb->len == rb->cap;
}

/**
 * Peeks into the ring, i.e. reads the next `size` bytes and saves them
 * into `target` WITHOUT deleting them from the ring buffer.
 * 
 * @param rb        A pointer to the ring buffer struct to be peeked from
 * @param target    The result buffer
 * @param size      The number of bytes to be peeked from the ring buffer
 * 
 * @return          The number of bytes peeked
 */
__attribute__((section(".lib")))
size_t ring_peek(struct ring_buffer* rb, int8_t* target, size_t size) {
    size_t i;
    if (rb->len < size) {
        size = rb->len;
    }

    for (i = 0; i < size; i++) {
        target[i] = rb->buffer[math_mod(rb->r + i, rb->cap)];
    }

    return size;
}

/**
 * Reads from the ring, i.e. reads the next `size` bytes and saves them
 * into `target`, deleting them from the ring buffer.
 * 
 * @param rb        A pointer to the ring buffer struct to be read from
 * @param target    The result buffer
 * @param size      The number of bytes to be read from the ring buffer
 * 
 * @return          The number of bytes read
 */
__attribute__((section(".lib")))
size_t ring_read(struct ring_buffer* rb, int8_t* target, size_t size) {
    size = ring_peek(rb, target, size);

    rb->len -= size;
    rb->r = math_mod(rb->r + size, rb->cap);

    return size;
}

/**
 * Writes into the ring, i.e. writes `size` bytes from `source` into the
 * ring buffer.
 * 
 * @param rb        A pointer to the ring buffer struct to be written to
 * @param source    The source buffer
 * @param size      The number of bytes to be written into the ring buffer
 * 
 * @return          The number of bytes written
 */
__attribute__((section(".lib")))
size_t ring_write(struct ring_buffer* rb, int8_t* source, size_t size) {
    size_t i;
    size_t space = rb->cap - rb->len;
    if (size > space) {
        size = space;
    }

    for (i = 0; i < size; i++) {
        rb->buffer[math_mod(rb->r + rb->len + i, rb->cap)] = source[i];
    }

    rb->len += size;
    return size;
}

/**
 * Flushes the ring buffer.
 * 
 * @param rb        A pointer to the ring buffer struct to be flushed
 */
__attribute__((section(".lib")))
void ring_flush(struct ring_buffer* rb) {
    rb->len = 0;
}


/* END Ring buffer */
