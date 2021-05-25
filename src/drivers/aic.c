
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Driver for the Advanced Interrupt Controller (AIC).
 * 
 * Documentation source: doc/Atmel/AT91RM9200.pdf
 * AIC registers are mapped to 0xFFFF F000 - F200 (page 17)
 * AIC description on pages 239 - 250
 * AIC User Interface on page 251
 * Register specifications on pages 252 - 260
 */


#include "drivers/aic.h"
#include "drivers/util.h"
#include "lib/inttypes.h"


/* BEGIN Register Mapping */

#define AIC_BASE        0xFFFFF000

#define AIC_SMR0        0x0000  // Source Mode Register 0               (READ/WRITE)    (RESET VALUE     0x0)
                                // Source 0 is reserved for the Fast Interrupt Input (FIQ)
#define AIC_SMR1        0x0004  // Source Mode Register 1               (READ/WRITE)    (RESET VALUE     0x0)
                                // Source 1 is reserved for System Peripherals (ST, RTC, PMC, DBGU ...)
#define AIC_SMR2        0x0008  // Source Mode Register 2               (READ/WRITE)    (RESET VALUE     0x0)
                                // Source 2 to Source 31 control up to thirty Embedded Peripheral Interrupts
                                // or External Interrupts
#define AIC_SMR3        0x000C  // Source Mode Register 3               (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR4        0x0010  // Source Mode Register 4               (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR5        0x0014  // Source Mode Register 5               (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR6        0x0018  // Source Mode Register 6               (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR7        0x001C  // Source Mode Register 7               (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR8        0x0020  // Source Mode Register 8               (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR9        0x0024  // Source Mode Register 9               (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR10       0x0028  // Source Mode Register 10              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR11       0x002C  // Source Mode Register 11              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR12       0x0030  // Source Mode Register 12              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR13       0x0034  // Source Mode Register 13              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR14       0x0038  // Source Mode Register 14              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR15       0x003C  // Source Mode Register 15              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR16       0x0040  // Source Mode Register 16              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR17       0x0044  // Source Mode Register 17              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR18       0x0048  // Source Mode Register 18              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR19       0x004C  // Source Mode Register 19              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR20       0x0050  // Source Mode Register 20              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR21       0x0054  // Source Mode Register 21              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR22       0x0058  // Source Mode Register 22              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR23       0x005C  // Source Mode Register 23              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR24       0x0060  // Source Mode Register 24              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR25       0x0064  // Source Mode Register 25              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR26       0x0068  // Source Mode Register 26              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR27       0x006C  // Source Mode Register 27              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR28       0x0070  // Source Mode Register 28              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR29       0x0074  // Source Mode Register 29              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR30       0x0078  // Source Mode Register 30              (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SMR31       0x007C  // Source Mode Register 31              (READ/WRITE)    (RESET VALUE     0x0)

#define AIC_SVR0        0x0080  // Source Vector Register 0             (READ/WRITE)    (RESET VALUE     0x0)
                                // Source 0 is reserved for the Fast Interrupt Input (FIQ)
                                //
                                // These hold only one value inside the first 32 bits, the rest is always empty.
                                //
                                // VECTOR: Source Vector
                                // The user may store in these registers the addresses of the corresponding
                                // handler for each interrupt source.
                                //
                                // We just need to address four bytes beginning in AIC_SVR* and don't need any
                                // defines for these registers.
#define AIC_SVR1        0x0084  // Source Vector Register 1             (READ/WRITE)    (RESET VALUE     0x0)
                                // Source 1 is reserved for System Peripherals (ST, RTC, PMC, DBGU ...)
#define AIC_SVR2        0x0088  // Source Vector Register 2             (READ/WRITE)    (RESET VALUE     0x0)
                                // Source 2 to Source 31 control up to thirty Embedded Peripheral Interrupts
                                // or External Interrupts
#define AIC_SVR3        0x008C  // Source Vector Register 3             (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR4        0x0090  // Source Vector Register 4             (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR5        0x0094  // Source Vector Register 5             (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR6        0x0098  // Source Vector Register 6             (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR7        0x009C  // Source Vector Register 7             (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR8        0x00A0  // Source Vector Register 8             (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR9        0x00A4  // Source Vector Register 9             (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR10       0x00A8  // Source Vector Register 10            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR11       0x00AC  // Source Vector Register 11            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR12       0x00B0  // Source Vector Register 12            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR13       0x00B4  // Source Vector Register 13            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR14       0x00B8  // Source Vector Register 14            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR15       0x00BC  // Source Vector Register 15            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR16       0x00C0  // Source Vector Register 16            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR17       0x00C4  // Source Vector Register 17            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR18       0x00C8  // Source Vector Register 18            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR19       0x00CC  // Source Vector Register 19            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR20       0x00D0  // Source Vector Register 20            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR21       0x00D4  // Source Vector Register 21            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR22       0x00D8  // Source Vector Register 22            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR23       0x00DC  // Source Vector Register 23            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR24       0x00E0  // Source Vector Register 24            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR25       0x00E4  // Source Vector Register 25            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR26       0x00E8  // Source Vector Register 26            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR27       0x00EC  // Source Vector Register 27            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR28       0x00F0  // Source Vector Register 28            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR29       0x00F4  // Source Vector Register 29            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR30       0x00F8  // Source Vector Register 30            (READ/WRITE)    (RESET VALUE     0x0)
#define AIC_SVR31       0x00FC  // Source Vector Register 31            (READ/WRITE)    (RESET VALUE     0x0)

#define AIC_IVR         0x0100  // Interrupt Vector Register            (READ-ONLY)     (RESET VALUE     0x0)
                                //
                                // This hold one single value inside all 32 bits.
                                //
                                // IRQV: Interrupt Vector Register
                                // The Interrupt Vector Register contains the vector programmed by the user in
                                // the Source Vector Register corresponding to the current interrupt.
                                // The Source Vector Register is indexed using the current interrupt number when
                                // the Interrupt Vector Register is read.
                                // When there is no current interrupt, the Interrupt Vector Register reads the
                                // value stored in AIC_SPU.
                                //
                                // We just need to address four bytes beginning in AIC_IVR and don't need any
                                // defines for this register.
#define AIC_FVR         0x0104  // Fast Interrupt Vector Register       (READ-ONLY)     (RESET VALUE     0x0)
                                //
                                // This hold one single value inside all 32 bits.
                                //
                                // FIQV: FIQ Vector Register
                                // The FIQ Vector Register contains the vector programmed by the user in the
                                // Source Vector Register 0. When there is no fast interrupt, the
                                // Fast Interrupt Vector Register reads the value stored in AIC_SPU.
                                //
                                // We just need to address four bytes beginning in AIC_FVR and don't need any
                                // defines for this register.
#define AIC_ISR         0x0108  // Interrupt Status Register            (READ-ONLY)     (RESET VALUE     0x0)
                                //
                                // This holds only one value inside the first five bits, the rest is always empty.
                                //
                                // IRQID: Current Interrupt Identifier
                                // The Interrupt Status Register returns the current interrupt source number.
                                //
                                // We just need to address one byte (filled with only five bits) beginning in
                                // AIC_ISR and don't need any defines for this register.
#define AIC_IPR         0x010C  // Interrupt Pending Register           (READ-ONLY)     (RESET VALUE     0x0)
                                // Reset value of the IPR depends on the level of the external interrupt
                                // source. All other sources are cleared at reset, thus not pending.
#define AIC_IMR         0x0110  // Interrupt Mask Register              (READ-ONLY)     (RESET VALUE     0x0)
#define AIC_CISR        0x0114  // Core Interrupt Status Register       (READ-ONLY)     (RESET VALUE     0x0)

#define AIC_IECR        0x0120  // Interrupt Enable Command Register    (WRITE-ONLY)
#define AIC_IDCR        0x0124  // Interrupt Disable Command Register   (WRITE-ONLY)
#define AIC_ICCR        0x0128  // Interrupt Clear Command Register     (WRITE-ONLY)
#define AIC_ISCR        0x012C  // Interrupt Set Command Register       (WRITE-ONLY)
#define AIC_EOICR       0x0130  // End of Interrupt Command Register    (WRITE-ONLY)
                                //
                                // The End of Interrupt Command Register is used by the interrupt routine to
                                // indicate that the interrupt treatment is complete. Any value can be written
                                // because it is only necessary to make a write to this register location to
                                // signal the end of the interrupt treatment.
#define AIC_SPU         0x0134  // Spurious Interrupt Vector Register   (READ/WRITE)    (RESET VALUE     0x0)
                                //
                                // This hold one single value inside all 32 bits.
                                //
                                // SIQV: Spurious Interrupt Vector Register
                                // The user may store the address of a spurious interrupt handler in this register.
                                // The written value is returned in AIC_IVR in case of a spurious interrupt and
                                // in AIC_FVR in case of a spurious fast interrupt.
                                //
                                // We just need to address four bytes beginning in AIC_SPU and don't need any
                                // defines for this register.
#define AIC_DCR         0x0138  // Debug Control Register               (READ/WRITE)    (RESET VALUE     0x0)


/* END Register Mapping */


/* BEGIN Register specifications */

/* BEGIN AIC_SMR0..AIC_SMR31: AIC Source Mode Register*/
#define AIC_PRIOR_0     0 << 0  // Priority Level
                                //
                                // Programs the priority level for all sources except FIQ source (source 0).
                                // The priority level can be between 0 (lowest) and 7 (highest).
                                // The priority level is not used for the FIQ in the related SMR register AIC_SMRx.
#define AIC_PRIOR_1     1 << 0  // ...
#define AIC_PRIOR_2     2 << 0  // ...
#define AIC_PRIOR_3     3 << 0  // ...
#define AIC_PRIOR_4     4 << 0  // ...
#define AIC_PRIOR_5     5 << 0  // ...
#define AIC_PRIOR_6     6 << 0  // ...
#define AIC_PRIOR_7     7 << 0  // ...
#define AIC_SRCTYPE_00  0 << 5  // Interrupt Source Type
                                //
                                // The active level or edge is not programmable for the internal interrupt sources.
                                //
                                //  SRCTYPE   |    Internal Interrupt Sources   |   External Interrupt Sources
                                // -----------|---------------------------------|-----------------------------
                                //    00      |    High-level Sensitive         |   Low-level Sensitive
                                //    01      |    Positive-edge Triggered      |   Negative-edge Triggered
                                //    10      |    High-level Sensitive         |   High-level Sensitive
                                //    11      |    Positive-edge Triggered      |   Positive-edge Triggered
#define AIC_SRCTYPE_01  1 << 5  // ...
#define AIC_SRCTYPE_10  2 << 5  // ...
#define AIC_SRCTYPE_11  3 << 5  // ...
/* END AIC_SMR0..AIC_SMR31 */

/* BEGIN
 * AIC_IPR: AIC Interrupt Pending Register
 * AND
 * AIC_IMR: AIC Interrupt Mask Register
 * AND
 * AIC_IECR: AIC Interrupt Enable Command Register
 * AND
 * AIC_IDCR: AIC Interrupt Disable Command Register
 * AND
 * AIC_ICCR: AIC Interrupt Clear Command Register
 * AND
 * AIC_ISCR: AIC Interrupt Set Command Register
 */
#define AIC_FIQ         1 << 0  // AIC_IPR:     Interrupt Pending
                                //              0 = Corresponding interrupt is not pending
                                //              1 = Corresponding interrupt is pending
                                // AIC_IMR:     Interrupt Mask
                                //              0 = Corresponding interrupt is disabled.
                                //              1 = Corresponding interrupt is enabled.
                                // AIC_IECR:    Enables corresponding interrupt.
                                // AIC_IDCR:    Disables corresponding interrupt.
                                // AIC_ICCR:    Clears corresponding interrupt.
                                // AIC_ISCR:    Sets corresponding interrupt.
#define AIC_SYS         1 << 1  // ...
#define AIC_PID2        1 << 2  // ...
#define AIC_PID3        1 << 3  // ...
#define AIC_PID4        1 << 4  // ...
#define AIC_PID5        1 << 5  // ...
#define AIC_PID6        1 << 6  // ...
#define AIC_PID7        1 << 7  // ...
#define AIC_PID8        1 << 8  // ...
#define AIC_PID9        1 << 9  // ...
#define AIC_PID10       1 << 10 // ...
#define AIC_PID11       1 << 11 // ...
#define AIC_PID12       1 << 12 // ...
#define AIC_PID13       1 << 13 // ...
#define AIC_PID14       1 << 14 // ...
#define AIC_PID15       1 << 15 // ...
#define AIC_PID16       1 << 16 // ...
#define AIC_PID17       1 << 17 // ...
#define AIC_PID18       1 << 18 // ...
#define AIC_PID19       1 << 19 // ...
#define AIC_PID20       1 << 20 // ...
#define AIC_PID21       1 << 21 // ...
#define AIC_PID22       1 << 22 // ...
#define AIC_PID23       1 << 23 // ...
#define AIC_PID24       1 << 24 // ...
#define AIC_PID25       1 << 25 // ...
#define AIC_PID26       1 << 26 // ...
#define AIC_PID27       1 << 27 // ...
#define AIC_PID28       1 << 28 // ...
#define AIC_PID29       1 << 29 // ...
#define AIC_PID30       1 << 30 // ...
#define AIC_PID31       1 << 31 // ...
/* END AIC_IPR, AIC_IMR, AIC_IECR, AIC_IDCR, AIC_ICCR, AIC_ISCR */

/* BEGIN AIC_CISR: AIC Core Interrupt Status Register */
#define AIC_NFIQ        1 << 0  // NFIQ Status
                                // 0 = nFIQ line is deactivated.
                                // 1 = nFIQ line is active.
#define AIC_NIRQ        1 << 1  // NIRQ Status
                                // 0 = nIRQ line is deactivated.
                                // 1 = nIRQ line is active.
/* END AIC_CISR */

/* BEGIN AIC_DEBUG: AIC Debug Control Register */
#define AIC_PROT        1 << 0  // PROT: Protection Mode
                                // 0 = The Protection Mode is disabled.
                                // 1 = The Protection Mode is enabled.
#define AIC_GMSK        1 << 1  // GMSK: General Mask
                                // 0 = The nIRQ and nFIQ lines are normally controlled by the AIC.
                                // 1 = The nIRQ and nFIQ lines are tied to their inactive state.
/* END AIC_DEBUG */

/* END Register specifications */




/* BEGIN Functions to interact with the hardware directly */

void aic_enable_system_peripherals(void) {
    write_u32(AIC_BASE, AIC_SMR1, AIC_SRCTYPE_00 | AIC_PRIOR_7);
    write_u32(AIC_BASE, AIC_IECR, AIC_SYS);
}

void aic_clear_system_peripherals(void) {
    write_u32(AIC_BASE, AIC_ICCR, AIC_SYS);
}

void aic_read_ivr(void) {
    read_u32(AIC_BASE, AIC_IVR);
}

void aic_end_of_interrupt(void) {
    write_u32(AIC_BASE, AIC_EOICR, 0x01);
}

/* END Functions to interact with the hardware directly */
