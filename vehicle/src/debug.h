/**
 * @file    debug.h
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-12
 * @date    @showdate "%Y-%m-%d"
 */

#ifndef DEBUG_H_
#define DEBUG_H_

/******************************** Included files ******************************/
#include <stdint.h>
#include "bsp.h"

/********************************* Definitions ********************************/

/**
 * @def DEBUG_CONSOLE_ENABLED
 * @brief Enable/disable the debug console over UART.
 * @note  Controlled by BSP_DEBUG_CONSOLE; set to 0 to completely remove this module from the build.
 */
#define DEBUG_CONSOLE_ENABLED       BSP_DEBUG_CONSOLE

/****************************** Module variables ******************************/

/**
 * @struct SDebugMotorCmd
 * @brief Current motor command state (from UART console or radio).
 */
typedef struct {
    int16_t  throttle;      /**< Throttle command: [-1000, +1000]. */
    uint8_t  direction;     /**< Direction: 0=idle, 1=forward, 2=reverse. */
    uint8_t  brake_active;  /**< 1 if active braking, 0 otherwise. */
} SDebugMotorCmd;

/**
 * @struct SDebugSteeringCmd
 * @brief Current steering command state (from UART console or radio).
 */
typedef struct {
    int16_t  angle;         /**< Steering angle: [-1000, +1000]. */
} SDebugSteeringCmd;

/**
 * @struct SDebugLightCmd
 * @brief Current lighting command state (from UART console or radio).
 */
typedef struct {
    uint8_t  headlights;    /**< 1 = on, 0 = off. */
    uint8_t  sidelights;    /**< 1 = on, 0 = off. */
    uint8_t  reverse_light; /**< 1 = on, 0 = off. */
    uint8_t  left_turn;     /**< 1 = on, 0 = off. */
    uint8_t  right_turn;    /**< 1 = on, 0 = off. */
} SDebugLightCmd;

/********************* Application Programming Interface *********************/

#if (DEBUG_CONSOLE_ENABLED == 1)

/**
 * @brief Initialize the debug console (call once after bspInit).
 */
void debugInit(void);

/**
 * @brief Process one character from the debug UART.
 * @details Call this from the main loop or a periodic task. It reads available
 *          characters from the UART RX FIFO and parses commands.
 */
void debugPoll(void);

/**
 * @brief Get the current motor command state.
 * @returns Pointer to the motor command structure (read-only).
 */
const SDebugMotorCmd *debugGetMotorCmd(void);

/**
 * @brief Get the current steering command state.
 * @returns Pointer to the steering command structure (read-only).
 */
const SDebugSteeringCmd *debugGetSteeringCmd(void);

/**
 * @brief Get the current lighting command state.
 * @returns Pointer to the lighting command structure (read-only).
 */
const SDebugLightCmd *debugGetLightCmd(void);

/**
 * @brief Simulate a radio link loss (triggers fail-safe).
 * @details Sets a flag that will inject a loss-of-signal condition for testing
 *          the fail-safe and recovery logic.
 * @param[in] enable - 1 to enable loss simulation, 0 to disable.
 */
void debugSimulateLinkLoss(uint8_t enable);

/**
 * @brief Check if link loss is currently simulated.
 * @returns 1 if link loss is active, 0 otherwise.
 */
uint8_t debugIsLinkLossActive(void);

#else

/** @brief No-op stub when DEBUG_CONSOLE_ENABLED is 0. */
static inline void debugInit(void) { }

/** @brief No-op stub when DEBUG_CONSOLE_ENABLED is 0. */
static inline void debugPoll(void) { }

/** @brief No-op stub when DEBUG_CONSOLE_ENABLED is 0. */
static inline const SDebugMotorCmd *debugGetMotorCmd(void) {
    static const SDebugMotorCmd dummy = {0, 0, 0};
    return &dummy;
}

/** @brief No-op stub when DEBUG_CONSOLE_ENABLED is 0. */
static inline const SDebugSteeringCmd *debugGetSteeringCmd(void) {
    static const SDebugSteeringCmd dummy = {0};
    return &dummy;
}

/** @brief No-op stub when DEBUG_CONSOLE_ENABLED is 0. */
static inline const SDebugLightCmd *debugGetLightCmd(void) {
    static const SDebugLightCmd dummy = {0, 0, 0, 0, 0};
    return &dummy;
}

/** @brief No-op stub when DEBUG_CONSOLE_ENABLED is 0. */
static inline void debugSimulateLinkLoss(uint8_t enable) { (void)enable; }

/** @brief No-op stub when DEBUG_CONSOLE_ENABLED is 0. */
static inline uint8_t debugIsLinkLossActive(void) { return 0U; }

#endif /* DEBUG_CONSOLE_ENABLED */

/*****************************************************************************/
#endif //! DEBUG_H_
