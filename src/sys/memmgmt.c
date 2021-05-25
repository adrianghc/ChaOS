
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions for the memory management via MMU. 
 */


#include "sys/memmgmt.h"
#include "drivers/util.h"
#include "lib/inttypes.h"
#include "lib/math.h"
#include "lib/mem.h"


/* BEGIN Translation and resolving functions */

/**
 * Returns the first address of a page whose index is given.
 * 
 * @param page      The index of the page
 * 
 * @return          The page's first address
 */
void* memmgmt_page_to_address(uint16_t page) {
    return (void*)((uint32_t)page * PAGE_SIZE + EXT_RAM);
}

/**
 * Returns the index of the page a given address is in.
 * 
 * @param address   The address for which we want the page index
 * 
 * @return          The index of the page
 */
int32_t memmgmt_address_to_page(void* address) {

    if ((uint32_t) address < EXT_RAM || (uint32_t) address >= EXT_RAM + EXT_RAM_LEN) {
        return -1;
    }

    return math_div((uint32_t)address - EXT_RAM, PAGE_SIZE);

}

/**
 * Returns the section descriptor for a given address and access permissions.
 * 
 * @param address   The given address
 * @param read      Whether read permissions are requested
 * @param write     Whether write permissions are requested
 * 
 * @return          The section descriptor for the given address
 */
uint32_t memmgmt_section_descriptor(uint32_t address, uint8_t read, uint8_t write) {

    uint32_t options = 0x00000012;
    uint32_t perm = 1; // user permissions, supervisor can r/w
    if (read && write) {
        perm += 2;
    } else if (read) {
        perm += 1;
    }

    return (address & 0xFFF00000) | options | perm << 10;

}

/**
 * Resolves a virtual address into a physical address.
 * 
 * @param ttb       A pointer to the translation table base to use for the given virtual address
 * @param address   The virtual address to resolve
 * 
 * @return          The physical address to resolve
 */
uint32_t memmgmt_resolve(uint32_t* ttb, uint32_t address) {

    uint32_t offset = address & 0x000FFFFF;
    uint32_t index = address >> 20;

    uint32_t resolved = ttb[index];

    // check if the page is mapped to a section
    if ((resolved & 0x03) != 0x02) {
        return 0;
    }

    resolved &= 0xFFF00000;
    return resolved | offset;

}

/* END Translation and resolving functions */


/* BEGIN Initialization functions */

/**
 * Initializes the page table for a given page.
 * 
 * @param page      The index of the page to initialize the page table for
 */
void memmgmt_init_page_table(uint16_t page) {
    memzero(memmgmt_page_to_address(page), MEMMGMT_TTB_ENTRIES * 4);
}

/**
 * Initializes the allocation table.
 */
void memmgmt_init_allocation_table(void) {

    uint32_t* alloc_table = (uint32_t*) ALLOC_TABLE;

    memzero((uint8_t*) alloc_table, ALLOC_TABLE_ENTRIES * 4);
    alloc_table[0] = 0x00000003;  // reserve the first two pages in RAM (the kernel memory!)

}

/* END Initialization functions */


/* BEGIN Freeing functions */

/**
 * Finds the next free page and returns its index.
 * 
 * @return          The index of the next free page
 */
int32_t memmgmt_find_free_page(void) {

    uint16_t i, j;
    uint32_t entry;
    uint32_t* alloc_table = (uint32_t*) ALLOC_TABLE;

    for (i = 0; i < ALLOC_TABLE_ENTRIES; i++) {
        entry = alloc_table[i];
        if (entry == 0xFFFFFFFF) {
            continue;
        }

        // we found a free spot!
        for (j = 0; j < 32; j++) {

            if ((entry & 1) == 0) {
                return i*32 + j;
            }

            entry >>= 1;  // shift by one for the next iteration
        }
    }

    return -1;

}

/**
 * Frees the page with the given index.
 * 
 * @param page      The index of the page to free
 * 
 * @return          1 iff the page could be freed, 0 otherwise
 */
uint8_t memmgmt_free_page(uint16_t page) {

    if (page <= 1 || page >= ALLOC_TABLE_ENTRIES) {
        // prevent freeing of the first two entries or entries behind the table
        return 0;
    }

    uint32_t* alloc_table = (uint32_t*) ALLOC_TABLE;
    uint16_t idx = page >> 5;
    uint8_t bit = page & 0x1F;
    uint32_t entry = alloc_table[idx];
    uint32_t b = 1 << bit;

    if ((entry & b) == 1) {
        alloc_table[idx] &= ~b;
        return 1;
    }
    return 0; // ERROR: page not allocated

}

/**
 * Frees a block of @param pages_num contiguous pages beginning at a given address.
 * 
 * @param pages_num The number of contiguous pages to free
 * @param page_addr The first address of the first page of the block
 * 
 * @return          1 iff the pages could be freed, 0 otherwise
 */
uint8_t memmgmt_free_next_pages(uint16_t pages_num, void* page_addr) {

    uint32_t i;
    int32_t page = memmgmt_address_to_page(page_addr);
    if (page == -1) {
        return 0;
    }

    uint32_t result_errcode = 1;

    for (i = 0; i < pages_num; i++) {
        if (memmgmt_free_page(page)) {
            page++;
        } else {
            // ERROR
            result_errcode = 0;
            break;
        }
    }

    return result_errcode;

}

/* END Freeing functions */


/* BEGIN Allocation functions */

/**
 * Allocates the page with the given index.
 * 
 * @param page      The index of the page to allocate
 * 
 * @return          1 iff the page could be allocated, 0 otherwise
 */
uint8_t memmgmt_allocate_page(uint16_t page) {

    uint32_t* alloc_table = (uint32_t*) ALLOC_TABLE;
    uint16_t idx = page >> 5;
    uint8_t bit = page & 0x1F;
    uint32_t entry = alloc_table[idx];
    uint32_t b = 1 << bit;

    if ((entry & b) == 0) {
        alloc_table[idx] |= b;
        return 1;
    }
    return 0; // ERROR: page not free

}

/**
 * Find and allocate four continuous pages at a 4kb boundary (for the page table).
 * (This function is currently unused due to the 1MB pages.)
 * 
 * @return          The index of the first of the four pages
 */
int32_t memmgmt_allocate_four_pages(void) {

    uint32_t matcher;
    uint32_t i, j;
    uint32_t* alloc_table = (uint32_t*) ALLOC_TABLE;

    for (i = 0; i < ALLOC_TABLE_ENTRIES; i++) {

        if (alloc_table[i] == 0xFFFFFFFF) {
            // this section is full, try the next one
            continue;
        }

        j = 0;
        for (matcher = 0x0000000F; matcher != 0x00000000; matcher <<= 4) {
            j += 4;

            if ((alloc_table[i] & matcher) == 0) {
                // we've found four free pages, allocate them and return the number of the first one

                alloc_table[i] |= matcher;
                return i*32 + j;

            }

        }

    }

    return -1;

}

/**
 * Allocates a block of @param pages_num contiguous pages.
 * 
 * @param pages_num The number of pages to allocate contiguously
 * 
 * @return          The first address of the first allocated page, or 0 if there was an error
 */
void* memmgmt_allocate_next_pages(uint16_t pages_num) {

    int i;
    uint32_t page = 0;
    uint32_t* first_page_address = 0;

    for (i = 0; i < pages_num; i++) {
        page = memmgmt_find_free_page();
        if (!memmgmt_allocate_page(page)) {
            // ERROR
            first_page_address = 0;
            break;
        }
        if (i == 0) {
            first_page_address = (void*) (EXT_RAM + page * PAGE_SIZE);
        }

    }

    return first_page_address;

}

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
void memmgmt_map_page(uint32_t* ttb, uint32_t page_num, uint32_t target, uint8_t read, uint8_t write) {

    if (page_num >= MEMMGMT_TTB_ENTRIES) {
        return;
    }

    ttb[page_num] = memmgmt_section_descriptor(target, read, write);

}

/**
 * Maps a physical address to a virtual address inside a given translation table base.
 * 
 * @param ttb       A pointer to the translation table base to write the mapping into
 * @param from      The physical address
 * @param to        The virtual address
 * @param read      Whether read permissions are requested
 * @param write     Whether write permissions are requested
 */
void memmgmt_map_to(uint32_t* ttb, uint32_t from, uint32_t to, uint8_t read, uint8_t write) {

    int32_t page;

    from &= 0xFFF00000;
    to   &= 0xFFF00000;

    memmgmt_map_page(ttb, math_div(from, PAGE_SIZE), to, read, write);

    page = memmgmt_address_to_page((void*) to);
    if (page != -1) {
        memmgmt_allocate_page(page);
    }

}

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
uint8_t memmgmt_map_any(uint32_t* ttb, uint32_t from, uint8_t read, uint8_t write) {

    uint32_t table_entry = math_div(from & 0xFFF00000, PAGE_SIZE);
    if (ttb[table_entry]) {
        return 0;  // this entry is already occupied
    }

    int32_t page = memmgmt_find_free_page();
    if (page == -1) {
        return 0;
    }
    memmgmt_allocate_page((uint16_t)page);

    memmgmt_map_page(ttb, math_div(from, PAGE_SIZE), (uint32_t) memmgmt_page_to_address(page), read, write);

    return 1;

}

/**
 * Unmaps a section from a page with a given number inside a given translation table base.
 * 
 * @param ttb       A pointer to the translation table base to remove the mapping from
 * @param page_num  The number of the page
 */
void memmgmt_unmap_page(uint32_t* ttb, uint32_t page_num) {

    if (page_num >= MEMMGMT_TTB_ENTRIES) {
        return;
    }

    ttb[page_num] = 0x00000000;

}

/* END Mapping functions */


/* BEGIN Thread management functions */

/**
 * Sets up a thread by allocating a page for its translation table base.
 * 
 * @param id        The thread's ID
 * 
 * @return          A pointer to the first address of the translation table base
 */
uint32_t* memmgmt_setup_thread(uint32_t id) {

    uint32_t* ttb_addr = (uint32_t*)(TTB_FIRST_ADDR + (id-1) * 16*KB);
    memzero((uint8_t*) ttb_addr, MEMMGMT_TTB_ENTRIES * 4);

    return ttb_addr;

}

/**
 * Cleans up a thread by freeing all its pages.
 * 
 * @param ttb_addr      A pointer to the first address of the translation table base
 */
void memmgmt_cleanup_thread(uint32_t* ttb_addr) {

    uint16_t i;
    int32_t page;

    for (i = 0; i < MEMMGMT_TTB_ENTRIES; i++) {

        if (ttb_addr[i] == 0) {
            // Empty entry, nothing to do
            continue;
        }

        // free the page the table points to
        page = memmgmt_address_to_page((void*) (ttb_addr[i] & 0xFFF00000));

        if (page > 1) {               // if the page is not unallocatable and or the OS ...
            memmgmt_free_page(page);  // ... free it
        }
        // ttb_addr[i] = 0;

    }

    // free the page the table is on
    page = memmgmt_address_to_page((void*)((uint32_t)ttb_addr & 0xFFF00000));

    if (page > 1) {
        memmgmt_free_page(page);
    }

}

/* END Thread management functions */
