/**
 * @file    steering.c
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-12
 * @date    @showdate "%Y-%m-%d"
 */

/******************************** Included files ******************************/
#include "steering.h"
#include "bsp_steering.h"
#include <string.h>

/****************************** Module variables ******************************/

#if (STEERING_ENABLED == 1)

/** Current steering configuration */
static SSteeringConfig steering_config = {
    .trim_center = 0,
    .limit_left = -1000,
    .limit_right = 1000,
    .rate_limit_us = 0  /* No rate limit by default */
};

/** Current steering angle (last command) */
static int16_t current_angle = 0;

/** Current pulse width in microseconds */
static uint16_t current_pulse_us = 1500U;

/**
 * @brief Convert steering angle to servo pulse width.
 * @param[in] angle - steering angle [-1000..+1000].
 * @returns Pulse width in microseconds [1000..2000].
 * @details Linear conversion:
 *          angle = -1000 → pulse = 1000 us (full left)
 *          angle =     0 → pulse = 1500 us (center)
 *          angle = +1000 → pulse = 2000 us (full right)
 */
static uint16_t angle_to_pulse_us(int16_t angle) {
    int32_t tmp;

    /* Apply trim offset */
    tmp = (int32_t)angle + steering_config.trim_center;

    /* Clamp to endpoint limits */
    if (tmp < steering_config.limit_left) {
        tmp = steering_config.limit_left;
    }
    else if (tmp > steering_config.limit_right) {
        tmp = steering_config.limit_right;
    }
    else {
        /* Within limits, no change */
    }

    /* Convert to pulse width: 1500 us + angle * 0.5 us
     * (angle=0 → 1500, angle=±1000 → 1500±500)
     */
    tmp = 1500 + (tmp / 2);

    return (uint16_t)tmp;
}

/*----------------------------------------------------------------------------*/

/** @fn steeringInit */
void steeringInit(void) {
    current_angle = 0;
    current_pulse_us = 1500U;
    memset(&steering_config, 0, sizeof(steering_config));
    steering_config.trim_center = 0;
    steering_config.limit_left = -1000;
    steering_config.limit_right = 1000;
    steering_config.rate_limit_us = 0;

    bspSteeringInit();
    bspSteeringSetPulseWidth(current_pulse_us);
}

/*----------------------------------------------------------------------------*/

/** @fn steeringSetAngle */
void steeringSetAngle(int16_t angle) {
    uint16_t target_pulse = angle_to_pulse_us(angle);
    uint16_t new_pulse = target_pulse;

    /* Apply rate limiting if configured */
    if (steering_config.rate_limit_us > 0U) {
        int32_t delta = (int32_t)target_pulse - (int32_t)current_pulse_us;
        int32_t max_delta = (int32_t)steering_config.rate_limit_us;

        if (delta > max_delta) {
            new_pulse = current_pulse_us + (uint16_t)max_delta;
        }
        else if (delta < -max_delta) {
            new_pulse = current_pulse_us - (uint16_t)max_delta;
        }
        else {
            /* Within rate limit, use target */
            new_pulse = target_pulse;
        }
    }

    /* Update state and hardware */
    current_angle = angle;
    current_pulse_us = new_pulse;
    bspSteeringSetPulseWidth(new_pulse);
}

/*----------------------------------------------------------------------------*/

/** @fn steeringGetAngle */
int16_t steeringGetAngle(void) {
    return current_angle;
}

/*----------------------------------------------------------------------------*/

/** @fn steeringGetPulseWidth */
uint16_t steeringGetPulseWidth(void) {
    return current_pulse_us;
}

/*----------------------------------------------------------------------------*/

/** @fn steeringSetConfig */
void steeringSetConfig(const SSteeringConfig *config) {
    if (config == NULL) {
        /* Restore defaults */
        steering_config.trim_center = 0;
        steering_config.limit_left = -1000;
        steering_config.limit_right = 1000;
        steering_config.rate_limit_us = 0;
    } else {
        /* Copy configuration */
        memcpy(&steering_config, config, sizeof(steering_config));
    }

    /* Re-apply current angle with new config */
    steeringSetAngle(current_angle);
}

/*----------------------------------------------------------------------------*/

/** @fn steeringGetConfig */
const SSteeringConfig *steeringGetConfig(void) {
    return &steering_config;
}

#endif /* STEERING_ENABLED */

/******************************************************************************/
