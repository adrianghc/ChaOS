
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Functions to interact with the memory controller (mc), e.g. to toggle a memory remap.
 */


#include "drivers/util.h"
#include "lib/inttypes.h"


#ifndef MC_H_
#define MC_H_


/* BEGIN Memory Controller Memory Map */

#define MCB         0xFFFFFF00  // Base address

#define MC_RCR      0x00        // Remap Control Register           (WRITE ONLY)
                                //
                                // This holds only one value inside the first bit, the rest is always empty.
                                //
                                // RCB: Remap Command Bit
                                //
                                // 0 = No effect
                                // 1 = This Command Bit acts on a toggle basis: writing a 1 alternatively
                                //     cancels and restores the remapping of the page zero memory devices.
                                //
                                // We just need to address one bit beginning in MC_RCR and don't need any defines
                                // for this register.
#define MC_ASR      0x04        // Abort Status Register            (READ ONLY)
#define MC_AASR     0x08        // Abort Address Status Register    (READ ONLY)
                                //
                                // This hold one single value inside all 32 bits.
                                // ABTADD: Abort Adress
                                // This field contains the address of the last aborted access.
                                //
                                // We just need to address four bytes beginning in MC_ASR and don't need any defines
                                // for this register.
#define MC_MPR      0x0C        // Master Priority Register         (READ/WRITE)    (RESET VALUE 0x3210)

/* END Memory Controller Memory Map */


/* BEGIN Register specifications */

/* BEGIN MC_ASR: MC Abort Status Register */
#define MC_UNDADD   1 << 0      // Undefined Address Abort Status Flag
                                //
                                // 0 = The last abort was not due to the access of an undefined address
                                //     in the address space.
                                // 1 = The last abort was due to the access of an undefined address
                                //     in the address space.
#define MC_MISADD   1 << 1      // Misaligned Address Abort Status Flag
                                //
                                // 0 = The last aborted access was not due to an address misalignment.
                                // 1 = The last aborted access was due to an address misalignment.
#define MC_ABTSZ    3 << 8      // Abort Size Status Mask
                                //
                                // ABTSZ    Abort Size
                                // -------------------
                                // 00       Byte
                                // 01       Half-word
                                // 10       Word
                                // 11       Reserved
#define MC_ABTTYP   3 << 10     // Abort Type Status Mask
                                //
                                // ABTTYP   Abort Type
                                // -------------------
                                // 00       Data Read
                                // 01       Data Write
                                // 10       Code Fetch
                                // 11       Reserved
#define MC_MST0     1 << 16     // ARM920T Abort Source
                                //
                                // 0 = The last aborted access was not due to the ARM920T.
                                // 1 = The last aborted access was due to the ARM920T.
#define MC_MST1     1 << 17     // PDC Abort Source
                                //
                                // 0 = The last aborted access was not due to the PDC.
                                // 1 = The last aborted access was due to the PDC.
#define MC_MST2     1 << 18     // UHP Abort Source
                                //
                                // 0 = The last aborted access was not due to the UHP.
                                // 1 = The last aborted access was due to the UHP.
#define MC_MST3     1 << 19     // EMAC Abort Source
                                //
                                // 0 = The last aborted access was not due to the EMAC.
                                // 1 = The last aborted access was due to the EMAC.
#define MC_SVMST0   1 << 24     // Saved ARM920T Abort Source
                                //
                                // 0 = No abort due to the ARM920T occurred since the last read of MC_ASR or it is
                                //     notified in the bit MST0.
                                // 1 = At least one abort due to the ARM920T occurred since the last read of MC_ASR
#define MC_SVMST1   1 << 25     // Saved PDC Abort Source
                                //
                                // 0 = No abort due to the PDC occurred since the last read of MC_ASR or it is
                                //     notified in the bit MST1.
                                // 1 = At least one abort due to the PDC occurred since the last read of MC_ASR
#define MC_SVMST2   1 << 26     // Saved UHP Abort Source
                                //
                                // 0 = No abort due to the UHP occurred since the last read of MC_ASR or it is
                                //     notified in the bit MST2.
                                // 1 = At least one abort due to the UHP occurred since the last read of MC_ASR
#define MC_SVMST3   1 << 27     // Saved EMAC Abort Source
                                //
                                // 0 = No abort due to the EMAC occurred since the last read of MC_ASR or it is
                                //     notified in the bit MST3.
                                // 1 = At least one abort due to the EMAC occurred since the last read of MC_ASR
/* END MC_ASR: MC Abort Status Register */

/* BEGIN MC_MPR: MC Master Priority Register */
#define MC_MSTP0    7 << 0      // ARM920T Priority
#define MC_MSTP1    7 << 4      // PDC Priority
#define MC_MSTP2    7 << 8      // UHP Priority
#define MC_MSTP3    7 << 12     // EMAC Priority
                                //
                                // MSTP3    EMAC Priority
                                // -------------------------
                                // 000      Lowest priority
                                // 111      Highest priority
// In the case of equal priorities, Master 0 has highest and Master 3 has lowest priority.
/* END MC_MPR: MC Master Priority Register */

/* END Register specifications */


/* BEGIN Functions to interact with the hardware directly */

__attribute__((always_inline))
inline void mc_toggle_remap(void) {
    write_u32(MCB, MC_RCR, 0x00000001);
}

__attribute__((always_inline))
inline uint32_t mc_read_abort_status(void) {
    return read_u32(MCB, MC_ASR);
}

__attribute__((always_inline))
inline void* mc_read_abort_address_status(void) {
    return (void*)read_u32(MCB, MC_AASR);
}

__attribute__((always_inline))
inline uint32_t mc_read_master_priority(void) {
    return read_u32(MCB, MC_MPR);
}

/* END Functions to interact with the hardware directly */


#endif /* MC_H_ */
