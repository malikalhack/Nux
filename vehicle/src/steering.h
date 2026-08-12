/**
 * @file    steering.h
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-12
 * @date    @showdate "%Y-%m-%d"
 */

#ifndef STEERING_H_
#define STEERING_H_

/******************************** Included files ******************************/
#include <stdint.h>

/********************************* Definitions ********************************/

/**
 * @def STEERING_ENABLED
 * @brief Enable/disable the steering control module.
 * @note  Requires BSP_STEERING_ENABLED = 1 in bsp_steering.h.
 */
#define STEERING_ENABLED            1

/****************************** Module variables ******************************/

/**
 * @struct SSteeringConfig
 * @brief Steering servo configuration (trim, endpoints, rate limiting).
 */
typedef struct {
    int16_t trim_center;        /**< Trim offset at center (default 0). */
    int16_t limit_left;         /**< Left endpoint limit (default -1000). */
    int16_t limit_right;        /**< Right endpoint limit (default +1000). */
    uint16_t rate_limit_us;     /**< Max change in pulse width per tick [us]; 0 = unlimited. */
} SSteeringConfig;

/********************* Application Programming Interface *********************/

#if (STEERING_ENABLED == 1)

/**
 * @brief Initialize the steering control module (call once after bspInit).
 */
void steeringInit(void);

/**
 * @brief Set the steering angle command.
 * @param[in] angle - steering angle [-1000..+1000]:
 *                    -1000 = full left, 0 = center, +1000 = full right.
 * @note  The angle is converted to servo pulse width (1.0..2.0 ms) and applied.
 *        Rate limiting is applied if configured (smooth transitions).
 */
void steeringSetAngle(int16_t angle);

/**
 * @brief Get the current steering angle.
 * @returns Current angle (last command or center after init).
 */
int16_t steeringGetAngle(void);

/**
 * @brief Get the current servo pulse width in microseconds.
 * @returns Pulse width [1000..2000] us.
 */
uint16_t steeringGetPulseWidth(void);

/**
 * @brief Configure steering endpoints, trim, and rate limiting.
 * @param[in] config - pointer to configuration structure.
 *                     If NULL, defaults are restored (center=0, endpoints ±1000, no rate limit).
 */
void steeringSetConfig(const SSteeringConfig *config);

/**
 * @brief Get current steering configuration.
 * @returns Pointer to current configuration (read-only).
 */
const SSteeringConfig *steeringGetConfig(void);

#else

/** @brief No-op stub when STEERING_ENABLED is 0. */
static inline void steeringInit(void) { }

/** @brief No-op stub when STEERING_ENABLED is 0. */
static inline void steeringSetAngle(int16_t angle) { (void)angle; }

/** @brief No-op stub when STEERING_ENABLED is 0. */
static inline int16_t steeringGetAngle(void) { return 0; }

/** @brief No-op stub when STEERING_ENABLED is 0. */
static inline uint16_t steeringGetPulseWidth(void) { return 1500U; }

/** @brief No-op stub when STEERING_ENABLED is 0. */
static inline void steeringSetConfig(const SSteeringConfig *config) {
    (void)config;
}

/** @brief No-op stub when STEERING_ENABLED is 0. */
static inline const SSteeringConfig *steeringGetConfig(void) {
    static const SSteeringConfig dummy = {0, -1000, 1000, 0};
    return &dummy;
}

#endif /* STEERING_ENABLED */

/*****************************************************************************/
#endif //! STEERING_H_
