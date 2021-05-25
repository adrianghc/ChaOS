
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Driver for the System Timer (ST).
 * 
 * Documentation source: doc/Atmel/AT91RM9200.pdf
 * ST registers are mapped to 0xFFFF FD00 - FE00 (page 17)
 * ST User Interface on page 296
 * Register specifications on pages 297 - 303
 */


#include "drivers/timer.h"
#include "drivers/util.h"
#include "lib/inttypes.h"


/* BEGIN Register Mapping */

#define ST_BASE     0xFFFFFD00

#define ST_CR       0x0000  // Control Register                 (WRITE ONLY)
                            //
                            // This holds only one value inside the first bit, the rest is always empty.
                            //
                            // WDRST: Watchdog Timer Restart
                            // 0 = No effect.
                            // 1 = Reload the start-up value in the watchdog timer.
                            //
                            // We just need to address one bit beginning in ST_CR and don't need any defines
                            // for this register.
#define ST_PIMR     0x0004  // Period Interval Mode Register    (READ/WRITE)    (RESET VALUE 0x00000000)
                            //
                            // This holds only one value inside the first 16 bits, the rest is always empty.
                            //
                            // PIV: Period Interval Value
                            // Defines the value loaded in the 16-bit counter of the period interval timer.
                            // The maximum period is obtained by programming PIV at 0x0 corresponding to 65536
                            // slow clock cycles.
                            //
                            // We just need to address two bytes beginning in ST_PIMR and don't need any defines
                            // for this register.
#define ST_WDMR     0x0008  // Watchdog Mode Register           (READ/WRITE)    (RESET VALUE 0x00020000)
                            //
                            // This holds a value inside the first 16 bit.
                            //
                            // WDV: Watchdog Counter Value
                            // Defines the value loaded in the 16-bit counter. The maximum period is obtained by
                            // programming WDV to 0x0 corresponding to 65536 x 128 slow clock cycles.
#define ST_RTMR     0x000C  // Real-time Mode Register          (READ/WRITE)    (RESET VALUE 0x00080000)
                            //
                            // This holds only one value inside the first 16 bits, the rest is always empty.
                            //
                            // RTPRES: Real-time Timer Prescaler Value
                            // Defines the number of SLCK periods required to increment the real-time timer.
                            // The maximum period is obtained by programming RTPRES to 0x0 corresponding to 65536
                            // slow clock cycles.
                            //
                            // We just need to address two bytes beginning in ST_RTMR and don't need any defines
                            // for this register.
#define ST_SR       0x0010  // Status Register                  (READ ONLY)
#define ST_IER      0x0014  // Interrupt Enable Register        (WRITE ONLY)
#define ST_IDR      0x0018  // Interrupt Disable Register       (WRITE ONLY)
#define ST_IMR      0x001C  // Interrupt Mask Register          (READ ONLY)     (RESET VALUE 0x0)
#define ST_RTAR     0x0020  // Real-time Alarm Register         (READ/WRITE)    (RESET VALUE 0x0)
                            //
                            // This holds only one value inside the first 20 bits, the rest is always empty.
                            //
                            // ALMV: Alarm Value
                            // Defines the alarm value compared with the real-time timer. The maximum delay before
                            // ALMS status bit activation is obtained by programming ALMV to 0x0 corresponding to
                            // 1048576 seconds.
                            //
                            // We just need to address three bytes (the highest-order one only half filled) beginning
                            // in ST_RTAR and don't need any defines for this register.
#define ST_CRTR     0x0024  // Current Real-time Register       (READ ONLY)     (RESET VALUE 0x0)
                            //
                            // This holds only one value inside the first 20 bits, the rest is always empty.
                            //
                            // CRTV: Current Real-time Value
                            // Returns the current value of the real-time timer.
                            //
                            // We just need to address three bytes (the highest-order one only half filled) beginning
                            // in ST_CRTR and don't need any defines for this register.

/* END Register Mapping */


/* BEGIN Register specifications */

/* BEGIN ST_WDMR: ST Watchdog Mode Register */
// NOTE: This is commented for now, as the value n is missing from the documentation
// #define ST_RSTEN    1 << n  // Reset Enable
//                             // 0 =  No reset is generated when a watchdog overflow occurs.
//                             // 1 =  An internal reset is generated when a watchfog overflow occurs.
/* END ST_WDMR: Watchdog Mode Register */

/* 
 *BEGIN 
 * ST_SR: ST Status Register
 * AND
 * ST_IER: ST Interrupt Enable Register
 * AND
 * ST_IDR: ST Interrupt Disable Register
 * AND
 * ST_IMR: ST Interrupt Mask Register
 */
#define ST_PITS     1 << 0  // ST_SR:   Period Interval Timer Status
                            //          0 = The period interval timer has not reached 0 since the last read
                            //              of the Status Register.
                            //          1 = The period interval timer has reached 0 since the last read
                            //              of the Status Register.
                            // ST_IER:  Period Interval Timer Status Interrupt Enable
                            //          0 = No effect.
                            //          1 = Enables the corresponding interrupt.
                            // ST_IDR:  Period Interval Timer Status Interrupt Disable
                            //          0 = No effect.
                            //          1 = Disables the corresponding interrupt.
                            // ST_IMR:  Period Interval Timer Status Interrupt Mask
                            //          0 = The corresponding interrupt is disabled.
                            //          1 = The corresponding interrupt is enabled.
#define ST_WDOVF    1 << 1  // ST_SR:   Watchdog Overflow
                            //          0 = The watchdog timer has not reached 0 since the last read
                            //              of the Status Register.
                            //          1 = The watchdog timer has reached 0 since the last read
                            //              of the Status Register.
                            // ST_IER:  Watchdog Overflow Interrupt Enable
                            //          0 = No effect.
                            //          1 = Enables the corresponding interrupt.
                            // ST_IDR:  Watchdog Overflow Interrupt Disable
                            //          0 = No effect.
                            //          1 = Disables the corresponding interrupt.
                            // ST_IMR:  Watchdog Overflow Interrupt Mask
                            //          0 = The corresponding interrupt is disabled.
                            //          1 = The corresponding interrupt is enabled.
#define ST_RTTINC   1 << 2  // ST_SR:   Real-time Timer Increment
                            //          0 = The real-time timer has not been incremented since the last read
                            //              of the Status Register.
                            //          1 = The real-time timer has been incremented since the last read
                            //              of the Status Register.
                            // ST_IER:  Real-time Timer Increment Interrupt Enable
                            //          0 = No effect.
                            //          1 = Enables the corresponding interrupt.
                            // ST_IDR:  Real-time Timer Increment Interrupt Disable
                            //          0 = No effect.
                            //          1 = Disables the corresponding interrupt.
                            // ST_IMR:  Real-time Timer Increment Interrupt Mask
                            //          0 = The corresponding interrupt is disabled.
                            //          1 = The corresponding interrupt is enabled.
#define ST_ALMS     1 << 3  // ST_SR:   Alarm Status
                            //          0 = No alarm compare has been detected since the last read
                            //              of the Status Register.
                            //          1 = Alarm compare has been detected since the last read
                            //              of the Status Register.
                            // ST_IER:  Alarm Status Interrupt Enable
                            //          0 = No effect.
                            //          1 = Enables the corresponding interrupt.
                            // ST_IDR:  Alarm Status Interrupt Disable
                            //          0 = No effect.
                            //          1 = Disables the corresponding interrupt.
                            // ST_IMR:  Alarm Status Interrupt Mask
                            //          0 = The corresponding interrupt is disabled.
                            //          1 = The corresponding interrupt is enabled.
/* END ST_SR, ST_IER, ST_IDR, ST_IMR */

/* END Register specifications */




/* BEGIN Functions to interact with the hardware directly */

void timer_init_periodical(uint16_t slck_period) {
    write_u16(ST_BASE, ST_PIMR, slck_period);
    write_u32(ST_BASE, ST_IER, ST_PITS);
}

void timer_init_real_time(uint16_t slck_period) {
    write_u32(ST_BASE, ST_RTMR, slck_period);
    write_u32(ST_BASE, ST_IER, ST_RTTINC);
}

uint32_t timer_read_status(void) {
    return read_u32(ST_BASE, ST_SR);
}

uint32_t timer_read_PIT_status(void) {
    return timer_read_status() & ST_PITS;
}

uint32_t timer_read_RTTINC_status(void) {
    return timer_read_status() & ST_RTTINC;
}

/* END Functions to interact with the hardware directly */


/* BEGIN Functions abstracting direct hardware access */

void timer_clksleep(uint16_t ms) {

    uint32_t time_start;
    uint32_t time_now;
    uint32_t time_end;
    time_start = read_u32(ST_BASE, ST_CRTR);
    time_now = time_start;
    time_end = time_start + ms;
    if (time_end >= 1048576) {
        time_end = 1048575;
    }
    while (time_end > time_now) {
        time_now = read_u32(ST_BASE, ST_CRTR);
    }

}

/* END Functions abstracting direct hardware access */
