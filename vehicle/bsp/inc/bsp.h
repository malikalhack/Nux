/**
 * @file    bsp.h
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-04
 * @date    @showdate "%Y-%m-%d"
 */

#ifndef BSP_H_
#define BSP_H_
/******************************** Included files ******************************/
#include <stdint.h>
/********************************* Definitions *******************************/

/**
 * @def BSP_TICKS_PER_SEC
 * @brief SysTick timer frequency in Hz (scheduler time base).
 */
#define BSP_TICKS_PER_SEC   1000U

/**
 * @enum EResetCause
 * @brief Cause of the most recent MCU reset, as reported by @ref reset_cause.
 */
enum EResetCause {
    eSoftware = 0, /**< Software reset (NVIC_SystemReset).       */
    eLowPower = 1, /**< Low-power / standby exit reset.          */
    eWatchDog = 2, /**< Independent or window watchdog reset.    */
    eBrownOut = 3, /**< Power-on / brown-out reset.              */
    ePin      = 4, /**< External NRST pin reset.                 */
    eUnknown  = 5  /**< Cause could not be determined.           */
};

/********************* Application Programming Interface *********************/

/**
 * @brief Configures SysTick for 1 kHz ticks and enables global interrupts.
 * @details Call from main() after startup has run SystemInit() (clocks, GPIO
 *          and UART are already configured by then).
 */
void bspInit(void);

/**
 * @brief Get the system core clock frequency.
 * @returns System core clock frequency in Hz.
 */
uint32_t get_system_core_clock(void);

/** @brief Reset the MCU. */
void reset_mcu(void);

/**
 * @brief Reads and clears the cause of the last reset.
 * @returns One of @ref EResetCause.
 */
unsigned short reset_cause(void);

/**
 * @brief Arms the independent watchdog (IWDG) with a fixed timeout.
 * @details Once started the watchdog cannot be stopped except by a reset; the
 *          application must call bspWatchdogKick() more often than the timeout
 *          or the MCU is reset. Drives a nominal ~2 s timeout from the LSI.
 */
void bspWatchdogStart(void);

/** @brief Reloads (kicks) the independent watchdog to postpone its reset. */
void bspWatchdogKick(void);

/**
 * @brief Transmits a single character over the debug UART (blocking).
 * @param[in] c - character to transmit.
 */
void uartSendChar(char c);

/**
 * @brief Transmits a null-terminated string over the debug UART (blocking).
 * @param[in] str - pointer to the string to transmit (must not be NULL).
 */
void uartSendStr(const char *str);

/**
 * @brief Transmits an unsigned 8-bit integer as decimal digits over UART.
 * @param[in] n - value to transmit (0..255).
 */
void uartSendUint8(uint8_t n);

/**
 * @brief Transmits an unsigned 16-bit integer as decimal digits over UART.
 * @details Leading zeros are suppressed; the value 0 is printed as "0".
 * @param[in] n - value to transmit (0..65535).
 */
void uartSendUint16(uint16_t n);

/*****************************************************************************/
#endif //! BSP_H_
