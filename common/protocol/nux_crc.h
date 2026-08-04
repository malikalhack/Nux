/**
 * @file    nux_crc.h
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-03
 * @date    @showdate "%Y-%m-%d"
 */

#ifndef NUX_CRC_H_
#define NUX_CRC_H_

/******************************** Included files ******************************/
#include <stdint.h>
#include <stddef.h>

/********************* Application Programming Interface *********************/

/**
 * @brief Compute a CRC-16/CCITT-FALSE over a byte buffer
 * @details Polynomial 0x1021, seed 0xFFFF, no input/output reflection and no
 * final XOR — the checksum that guards every Nux radio frame.
 * @param[in] data   Start of the buffer to checksum
 * @param[in] length Number of bytes to process
 * @returns The 16-bit checksum.
 * @retval 0xFFFF @p data is NULL or @p length is zero (the untouched seed).
 */
uint16_t nuxCrc16(void const *data, size_t length);

/******************************************************************************/
#endif //! NUX_CRC_H_
