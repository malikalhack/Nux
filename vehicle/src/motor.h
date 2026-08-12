/**
 * @file    motor.h
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-12
 * @date    @showdate "%Y-%m-%d"
 */

#ifndef MOTOR_H_
#define MOTOR_H_

/******************************** Included files ******************************/
#include <stdint.h>

/********************************* Definitions ********************************/

/**
 * @def MOTOR_ENABLED
 * @brief Enable/disable the motor control module.
 * @note  Requires BSP_MOTOR_ENABLED = 1 in bsp_motor.h.
 */
#define MOTOR_ENABLED               1

/****************************** Module variables ******************************/

/**
 * @struct SMotorConfig
 * @brief Motor control configuration (ramp rates, limits).
 */
typedef struct {
    uint16_t ramp_rate_ms;      /**< Acceleration ramp time in ms (0 = immediate). */
    uint16_t max_throttle;      /**< Max throttle value [0..10000] (default 10000 = 100%). */
} SMotorConfig;

/********************* Application Programming Interface *********************/

#if (MOTOR_ENABLED == 1)

/**
 * @brief Initialize the motor control module (call once after bspInit).
 */
void motorInit(void);

/**
 * @brief Set motor command (throttle and direction).
 * @param[in] throttle - throttle [-1000..+1000]:
 *                       negative = reverse, 0 = stop, positive = forward.
 * @param[in] brake_active - 1 = active braking, 0 = coast.
 * @details The throttle is converted to PWM and direction bits.
 *          Rate limiting (ramp) is applied if configured.
 */
void motorSetCommand(int16_t throttle, uint8_t brake_active);

/**
 * @brief Get the current motor throttle.
 * @returns Current throttle [-1000..+1000] or 0 if stopped.
 */
int16_t motorGetThrottle(void);

/**
 * @brief Get the current motor direction.
 * @returns 0=stop, 1=forward, 2=reverse.
 */
uint8_t motorGetDirection(void);

/**
 * @brief Configure motor control parameters.
 * @param[in] cfg - new configuration.
 */
void motorSetConfig(const SMotorConfig *cfg);

/**
 * @brief Get current motor configuration.
 * @param[out] cfg - filled with current configuration.
 */
void motorGetConfig(SMotorConfig *cfg);

#else

/** @brief No-op stub when MOTOR_ENABLED is 0. */
static inline void motorInit(void) { }

/** @brief No-op stub when MOTOR_ENABLED is 0. */
static inline void motorSetCommand(int16_t throttle, uint8_t brake_active) {
    (void)throttle; (void)brake_active;
}

/** @brief No-op stub when MOTOR_ENABLED is 0. */
static inline int16_t motorGetThrottle(void) { return 0; }

/** @brief No-op stub when MOTOR_ENABLED is 0. */
static inline uint8_t motorGetDirection(void) { return 0; }

/** @brief No-op stub when MOTOR_ENABLED is 0. */
static inline void motorSetConfig(const SMotorConfig *cfg) { (void)cfg; }

/** @brief No-op stub when MOTOR_ENABLED is 0. */
static inline void motorGetConfig(SMotorConfig *cfg) { (void)cfg; }

#endif /* MOTOR_ENABLED */

#endif //! MOTOR_H_
