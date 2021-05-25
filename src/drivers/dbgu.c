
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Driver for serial communication over debug unit (DBGU)
 * 
 * Documentation source: doc/Atmel/AT91RM9200.pdf
 * DBGU registers are mapped to 0xFFFF F200 - F400 (page 17)
 * Debug Unit Memory Map on page 330
 * Register specifications on pages 331 - 343
 */


#include "drivers/dbgu.h"
#include "drivers/util.h"


/* BEGIN Debug Unit Memory Map */

#define DBGUB       0xFFFFF200  // Base address

#define DBGU_CR     0x0000      // Control Register                 (WRITE ONLY)
#define DBGU_MR     0x0004      // Mode Register                    (READ/WRITE)    (RESET VALUE 0x00)
#define DBGU_IER    0x0008      // Interrupt Enable Register        (WRITE ONLY)
#define DBGU_IDR    0x000C      // Interrupt Disable Register       (WRITE ONLY)
#define DBGU_IMR    0x0010      // Interrupt Mask Register          (READ ONLY)     (RESET VALUE 0x00)
#define DBGU_SR     0x0014      // Status Register                  (READ ONLY)
#define DBGU_RHR    0x0018      // Receive Holding Register         (READ ONLY)     (RESET VALUE 0x00)
                                //
                                // This holds only one value inside the first eight bits, the rest is always empty.
                                //
                                // RXCHR: Received Character
                                // Last received character if RXRDY is set.
                                //
                                // We just need to address one byte beginning in DBGU_RHR and don't need any defines
                                // for this register.
#define DBGU_THR    0x001C      // Transmit Holding Register        (WRITE ONLY)
                                //
                                // This holds only one value inside the first eight bits, the rest is always empty.
                                //
                                // TXCHR: Character to be Transmitted
                                // Next character to be transmitted after the current character if TXRDY is not set.
                                //
                                // We just need to address one byte beginning in DBGU_THR and don't need any defines
                                // for this register.
#define DBGU_BRGR   0x0020      // Baud Rate Generator Register     (READ/WRITE)    (RESET VALUE 0x00)
                                //
                                // This holds only one value inside the first 16 bits, the rest is always empty.
                                //
                                // CD: Clock Divisor
                                //
                                // CD           Baud Rate Clock
                                // ----------------------------
                                // 0            Disabled
                                // 1            MCK
                                // 2 to 65535   MCK / (CD * 16)
                                //
                                // We just need to address two bytes beginning in DBGU_BRGR and don't need any defines
                                // for this register.

// Reserved memory space from 0x0024 - 0x003C

#define DBGU_CIDR   0x0040      // Chip ID Register                 (READ ONLY)
#define DBGU_EXID   0x0044      // Chip ID Extension Register       (READ ONLY)
                                // Adding further specifications for these two registers would add a tremendous
                                // overhead to this file for something that is most definitely not going to be
                                // needed.
                                // Therefore, we have left it out. If needed, it can still be added in.

// Reserved memory space from 0x0048 - 0x00FC
// PDC Area from 0x0100 - 0x0124

/* END Debug Unit Memory Map */


/* BEGIN Register specifications */

/* BEGIN DBGU_CR: Debug Unit Control Register */
#define DBGU_RSTRX      1 << 2      // Reset Receiver       -   The receiver logic is reset and disabled.
                                    //                          If a character is being received, the reception is
                                    //                          aborted.
#define DBGU_RSTTX      1 << 3      // Reset Transmitter    -   The transmitter logic is reset and disabled.
                                    //                          If a character is being trasnmitted, the transmission
                                    //                          is aborted.
#define DBGU_RXEN       1 << 4      // Receiver Enable      -   The receiver is enabled if RXDIS is 0.
#define DBGU_RXDIS      1 << 5      // Receiver Disable     -   The receiver is disabled. If a character is being
                                    //                          processed and RSTRX is not set, the character is
                                    //                          completed before the receiver is stopped.
#define DBGU_TXEN       1 << 6      // Transmitter Enable   -   The transmitter is enabled if TXDIS is 0.
#define DBGU_TXDIS      1 << 7      // Transmitter Disable  -   The transmitter is disabled. If a character is being
                                    //                          processed and a character has been written the DBGU_THR
                                    //                          and RSTTX is not set, both characters are completed
                                    //                          before the transmitter is stopped.
#define DBGU_RSTSTA     1 << 8      // Reset Status Bits    -   Resets the status bits PARE, FRAME and OVRE in the
                                    //                          DBGU_SR
/* END DBGU_CR: Debug Unit Control Register */

/* BEGIN DBGU_MR: Debug Unit Mode Register */
// PAR: Parity Type
#define DBGU_PAR_EVEN               0 << 9      // Even parity
#define DBGU_PAR_ODD                1 << 9      // Odd parity
#define DBGU_PAR_SPACE              1 << 10     // Space: parity forced to 0
#define DBGU_PAR_MARK               3 << 9      // Mark: parity forced to 1
#define DBGU_PAR_NONE               1 << 11     // No parity

// CHMODE: Channel Mode
#define DBGU_CHMODE_NORMAL          0 << 14     // Normal Mode
#define DBGU_CHMODE_AUTOECHO        1 << 14     // Automatic Echo
#define DBGU_CHMODE_LOCAL_LOOPBACK  1 << 15     // Local Loopback
#define DBGU_CHMODE_REMOTE_LOOPBACK 3 << 14     // Remote Loopback
/* END DBGU_MR: Debug Unit Mode Register */

/* 
 * BEGIN
 * DBGU_IER: Debug Unit Interrupt Enable Register 
 * AND
 * DBGU_IDR: Debug Unit Interrupt Disable Register
 * AND
 * DBGU_IMR: Debug Unit Interrupt Mask Register
 * AND
 * DBGU_SR: Debug Unit Status Register
 * 
 * The following macros enable (for DBGU_IER) or disable (for DBGU_IDR) their corresponding interrupts.
 * 
 * For DBGU_IMR, they mean that the corresponding interrupt is enabled. Disabled would be 0.
 * Recall that DBGU_IMR is read-only so it serves to see the result of writing to DBGU_IER and DBGU_IDR,
 * which are write-only. We can see whether a given interrupt is currently enabled or disabled.
 * 
 * For DBGU_SR, the macros give us the status of the corresponding interrupts. See descriptions.
 */
#define DBGU_RXRDY      1 << 0              // DBGU_IER:    Enable RXRDY Interrupt
                                            // DBGU_IDR:    Disable RXRDY Interrupt
                                            // DBGU_IMR:    Mask RXRDY Interrupt
                                            // DBGU_SR:     Receiver Ready
                                            //              0 = No character has been received since the last read of
                                            //                  the DBGU_RHR or the receiver is disabled.
                                            //              1 = At least one complete character has been received,
                                            //                  transferred to DBGU_RHR and not yet read.
#define DBGU_TXRDY      1 << 1              // DBGU_IER:    Enable TXRDY Interrupt
                                            // DBGU_IDR:    Disable TXRDY Interrupt
                                            // DBGU_IMR:    Mask TXRDY Interrupt
                                            // DBGU_SR:     Transmitter Ready
                                            //              0 = A character has been written to DBGU_THR and not yet
                                            //                  transferred to the Shift Register, or the transmitter
                                            //                  is disabled.
                                            //              1 = There is no character written to DBGU_THR and not yet
                                            //                  transferred to the Shift Register.
#define DBGU_ENDRX      1 << 3              // DBGU_IER:    Enable End of Receive Transfer Interrupt
                                            // DBGU_IDR:    Disable End of Receive Transfer Interrupt
                                            // DBGU_IMR:    Mask End of Receive Transfer Interrupt
                                            // DBGU_SR:     End of Receiver Transfer
                                            //              0 = The End of Transfer signal from the receiver
                                            //                  Peripheral DMA Controller is inactive.
                                            //              1 = The End of Transfer signal from the receiver
                                            //                  Peripheral DMA Controller is active.
#define DBGU_ENDTX      1 << 4              // DBGU_IER:    Enable End of Transmit Interrupt
                                            // DBGU_IDR:    Disable End of Transmit Interrupt
                                            // DBGU_IMR:    Mask End of Transmit Interrupt
                                            // DBGU_SR:     End of Transmitter Transfer
                                            //              0 = The End of Transfer signal from the transmitter
                                            //                  Peripheral DMA Controller is inactive.
                                            //              1 = The End of Transfer signal from the transmitter
                                            //                  Peripheral DMA Controller is active.
#define DBGU_OVRE       1 << 5              // DBGU_IER:    Enable Overrun Error Interrupt
                                            // DBGU_IDR:    Disable Overrun Error Interrupt
                                            // DBGU_IMR:    Mask Overrun Error Interrupt
                                            // DBGU_SR:     Overrun Error
                                            //              0 = No overrun error has occurred since the last RSTSTA.
                                            //              1 = At least one overrun error has occurred since the last
                                            //                  RSTSTA.
#define DBGU_FRAME      1 << 6              // DBGU_IER:    Enable Framing Error Interrupt
                                            // DBGU_IDR:    Disable Framing Error Interrupt
                                            // DBGU_IMR:    Mask Framing Error Interrupt
                                            // DBGU_SR:     Framing Error
                                            //              0 = No framing error has occurred since the last RSTSTA.
                                            //              1 = At least one framing error has occurred since the last
                                            //                  RSTSTA.
#define DBGU_PARE       1 << 7              // DBGU_IER:    Enable Parity Error Interrupt
                                            // DBGU_IDR:    Disable Parity Error Interrupt
                                            // DBGU_IMR:    Mask Parity Error Interrupt
                                            // DBGU_SR:     Parity Error
                                            //              0 = No parity error has occurred since the last RSTSTA.
                                            //              1 = At least one parity error has occurred since the last
                                            //                  RSTSTA.
#define DBGU_TXEMPTY    1 << 9              // DBGU_IER:    Enable TXEMPTY Interrupt
                                            // DBGU_IDR:    Disable TXEMPTY Interrupt
                                            // DBGU_IMR:    Mask TXEMPTY Interrupt
                                            // DBGU_SR:     Transmitter Empty
                                            //              0 = There are characters in DBGU_THR, or characters being
                                            //                  processed by the transmitter, or the transmitter is
                                            //                  disabled.
                                            //              1 = There are no characters in DBGU_THR and there are no
                                            //                  characters being processed by the transmitter.
#define DBGU_TXBUFE     1 << 11             // DBGU_IER:    Enable Buffer Empty Interrupt
                                            // DBGU_IDR:    Disable Buffer Empty Interrupt
                                            // DBGU_IMR:    Mask Buffer Empty Interrupt
                                            // DBGU_SR:     Transmission Buffer Empty
                                            //              0 = The buffer empty signal from the transmitter PDC
                                            //                  channel is inactive.
                                            //              1 = The buffer empty signal from the transmitter PDC
                                            //                  channel is active.
#define DBGU_RXBUFF     1 << 12             // DBGU_IER:    Enable Buffer Full Interrupt
                                            // DBGU_IDR:    Disable Buffer Full Interrupt
                                            // DBGU_IMR:    Mask Buffer Full Interrupt
                                            // DBGU_SR:     Receive Buffer Full
                                            //              0 = The buffer full signal from the transmitter PDC
                                            //                  channel is inactive.
                                            //              1 = The buffer full signal from the transmitter PDC
                                            //                  channel is active.
#define DBGU_COMMTX     1 << 30             // DBGU_IER:    Enable COMMTX (from ARM) Interrupt
                                            // DBGU_IDR:    Disable COMMTX (from ARM) Interrupt
                                            // DBGU_IMR:    Mask COMMTX (from ARM) Interrupt
                                            // DBGU_SR:     Debug Communication Channel Write Status
                                            //              0 = COMMTX from the ARM processor is inactive.
                                            //              1 = COMMTX from the ARM processor is active.
#define DBGU_COMMRX     1 << 31             // DBGU_IER:    Enable COMMRX (from ARM) Interrupt
                                            // DBGU_IDR:    Disable COMMRX (from ARM) Interrupt
                                            // DBGU_IMR:    Mask COMMRX (from ARM) Interrupt
                                            // DBGU_SR:     Debug Communication Channel Read Status
                                            //              0 = COMMRX from the ARM processor is inactive.
                                            //              1 = COMMRX from the ARM processor is active.
/* END DBGU_IER, DBGU_IDR, DBGU_IMR, DBGU_SR */

/* END Register specifications */




/* BEGIN Functions to interact with the hardware directly */

void dbgu_rx_enable(void) {
    write_u32(DBGUB, DBGU_CR, DBGU_RXEN);
}

void dbgu_rx_disable(void) {
    write_u32(DBGUB, DBGU_CR, DBGU_RXDIS);
}

void dbgu_rx_reset(void) {
    write_u32(DBGUB, DBGU_CR, DBGU_RSTRX);
}

void dbgu_tx_enable(void) {
    write_u32(DBGUB, DBGU_CR, DBGU_TXEN);
}

void dbgu_tx_disable(void) {
    write_u32(DBGUB, DBGU_CR, DBGU_TXDIS);
}

void dbgu_tx_reset(void) {
    write_u32(DBGUB, DBGU_CR, DBGU_RSTTX);
}

void dbgu_enable(void) {
    write_u32(DBGUB, DBGU_CR, DBGU_RXEN|DBGU_TXEN);
}

void dbgu_disable(void) {
    write_u32(DBGUB, DBGU_CR, DBGU_RXDIS|DBGU_TXDIS);
}

void dbgu_reset(void) {
    write_u32(DBGUB, DBGU_CR, DBGU_RSTRX|DBGU_RSTTX);
}

void dbgu_rxrdy_interrupt_enable(void) {
    write_u32(DBGUB, DBGU_IER, DBGU_RXRDY);
}

void dbgu_rxrdy_interrupt_disable(void) {
    write_u32(DBGUB, DBGU_IDR, DBGU_RXRDY);
}

void dbgu_txrdy_interrupt_enable(void) {
    write_u32(DBGUB, DBGU_IER, DBGU_TXRDY);
}

void dbgu_txrdy_interrupt_disable(void) {
    write_u32(DBGUB, DBGU_IDR, DBGU_TXRDY);
}

/* END Functions to interact with the hardware directly */


/* BEGIN Functions abstracting direct hardware access */

/**
 * Writes a string into the DBGU by writing single characters into the Transmit Holding Register
 * until all characters have been sent.
 * 
 * @param string    A pointer to a buffer containing the string to be written
 */
void dbgu_write_string(char* string) {

    while (*string) {
        while (!(read_u8(DBGUB, DBGU_SR) & DBGU_TXRDY));
        write_u8(DBGUB, DBGU_THR, *(string++));
    }

}

/**
 * Reads a single character from the DBGU that is being held in the Receive Holding Register.
 * This function uses polling to determine whether there is a character to be read.
 * 
 * @return          The character that has been read
 */
char dbgu_read_char_poll(void) {

    while (!(read_u8(DBGUB, DBGU_SR) & DBGU_RXRDY));
    return dbgu_read_char();

}

/**
 * Reads a single character from the DBGU that is being held in the Receive Holding Register.
 * 
 * @return          The character that has been read
 */
char dbgu_read_char(void) {
    return read_u8(DBGUB, DBGU_RHR);
}

/**
 * Writes a single character into the DBGU's Transmit Holding Register.
 * 
 * @param c         The character to be written
 */
void dbgu_write_char(char c) {
    write_u8(DBGUB, DBGU_THR, c);
}

/**
 * Returns whether a character is available to be read.
 * 
 * @return          0 = No character has been received since the last read of
 *                      the DBGU_RHR or the receiver is disabled.
 *                  1 = At least one complete character has been received,
 *                      transferred to DBGU_RHR and not yet read.
 */
uint8_t dbgu_char_readable(void) {
    return read_u8(DBGUB, DBGU_SR) & DBGU_RXRDY;
}

/**
 * Returns whether a character can be written.
 * 
 * @return          0 = A character has been written to DBGU_THR and not yet
 *                      transferred to the Shift Register, or the transmitter
 *                      is disabled.
 *                  1 = There is no character written to DBGU_THR and not yet
 *                      transferred to the Shift Register.
 */
uint8_t dbgu_char_writable(void) {
    return read_u8(DBGUB, DBGU_SR) & DBGU_TXRDY;
}

/* END Functions abstracting direct hardware access */
