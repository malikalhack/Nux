/**
 * @file    nux_crc.c
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-03
 * @date    @showdate "%Y-%m-%d"
 */

/******************************** Included files ******************************/
#include "nux_crc.h"

/********************************* Definitions ********************************/

/**
 * @def NUX_CRC16_POLY
 * @brief CRC-16/CCITT-FALSE generator polynomial.
 */
#define NUX_CRC16_POLY          0x1021U

/**
 * @def NUX_CRC16_INIT
 * @brief CRC-16/CCITT-FALSE initial (seed) value.
 */
#define NUX_CRC16_INIT          0xFFFFU

/********************* Application Programming Interface *********************/

/** @fn nuxCrc16 */
uint16_t nuxCrc16(void const *data, size_t length) {
    uint8_t const *pData = (uint8_t const *)data;
    uint16_t usCrc = NUX_CRC16_INIT;
    size_t ulIndex;
    uint8_t ucBit;

    if (pData != NULL) {
        for (ulIndex = 0U; ulIndex < length; ulIndex++) {
            usCrc ^= (uint16_t)((uint16_t)pData[ulIndex] << 8);
            for (ucBit = 0U; ucBit < 8U; ucBit++) {
                if ((usCrc & 0x8000U) != 0U) {
                    usCrc = (uint16_t)((usCrc << 1) ^ NUX_CRC16_POLY);
                }
                else {
                    usCrc = (uint16_t)(usCrc << 1);
                }
            }
        }
    }
    return usCrc;
}

/******************************************************************************/
