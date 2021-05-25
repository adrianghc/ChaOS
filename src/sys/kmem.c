
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Dynamic kernel memory management
 */


#include "sys/kmem.h"
#include "drivers/util.h"


#define KMEM_START  INT_RAM + KB
#define KMEM_SIZE   11 * KB


/**
 * Returns a pointer to the previous entry for a given header.
 * 
 * @param header    A pointer to the header
 * 
 * @return          The pointer, or 0 iff @param header has no predecessor
 */
struct kmem_header* kmem_prev(struct kmem_header* header) {
    return (struct kmem_header*)(header->prev & ~1);
}

/**
 * Returns a pointer to the next entry for a given header.
 * 
 * @param header    A pointer to the header
 * 
 * @return          The pointer, or 0 iff @param header has no successor
 */
struct kmem_header* kmem_next(struct kmem_header* header) {
    return (struct kmem_header*)(header->next & ~1);
}


/**
 * Initializes a header entry.
 * 
 * @param header    A pointer to the header
 * @param prev      "Pointer" to the previous entry (except for the last bit)
 * @param next      "Pointer" to the next entry (except for the last bit)
 */
void kmem_write_header(struct kmem_header* header, uint32_t prev, uint32_t next) {

    header->prev = prev;
    header->next = next;

}

/**
 * Finds the header entry for a given address.
 * 
 * @param header    A pointer to the first header
 * @param ptr       The given address
 * 
 * @return          The header entry where the given address is in, or 0 iff the address
 *                  is before the managed memory
 */
struct kmem_header* kmem_find_header(struct kmem_header* header, void* ptr) {

    if (ptr < (void*) header) {
        return 0; // The allocation is before the managed memory
    }

    while (header && (void*) header < ptr) {
        header = kmem_next(header);
    }

    if (!header) {
        return 0; // The allocation is after the managed memory
    }

    return kmem_prev(header);

}


/**
 * Initializes the dynamic memory management.
 * 
 * @param start     The start of the dynamically managed memory area
 * @param size      The size of the dynamically managed memory area
 * 
 * @return          A pointer to the first header
 */
struct kmem_header* kmem_init(void* start, uint32_t size) {

    // Make sure start and size are divisible by 2
    uint32_t start_ = (uint32_t) start;
    ++start_;
    start_ &= ~1;
    size = size & ~1;

    // Check the size, must not be too small
    if (size <= sizeof(struct kmem_header)+sizeof(struct kmem_header)) {
        return 0;
    }

    // Write the initial headers
    uint32_t end = start_ + size - sizeof(struct kmem_header);
    kmem_write_header((struct kmem_header*)start_, 0, end);
    kmem_write_header((struct kmem_header*)end, start_, 0);

    return (struct kmem_header*)start_;

}


/**
 * Returns whether an entry is reserved.
 * 
 * @param header    The entry for which to perform the check
 * 
 * @return          1 iff the entry is reserved, 0 otherwise
 */
int8_t kmem_is_reserved(struct kmem_header* header) {
    return header->next & 1;
}


/**
 * Returns the size of the dynamically managed memory area.
 * 
 * @param header    A pointer to the first header
 * 
 * @return          The size of the dynamically managed memory area
 */
uint32_t kmem_len(struct kmem_header* header) {

    if (!header->next) {
        return 0;
    }

    return (header->next & ~1) - ((uint32_t)header + sizeof(struct kmem_header));

}


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
int8_t kmem_split(struct kmem_header* header, uint32_t size) {

    if (!header->next) {
        return 0;
    }

    // Make sure the size is divisible by 2
    ++size;
    size &= ~1;

    uint32_t len = kmem_len(header);
    if (len < size) {
        return 0; // the section is too small for the request
    }
    if (len <= size + sizeof(struct kmem_header)) {
        return -1; // the section is too small to be split
    }

    // Write the new header
    uint32_t next_header = (uint32_t)header + size + sizeof(struct kmem_header);
    kmem_write_header((struct kmem_header*)(next_header), (uint32_t)header, header->next & ~1);

    // Link the header with the previous and next entry
    kmem_next(header)->prev = next_header;
    header->next = next_header;

    return 1;

}

/**
 * Tries to join two unreserved entries into one.
 * Works only with unreserved entries.
 * 
 * @param header    A pointer to the entry whose previous and next entries should be joined
 * 
 * @return          0 = Could not join
 *                  1 = Sucessfully joined
 */
int8_t kmem_join(struct kmem_header* header) {

    if (!header->prev || !header->next) {
        return 0;
    }

    struct kmem_header* prev = kmem_prev(header);
    struct kmem_header* next = kmem_next(header);

    prev->next = (uint32_t)next;
    next->prev = (uint32_t)prev;

    return 1;

}


/**
 * Sets an entry as reserved.
 * 
 * @param header    A pointer to the entry to be reserved
 * 
 * @return          1 = the reservation was successful,
 *                  0 = @param header is the last entry
 */
int8_t kmem_reserve(struct kmem_header* header) {

    if (!header->next) {
        return 0;
    }

    kmem_next(header)->prev |= 1;
    header->next |= 1;

    return 1;

}

/**
 * Sets an entry as released.
 * 
 * @param header    A pointer to the entry to be released
 * 
 * @return          1 = the release was successful,
 *                  0 = @param header is the last entry
 */
int8_t kmem_release(struct kmem_header* header) {

    if (!header->next) {
        return 0;
    }

    kmem_next(header)->prev &= ~1;
    header->next &= ~1;

    return 1;

}


/**
 * Allocates @param size bytes.
 * 
 * @param header    A pointer to the first header
 * @param size      The number of bytes to be allocated
 * 
 * @return          A pointer to the allocated memory
 */
void* kmem_alloc(struct kmem_header* header, uint32_t size) {

    while (header) {

        if (!kmem_is_reserved(header) && kmem_split(header, size) != 0) {
            kmem_reserve(header);
            return (void*)(header + 1);
        }

        header = kmem_next(header);

    }

    return 0;

}

/**
 * Allocates @param size bytes.
 * 
 * @param size      The number of bytes to be allocated
 * 
 * @return          A pointer to the allocated memory
 */
void* kmalloc(uint32_t size) {
    return kmem_alloc((struct kmem_header*) KMEM_START, size);
}


/**
 * Frees the dynamically managed memory area a given pointer is in.
 * 
 * @param header    A pointer to the first header
 * @param ptr       The pointer whose memory area we want to free
 */
void kmem_free(struct kmem_header* header, void* ptr) {

    header = kmem_find_header(header, ptr);
    if (!header) {
        return; // Outside the managed area
    }

    kmem_release(header);

    // Maybe we can join the allocation with the previous one?
    struct kmem_header* other = kmem_prev(header);
    if (other && !kmem_is_reserved(other)) {
        kmem_join(header);
        header = other;
    }

    // Maybe we can join the allocation with the next one?
    other = kmem_next(header);
    if (other && !kmem_is_reserved(other)) {
        kmem_join(header);
    }

}

/**
 * Frees the dynamically managed memory area a given pointer is in.
 * 
 * @param ptr       The pointer whose memory area we want to free
 */
void kfree(void* ptr) {
    kmem_free((struct kmem_header*) KMEM_START, ptr);
}


/**
 * Counts the total amount of free dynamically managed bytes.
 * This is only to give a general idea, without guarantees that the entire free space can be allocated.
 * 
 * @param header    A pointer to the first header
 * 
 * @return          The amount of free memory
 */
uint32_t kmem_count_free(struct kmem_header* header) {

    uint32_t size = 0;

    while (header) {

        if (!kmem_is_reserved(header)) {
            size += kmem_len(header);
        }

        header = kmem_next(header);

    }

    return size;

}

/**
 * Counts the total amount of allocated dynamically managed bytes.
 * 
 * @param header    A pointer to the first header
 * 
 * @return          The amount of allocated memory
 */
uint32_t kmem_count_alloc(struct kmem_header* header) {

    uint32_t size = 0;

    while (header) {

        if (kmem_is_reserved(header)) {
            size += kmem_len(header);
        }

        header = kmem_next(header);

    }

    return size;

}
