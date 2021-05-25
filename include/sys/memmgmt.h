
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for the memory management via MMU. 
 */


#include "lib/inttypes.h"


#ifndef MEMMGMT_H_
#define MEMMGMT_H_


#define MEMMGMT_TTB_ENTRIES 4096


/* BEGIN Translation and resolving functions */

/**
 * Returns the first address of a page whose index is given.
 * 
 * @param page      The index of the page
 * 
 * @return          The page's first address
 */
void* memmgmt_page_to_address(uint16_t page);

/**
 * Returns the index of the page a given address is in.
 * 
 * @param address   The address for which we want the page index
 * 
 * @return          The index of the page
 */
int32_t memmgmt_address_to_page(void* address);

/**
 * Returns the section descriptor for a given address and access permissions.
 * 
 * @param address   The given address
 * @param read      Whether read permissions are requested
 * @param write     Whether write permissions are requested
 * 
 * @return          The section descriptor for the given address
 */
uint32_t memmgmt_section_descriptor(uint32_t address, uint8_t read, uint8_t write);

/**
 * Resolves a virtual address into a physical address.
 * 
 * @param ttb       A pointer to the translation table base to use for the given virtual address
 * @param address   The virtual address to resolve
 * 
 * @return          The physical address to resolve
 */
uint32_t memmgmt_resolve(uint32_t* ttb, uint32_t address);

/* END Translation and resolving functions */


/* BEGIN Initialization functions */

/**
 * Initializes the page table for a given page.
 * 
 * @param page      The index of the page to initialize the page table for
 */
void memmgmt_init_page_table(uint16_t page);

/**
 * Initializes the allocation table.
 */
void memmgmt_init_allocation_table(void);

/* END Initialization functions */


/* BEGIN Freeing functions */

/**
 * Finds the next free page and returns its index.
 * 
 * @return          The index of the next free page
 */
int32_t memmgmt_find_free_page(void);

/**
 * Frees the page with the given index.
 * 
 * @param page      The index of the page to free
 * 
 * @return          1 iff the page could be freed, 0 otherwise
 */
uint8_t memmgmt_free_page(uint16_t page);

/**
 * Frees a block of @param pages_num contiguous pages beginning at a given address.
 * 
 * @param pages_num The number of contiguous pages to free
 * @param page_addr The first address of the first page of the block
 * 
 * @return          1 iff the pages could be freed, 0 otherwise
 */
uint8_t memmgmt_free_next_pages(uint16_t pages_num, void* page_addr);

/* END Freeing functions */


/* BEGIN Allocation functions */

/**
 * Allocates the page with the given index.
 * 
 * @param page      The index of the page to allocate
 * 
 * @return          1 iff the page could be allocated, 0 otherwise
 */
uint8_t memmgmt_allocate_page(uint16_t page);

/**
 * Find and allocate four continuous pages at a 4kb boundary (for the page table).
 * (This function is currently unused due to the 1MB pages.)
 * 
 * @return          The index of the first of the four pages
 */
int32_t memmgmt_allocate_four_pages(void);

/**
 * Allocates a block of @param pages_num contiguous pages.
 * 
 * @param pages_num The number of pages to allocate contiguously
 * 
 * @return          The first address of the first allocated page, or 0 if there was an error
 */
void* memmgmt_allocate_next_pages(uint16_t pages_num);

/* END Allocation functions */


/* BEGIN Mapping functions */

/**
 * Maps a page with a given number to a section inside a given translation table base.
 * 
 * @param ttb       A pointer to the translation table base to write the mapping into
 * @param page_num  The number of the page
 * @param target    The section's first address
 * @param read      Whether read permissions are requested
 * @param write     Whether write permissions are requested
 */
void memmgmt_map_page(uint32_t* ttb, uint32_t page_num, uint32_t target, uint8_t read, uint8_t write);

/**
 * Maps a physical address to a virtual address inside a given translation table base.
 * 
 * @param ttb       A pointer to the translation table base to write the mapping into
 * @param from      The physical address
 * @param to        The virtual address
 * @param read      Whether read permissions are requested
 * @param write     Whether write permissions are requested
 */
void memmgmt_map_to(uint32_t* ttb, uint32_t from, uint32_t to, uint8_t read, uint8_t write);

/**
 * Maps a virtual address inside a given translation table base to any (free) physical page.
 * 
 * @param ttb       A pointer to the translation table base to write the mapping into
 * @param from      The virtual address
 * @param read      Whether read permissions are requested
 * @param write     Whether write permissions are requested
 * 
 * @return          1 iff the mapping was successful, 0 otherwise
 */
uint8_t memmgmt_map_any(uint32_t* ttb, uint32_t from, uint8_t read, uint8_t write);

/**
 * Unmaps a section from a page with a given number inside a given translation table base.
 * 
 * @param ttb       A pointer to the translation table base to remove the mapping from
 * @param page_num  The number of the page
 */
void memmgmt_unmap_page(uint32_t* ttb, uint32_t page_num);

/* END Mapping functions */


/* BEGIN Thread management functions */

/**
 * Sets up a thread by allocating a page for its translation table base.
 * 
 * @param id        The thread's ID
 * 
 * @return          A pointer to the first address of the translation table base
 */
uint32_t* memmgmt_setup_thread(uint32_t);

/**
 * Cleans up a thread by freeing all its pages.
 * 
 * @param ttb_addr      A pointer to the first address of the translation table base
 */
void memmgmt_cleanup_thread(uint32_t* ttb_addr);

/* END Thread management functions */


#endif /* MEMMGMT_H_ */
