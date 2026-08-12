/**
 * @file    bsp_motor.h
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-12
 * @date    @showdate "%Y-%m-%d"
 */

#ifndef BSP_MOTOR_H_
#define BSP_MOTOR_H_

/******************************** Included files ******************************/
#include <stdint.h>

/********************************* Definitions ********************************/

/**
 * @def BSP_MOTOR_ENABLED
 * @brief Enable/disable the motor control (via TIM1).
 * @note  Set to 0 to completely remove this module from the build.
 */
#define BSP_MOTOR_ENABLED           1

/********************* Application Programming Interface *********************/

#if (BSP_MOTOR_ENABLED == 1)

/**
 * @brief Initialize the motor PWM control via TIM1.
 * @details Configures TIM1 for 1 kHz PWM on PA8 (TIM1_CH1).
 *          Call once after bspInit().
 *          Throttle range: 0..10000 (0% to 100%).
 */
void bspMotorInit(void);

/**
 * @brief Set motor throttle (PWM duty cycle).
 * @param[in] throttle - throttle value [0..10000].
 *                       0 = 0%, 10000 = 100%.
 * @note  Values outside the range are clamped to [0, 10000].
 */
void bspMotorSetThrottle(uint16_t throttle);

/**
 * @brief Set motor direction via GPIO.
 * @param[in] direction - 0=stop/idle, 1=forward, 2=reverse.
 * @note  Controls PA11 and PA12 (direction pins for H-bridge).
 */
void bspMotorSetDirection(uint8_t direction);

#else

/** @brief No-op stub when BSP_MOTOR_ENABLED is 0. */
static inline void bspMotorInit(void) { }

/** @brief No-op stub when BSP_MOTOR_ENABLED is 0. */
static inline void bspMotorSetThrottle(uint16_t throttle) { (void)throttle; }

/** @brief No-op stub when BSP_MOTOR_ENABLED is 0. */
static inline void bspMotorSetDirection(uint8_t direction) { (void)direction; }

#endif /* BSP_MOTOR_ENABLED */

#endif //! BSP_MOTOR_H_
