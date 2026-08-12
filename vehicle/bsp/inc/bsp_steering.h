/**
 * @file    bsp_steering.h
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-12
 * @date    @showdate "%Y-%m-%d"
 */

#ifndef BSP_STEERING_H_
#define BSP_STEERING_H_

/******************************** Included files ******************************/
#include <stdint.h>

/********************************* Definitions ********************************/

/**
 * @def BSP_STEERING_ENABLED
 * @brief Enable/disable the steering servo control (via TIM2).
 * @note  Set to 0 to completely remove this module from the build.
 */
#define BSP_STEERING_ENABLED        1

/********************* Application Programming Interface *********************/

#if (BSP_STEERING_ENABLED == 1)

/**
 * @brief Initialize the steering servo control via TIM2.
 * @details Configures TIM2 for 50 Hz PWM (20 ms period) on PB10 (TIM2_CH3).
 *          Call once after bspInit().
 *          Pulse width range: 1000..2000 us (SG90 standard: 1.0..2.0 ms).
 */
void bspSteeringInit(void);

/**
 * @brief Set the servo pulse width (non-blocking).
 * @param[in] pulse_us - pulse width in microseconds [1000..2000].
 *                       1000 us = full left, 1500 us = center, 2000 us = full right.
 * @note  Values outside the range are clamped to [1000, 2000].
 */
void bspSteeringSetPulseWidth(uint16_t pulse_us);

#else

/** @brief No-op stub when BSP_STEERING_ENABLED is 0. */
static inline void bspSteeringInit(void) { }

/** @brief No-op stub when BSP_STEERING_ENABLED is 0. */
static inline void bspSteeringSetPulseWidth(uint16_t pulse_us) {
    (void)pulse_us;
}

#endif /* BSP_STEERING_ENABLED */

/*****************************************************************************/
#endif //! BSP_STEERING_H_
