/**
 * @file    motor.c
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-12
 * @date    @showdate "%Y-%m-%d"
 */

/******************************** Included files ******************************/
#include "motor.h"
#include "bsp_motor.h"
#include <string.h>

/********************************* Definitions ********************************/

/**
 * @def MOTOR_DEFAULT_RAMP_MS
 * @brief Default acceleration ramp time in milliseconds.
 */
#define MOTOR_DEFAULT_RAMP_MS       100

/**
 * @def MOTOR_DEFAULT_MAX_THROTTLE
 * @brief Default maximum throttle (100% = 10000).
 */
#define MOTOR_DEFAULT_MAX_THROTTLE  10000

/****************************** Module variables ******************************/

/** @brief Current motor configuration. */
static SMotorConfig motor_config;

/** @brief Current throttle command (normalized). */
static int16_t motor_throttle_cmd;

/** @brief Current direction command. */
static uint8_t motor_direction_cmd;

/** @brief Current active throttle (after ramping). */
static int16_t motor_throttle_current;

/** @brief Last update timestamp (ms). */
static uint32_t motor_last_update_ms;

/***************************** Private prototypes *****************************/

/**
 * @brief Convert normalized throttle [-1000..+1000] to PWM value [0..10000].
 */
static uint16_t throttle_to_pwm(int16_t throttle);

/**
 * @brief Apply ramp rate limiting to throttle.
 */
static int16_t apply_ramp_limiting(int16_t target, int16_t current,
                                   uint16_t ramp_rate_ms, uint32_t dt_ms);

/****************************** Private functions *****************************/

/*----------------------------------------------------------------------------*/

/** @fn throttle_to_pwm */
static uint16_t throttle_to_pwm(int16_t throttle) {
    uint16_t pwm_val;

    /* Clamp to range */
    if (throttle < -1000) {
        throttle = -1000;
    }
    else if (throttle > 1000) {
        throttle = 1000;
    }

    /* Convert [-1000..+1000] to [0..10000] with center at 0 */
    if (throttle >= 0) {
        pwm_val = (uint16_t)((throttle * 10000) / 1000);
    }
    else {
        pwm_val = (uint16_t)(((-throttle) * 10000) / 1000);
    }

    /* Clamp to max configured throttle */
    if (pwm_val > motor_config.max_throttle) {
        pwm_val = motor_config.max_throttle;
    }

    return pwm_val;
}

/*----------------------------------------------------------------------------*/

/** @fn apply_ramp_limiting */
static int16_t apply_ramp_limiting(int16_t target, int16_t current,
                                   uint16_t ramp_rate_ms, uint32_t dt_ms) {
    int16_t max_delta;
    int16_t result = current;

    /* If ramp is disabled (0), apply immediately */
    if (ramp_rate_ms == 0) {
        return target;
    }

    /* Calculate max change based on elapsed time */
    if (ramp_rate_ms > 0) {
        max_delta = (int16_t)((1000 * dt_ms) / ramp_rate_ms);
        if (max_delta < 1) {
            max_delta = 1;
        }
    }
    else {
        max_delta = 0;
    }

    /* Apply ramp */
    if (target > current) {
        result = current + max_delta;
        if (result > target) {
            result = target;
        }
    }
    else if (target < current) {
        result = current - max_delta;
        if (result < target) {
            result = target;
        }
    }
    else {
        result = target;
    }

    return result;
}

/********************* Application Programming Interface *********************/

/*----------------------------------------------------------------------------*/

/** @fn motorInit */
void motorInit(void) {
    motor_config.ramp_rate_ms = MOTOR_DEFAULT_RAMP_MS;
    motor_config.max_throttle = MOTOR_DEFAULT_MAX_THROTTLE;

    motor_throttle_cmd = 0;
    motor_direction_cmd = 0;
    motor_throttle_current = 0;
    motor_last_update_ms = 0;

    bspMotorInit();
    bspMotorSetThrottle(0);
    bspMotorSetDirection(0);
}

/*----------------------------------------------------------------------------*/

/** @fn motorSetCommand */
void motorSetCommand(int16_t throttle, uint8_t brake_active) {
    uint16_t pwm_val;
    uint8_t direction;
    uint32_t now_ms;
    uint32_t dt_ms;

    /* Clamp throttle */
    if (throttle < -1000) {
        throttle = -1000;
    }
    else if (throttle > 1000) {
        throttle = 1000;
    }

    motor_throttle_cmd = throttle;

    /* Determine direction from throttle sign */
    if (throttle > 0) {
        direction = 1;  /* forward */
    }
    else if (throttle < 0) {
        direction = 2;  /* reverse */
    }
    else {
        direction = 0;  /* stop */
    }

    /* Override with brake if active */
    if (brake_active != 0) {
        direction = 0;
    }

    motor_direction_cmd = direction;

    /* Apply ramp limiting */
    now_ms = 0;  /* TODO: get actual tick from scheduler */
    dt_ms = (now_ms > motor_last_update_ms) ? (now_ms - motor_last_update_ms) : 1;
    motor_last_update_ms = now_ms;

    motor_throttle_current = apply_ramp_limiting(
        throttle, motor_throttle_current,
        motor_config.ramp_rate_ms, dt_ms
    );

    /* Convert to PWM and apply */
    pwm_val = throttle_to_pwm(motor_throttle_current);
    bspMotorSetThrottle(pwm_val);
    bspMotorSetDirection(direction);
}

/*----------------------------------------------------------------------------*/

/** @fn motorGetThrottle */
int16_t motorGetThrottle(void) {
    return motor_throttle_current;
}

/*----------------------------------------------------------------------------*/

/** @fn motorGetDirection */
uint8_t motorGetDirection(void) {
    return motor_direction_cmd;
}

/*----------------------------------------------------------------------------*/

/** @fn motorSetConfig */
void motorSetConfig(const SMotorConfig *cfg) {
    if (cfg != NULL) {
        motor_config = *cfg;
    }
}

/*----------------------------------------------------------------------------*/

/** @fn motorGetConfig */
void motorGetConfig(SMotorConfig *cfg) {
    if (cfg != NULL) {
        *cfg = motor_config;
    }
}

/******************************************************************************/
