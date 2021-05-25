
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Dynamic kernel memory management
 */


#include "lib/inttypes.h"


#ifndef KMEM_H_
#define KMEM_H_


/**
 * The struct for a kernel dynamic memory management entry header.
 * 
 * @field prev  "Pointer" to the previous entry (except for the last bit)
 * @field next  "Pointer" to the next entry (except for the last bit)
 */
struct kmem_header {
    uint32_t prev;
    uint32_t next;
};


/**
 * Returns a pointer to the previous entry for a given header.
 * 
 * @param header    A pointer to the header
 * 
 * @return          The pointer, or 0 iff @param header has no predecessor
 */
struct kmem_header* kmem_prev(struct kmem_header* header);

/**
 * Returns a pointer to the next entry for a given header.
 * 
 * @param header    A pointer to the header
 * 
 * @return          The pointer, or 0 iff @param header has no successor
 */
struct kmem_header* kmem_next(struct kmem_header* header);


/**
 * Initializes a header entry.
 * 
 * @param header    A pointer to the header
 * @param prev      "Pointer" to the previous entry (except for the last bit)
 * @param next      "Pointer" to the next entry (except for the last bit)
 */
void kmem_write_header(struct kmem_header* header, uint32_t prev, uint32_t next);

/**
 * Finds the header entry for a given address.
 * 
 * @param header    A pointer to the first header
 * @param ptr       The given address
 * 
 * @return          The header entry where the given address is in, or 0 iff the address
 *                  is before the managed memory
 */
struct kmem_header* kmem_find_header(struct kmem_header* header, void* ptr);


/**
 * Initializes the dynamic memory management.
 * 
 * @param start     The start of the dynamically managed memory area
 * @param size      The size of the dynamically managed memory area
 * 
 * @return          A pointer to the first header
 */
struct kmem_header* kmem_init(void* start, uint32_t size);


/**
 * Returns whether an entry is reserved.
 * 
 * @param header    The entry for which to perform the check
 * 
 * @return          1 iff the entry is reserved, 0 otherwise
 */
int8_t kmem_is_reserved(struct kmem_header* header);


/**
 * Returns the size of the dynamically managed memory area.
 * 
 * @param header    A pointer to the first header
 * 
 * @return          The size of the dynamically managed memory area
 */
uint32_t kmem_len(struct kmem_header* header);


/**
 * Tries to split an unreserved entry in two chunks of @param size bytes and the remaining size.
 * Works only with unreserved entries.
 * 
 * @param header    A pointer to the entry to split
 * @param size      The wanted size of the first chunk
 * 
 * @return           0 = Could not split, size too small
 *                  -1 = Did not split, but it fits
 *                   1 = Sucessfully splitted
 */
int8_t kmem_split(struct kmem_header* header, uint32_t size);

/**
 * Tries to join two unreserved entries into one.
 * Works only with unreserved entries.
 * 
 * @param header    A pointer to the entry whose previous and next entries should be joined
 * 
 * @return          0 = Could not join
 *                  1 = Sucessfully joined
 */
int8_t kmem_join(struct kmem_header* header);


/**
 * Sets an entry as reserved.
 * 
 * @param header    A pointer to the entry to be reserved
 * 
 * @return          1 = the reservation was successful,
 *                  0 = @param header is the last entry
 */
int8_t kmem_reserve(struct kmem_header* header);

/**
 * Sets an entry as released.
 * 
 * @param header    A pointer to the entry to be released
 * 
 * @return          1 = the release was successful,
 *                  0 = @param header is the last entry
 */
int8_t kmem_release(struct kmem_header* header);


/**
 * Allocates @param size bytes.
 * 
 * @param header    A pointer to the first header
 * @param size      The number of bytes to be allocated
 * 
 * @return          A pointer to the allocated memory
 */
void* kmem_alloc(struct kmem_header* header, uint32_t size);

/**
 * Allocates @param size bytes.
 * 
 * @param size      The number of bytes to be allocated
 * 
 * @return          A pointer to the allocated memory
 */
void* kmalloc(uint32_t size);


/**
 * Frees the dynamically managed memory area a given pointer is in.
 * 
 * @param header    A pointer to the first header
 * @param ptr       The pointer whose memory area we want to free
 */
void kmem_free(struct kmem_header* header, void* ptr);

/**
 * Frees the dynamically managed memory area a given pointer is in.
 * 
 * @param ptr       The pointer whose memory area we want to free
 */
void kfree(void* ptr);


/**
 * Counts the total amount of free dynamically managed bytes.
 * This is only to give a general idea, without guarantees that the entire free space can be allocated.
 * 
 * @param header    A pointer to the first header
 * 
 * @return          The amount of free memory
 */
uint32_t kmem_count_free(struct kmem_header* header);

/**
 * Counts the total amount of allocated dynamically managed bytes.
 * 
 * @param header    A pointer to the first header
 * 
 * @return          The amount of allocated memory
 */
uint32_t kmem_count_alloc(struct kmem_header* header);


#endif /* KMEM_H_ */
