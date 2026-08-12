/**
 * @file    debug.c
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-12
 * @date    @showdate "%Y-%m-%d"
 */

/******************************** Included files ******************************/
#include "debug.h"
#include "bsp.h"
#include <string.h>

/****************************** Module variables ******************************/

#if (DEBUG_CONSOLE_ENABLED == 1)

/** Current motor command state. */
static SDebugMotorCmd motor_cmd = {0, 0, 0};

/** Current steering command state. */
static SDebugSteeringCmd steering_cmd = {0};

/** Current lighting command state. */
static SDebugLightCmd light_cmd = {0, 0, 0, 0, 0};

/** Link loss simulation flag. */
static uint8_t link_loss_simulated = 0U;

/** Command line buffer and parsing state. */
static char cmd_buffer[64];
static uint8_t cmd_index = 0U;

/***************************** Private prototypes *****************************/

/**
 * @brief Parse and execute a debug command from the buffer.
 * @param[in] cmd - null-terminated command string.
 */
static void debug_parse_command(const char *cmd);

/**
 * @brief Helper to parse a signed 16-bit integer from a string.
 * @param[in] str - string to parse.
 * @param[out] val - parsed value.
 * @returns 1 if successful, 0 if parse error.
 */
static uint8_t parse_int16(const char *str, int16_t *val);

/**
 * @brief Helper to parse an unsigned 8-bit integer from a string.
 * @param[in] str - string to parse.
 * @param[out] val - parsed value.
 * @returns 1 if successful, 0 if parse error.
 */
static uint8_t parse_uint8(const char *str, uint8_t *val);

/**
 * @brief Print the help menu to the console.
 */
static void debug_print_help(void);

/**
 * @brief Print the current state of all command structures.
 */
static void debug_print_state(void);

/**
 * @brief Handle received character (called by poll/IRQ).
 * @details Builds up a command in cmd_buffer and executes it when Enter is pressed.
 * @param[in] c - the received character.
 */
static void debug_handle_rx_char(uint8_t c);

/****************************** Private functions *****************************/

/** @fn parse_int16 */
static uint8_t parse_int16(const char *str, int16_t *val) {
    int32_t tmp = 0;
    int8_t  sign = 1;
    uint8_t digits = 0U;

    if (str == NULL || val == NULL) {
        return 0U;
    }

    /* Skip leading whitespace */
    while (*str == ' ' || *str == '\t') {
        str++;
    }

    /* Check for sign */
    if (*str == '-') {
        sign = -1;
        str++;
    }
    else if (*str == '+') {
        str++;
    }
    else {
        /* No sign, use default (positive) */
    }

    /* Parse digits */
    while (*str >= '0' && *str <= '9') {
        tmp = tmp * 10 + (int32_t)(*str - '0');
        digits++;
        str++;
    }

    if (digits == 0U || tmp > 32767L) {
        return 0U;
    }

    *val = (int16_t)(sign * tmp);
    return 1U;
}

/** @fn parse_uint8 */
static uint8_t parse_uint8(const char *str, uint8_t *val) {
    uint32_t tmp = 0U;
    uint8_t  digits = 0U;

    if (str == NULL || val == NULL) {
        return 0U;
    }

    /* Skip leading whitespace */
    while (*str == ' ' || *str == '\t') {
        str++;
    }

    /* Parse digits */
    while (*str >= '0' && *str <= '9') {
        tmp = tmp * 10U + (uint32_t)(*str - '0');
        digits++;
        str++;
    }

    if (digits == 0U || tmp > 255UL) {
        return 0U;
    }

    *val = (uint8_t)tmp;
    return 1U;
}

/** @fn debug_print_help */
static void debug_print_help(void) {
    uartSendStr(
        "\r\n=== Nux Debug Console (v0.1) ===\r\n"
        "Commands:\r\n"
        "  motor <throttle> <dir> <brake>    Set motor: throttle [-1000..1000], dir [0=idle,1=fwd,2=rev], brake [0|1]\r\n"
        "  steer <angle>                     Set steering: angle [-1000..1000]\r\n"
        "  light <h> <s> <r> <l> <r>        Set lights: headlights, sidelights, reverse, left_turn, right_turn [0|1]\r\n"
        "  loslink <0|1>                     Simulate link loss (0=off, 1=on)\r\n"
        "  state                             Print current command state\r\n"
        "  help                              Print this menu\r\n"
        "Example: motor 500 1 0   (50% throttle forward, no brake)\r\n"
        "> "
    );
}

/** @fn debug_print_state */
static void debug_print_state(void) {
    uartSendStr("\r\n--- Current State ---\r\n");
    
    uartSendStr("Motor: throttle=");
    if (motor_cmd.throttle < 0) {
        uartSendChar('-');
        uartSendUint16((uint16_t)(-motor_cmd.throttle));
    } else {
        uartSendUint16((uint16_t)motor_cmd.throttle);
    }
    uartSendStr(" dir=");
    uartSendUint8(motor_cmd.direction);
    uartSendStr(" brake=");
    uartSendUint8(motor_cmd.brake_active);
    uartSendStr("\r\n");

    uartSendStr("Steering: angle=");
    if (steering_cmd.angle < 0) {
        uartSendChar('-');
        uartSendUint16((uint16_t)(-steering_cmd.angle));
    } else {
        uartSendUint16((uint16_t)steering_cmd.angle);
    }
    uartSendStr("\r\n");

    uartSendStr("Lights: h=");
    uartSendUint8(light_cmd.headlights);
    uartSendStr(" s=");
    uartSendUint8(light_cmd.sidelights);
    uartSendStr(" r=");
    uartSendUint8(light_cmd.reverse_light);
    uartSendStr(" lt=");
    uartSendUint8(light_cmd.left_turn);
    uartSendStr(" rt=");
    uartSendUint8(light_cmd.right_turn);
    uartSendStr("\r\n");

    uartSendStr("Link loss simulated: ");
    uartSendUint8(link_loss_simulated);
    uartSendStr("\r\n> ");
}

/** @fn debug_parse_command */
static void debug_parse_command(const char *cmd) {
    int16_t  ival;
    uint8_t  uval;
    uint8_t  u1, u2, u3, u4, u5;
    const char *arg;

    if (cmd == NULL || cmd[0] == '\0') {
        return;
    }

    /* "motor" command */
    if (strncmp(cmd, "motor", 5U) == 0) {
        arg = &cmd[5];
        if (parse_int16(arg, &ival)) {
            motor_cmd.throttle = ival;
            /* Skip to next argument */
            while (*arg != ' ' && *arg != '\0') arg++;
            if (parse_uint8(arg, &uval)) {
                motor_cmd.direction = uval;
                /* Skip to next argument */
                while (*arg != ' ' && *arg != '\0') arg++;
                if (parse_uint8(arg, &uval)) {
                    motor_cmd.brake_active = uval;
                    uartSendStr("\r\nMotor command set\r\n> ");
                    return;
                }
            }
        }
        uartSendStr("\r\nMotor: error parsing (usage: motor <throttle> <dir> <brake>)\r\n> ");
        return;
    }

    /* "steer" command */
    if (strncmp(cmd, "steer", 5U) == 0) {
        arg = &cmd[5];
        if (parse_int16(arg, &ival)) {
            steering_cmd.angle = ival;
            uartSendStr("\r\nSteering command set\r\n> ");
            return;
        }
        uartSendStr("\r\nSteering: error parsing (usage: steer <angle>)\r\n> ");
        return;
    }

    /* "light" command */
    if (strncmp(cmd, "light", 5U) == 0) {
        arg = &cmd[5];
        if (parse_uint8(arg, &u1)) {
            while (*arg != ' ' && *arg != '\0') arg++;
            if (parse_uint8(arg, &u2)) {
                while (*arg != ' ' && *arg != '\0') arg++;
                if (parse_uint8(arg, &u3)) {
                    while (*arg != ' ' && *arg != '\0') arg++;
                    if (parse_uint8(arg, &u4)) {
                        while (*arg != ' ' && *arg != '\0') arg++;
                        if (parse_uint8(arg, &u5)) {
                            light_cmd.headlights = u1;
                            light_cmd.sidelights = u2;
                            light_cmd.reverse_light = u3;
                            light_cmd.left_turn = u4;
                            light_cmd.right_turn = u5;
                            uartSendStr("\r\nLight command set\r\n> ");
                            return;
                        }
                    }
                }
            }
        }
        uartSendStr("\r\nLight: error parsing (usage: light <h> <s> <r> <lt> <rt>)\r\n> ");
        return;
    }

    /* "loslink" command (simulate link loss) */
    if (strncmp(cmd, "loslink", 7U) == 0) {
        arg = &cmd[7];
        if (parse_uint8(arg, &uval)) {
            link_loss_simulated = (uval != 0U) ? 1U : 0U;
            uartSendStr("\r\nLink loss simulation ");
            uartSendStr(link_loss_simulated ? "enabled" : "disabled");
            uartSendStr("\r\n> ");
            return;
        }
        uartSendStr("\r\nLoslink: error parsing (usage: loslink <0|1>)\r\n> ");
        return;
    }

    /* "state" command */
    if (strncmp(cmd, "state", 5U) == 0) {
        debug_print_state();
        return;
    }

    /* "help" command */
    if (strncmp(cmd, "help", 4U) == 0) {
        debug_print_help();
        return;
    }

    /* Unrecognized command */
    uartSendStr("\r\nUnknown command. Type 'help' for a list.\r\n> ");
}

/********************* Application Programming Interface *********************/

/** @fn debugInit */
void debugInit(void) {
    memset(&motor_cmd, 0, sizeof(motor_cmd));
    memset(&steering_cmd, 0, sizeof(steering_cmd));
    memset(&light_cmd, 0, sizeof(light_cmd));
    link_loss_simulated = 0U;
    cmd_index = 0U;
    memset(cmd_buffer, 0, sizeof(cmd_buffer));

    debug_print_help();
}

/** @fn debugPoll */
void debugPoll(void) {
    char c;
    /* Poll for received data via BSP UART API */
    if (uartRecvChar(&c) != 0U) {
        debug_handle_rx_char((uint8_t)c);
    }
}

/** @fn debugGetMotorCmd */
const SDebugMotorCmd *debugGetMotorCmd(void) {
    return &motor_cmd;
}

/** @fn debugGetSteeringCmd */
const SDebugSteeringCmd *debugGetSteeringCmd(void) {
    return &steering_cmd;
}

/** @fn debugGetLightCmd */
const SDebugLightCmd *debugGetLightCmd(void) {
    return &light_cmd;
}

/** @fn debugSimulateLinkLoss */
void debugSimulateLinkLoss(uint8_t enable) {
    link_loss_simulated = (enable != 0U) ? 1U : 0U;
}

/** @fn debugIsLinkLossActive */
uint8_t debugIsLinkLossActive(void) {
    return link_loss_simulated;
}

/*----------------------------------------------------------------------------*/

/**
 * @brief Handle received character (called by poll/IRQ).
 * @details Builds up a command in cmd_buffer and executes it when Enter is pressed.
 * @param[in] c - the received character.
 */
static void debug_handle_rx_char(uint8_t c) {
    if (c == '\r' || c == '\n') {
        /* Execute command if buffer is not empty */
        if (cmd_index > 0U) {
            cmd_buffer[cmd_index] = '\0';
            debug_parse_command(cmd_buffer);
            cmd_index = 0U;
            memset(cmd_buffer, 0, sizeof(cmd_buffer));
        }
    }
    else if (c == '\x08' || c == '\x7F') {
        /* Backspace: erase one character */
        if (cmd_index > 0U) {
            cmd_index--;
            uartSendStr("\x08 \x08");  /* Backspace, space, backspace for display */
        }
    }
    else if (c >= 0x20 && c < 0x7F) {
        /* Printable character: add to buffer */
        if (cmd_index < (sizeof(cmd_buffer) - 1U)) {
            cmd_buffer[cmd_index++] = (char)c;
            uartSendChar((char)c);  /* Echo */
        }
    }
    else {
        /* Non-printable, non-backspace character, ignore */
    }
}

#else  /* DEBUG_CONSOLE_ENABLED == 0 */

/* No-op implementations are already provided as inline stubs in the header. */

#endif /* DEBUG_CONSOLE_ENABLED */

/******************************************************************************/
