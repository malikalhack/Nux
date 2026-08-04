/**
 * @file    nux_protocol.h
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-02
 * @date    @showdate "%Y-%m-%d"
 */

#ifndef NUX_PROTOCOL_H_
#define NUX_PROTOCOL_H_

/******************************** Included files ******************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/********************************* Definitions ********************************/

/*
 * Nux radio link contract, shared by the vehicle and transmitter firmwares.
 * Point-to-point, half-duplex link over an SX1276 (GFSK, 868 MHz): the
 * transmitter sends a command frame every 20 ms (50 Hz); the vehicle answers
 * with a telemetry frame in the ACK window. All multi-byte fields are
 * little-endian (both endpoints are little-endian Cortex-M). Frames are packed
 * and end with a CRC-16/CCITT-FALSE computed over every preceding byte.
 */

/**
 * @def NUX_PACKED
 * @brief Portable "packed struct" attribute (AC6 armclang and GNU Arm GCC).
 */
#if defined(__GNUC__) || defined(__clang__)
#define NUX_PACKED              __attribute__((packed))
#else
#error "Unsupported compiler: define NUX_PACKED for your toolchain."
#endif /* compiler */

/*----------------------------------------------------------------------------*/

/**
 * @def NUX_PROTOCOL_VERSION
 * @brief Wire-format version; bump on any change to the frame layout.
 */
#define NUX_PROTOCOL_VERSION    (1U)

/**
 * @def NUX_FRAME_PERIOD_MS
 * @brief Command cadence: one command frame every 20 ms.
 */
#define NUX_FRAME_PERIOD_MS     (20U)

/**
 * @def NUX_FAILSAFE_MISSES
 * @brief Fail-safe trips after this many consecutive missed command frames.
 */
#define NUX_FAILSAFE_MISSES     (3U)

/*----------------------------------------------------------------------------*/

/**
 * @def NUX_AXIS_MIN
 * @brief Lower bound of a steering / throttle axis (full left / full reverse).
 */
#define NUX_AXIS_MIN            (-1000)

/**
 * @def NUX_AXIS_CENTRE
 * @brief Neutral position of a steering / throttle axis (centre / idle).
 */
#define NUX_AXIS_CENTRE         (0)

/**
 * @def NUX_AXIS_MAX
 * @brief Upper bound of a steering / throttle axis (full right / full forward).
 */
#define NUX_AXIS_MAX            (1000)

/*----------------------------------------------------------------------------*/

/**
 * @enum ENuxFrameType
 * @brief Frame type discriminator (second byte, after the version).
 */
typedef enum ENuxFrameType {
    eNuxFrameCommand   = 0x01U, /**< Transmitter -> vehicle. */
    eNuxFrameTelemetry = 0x02U  /**< Vehicle -> transmitter. */
} ENuxFrameType_t;

/*----------------------------------------------------------------------------*/

/**
 * @enum ENuxLightFlags
 * @brief Lighting control bits carried in the command frame.
 * @details The reverse light is driven automatically by the vehicle from the
 * throttle sign and is therefore not represented here. @c eNuxLightMaster is
 * the master enable: when clear, the vehicle keeps all lamps off regardless of
 * the other bits.
 */
typedef enum ENuxLightFlags {
    eNuxLightMaster    = (1U << 0), /**< Master lighting enable.       */
    eNuxLightHead      = (1U << 1), /**< Head-lights.                  */
    eNuxLightSide      = (1U << 2), /**< Side / marker lights.         */
    eNuxLightTurnLeft  = (1U << 3), /**< Left turn signal (blinks).    */
    eNuxLightTurnRight = (1U << 4)  /**< Right turn signal (blinks).   */
} ENuxLightFlags_t;

/*----------------------------------------------------------------------------*/

/**
 * @enum ENuxStatusFlags
 * @brief Vehicle status bits carried in the telemetry frame.
 */
typedef enum ENuxStatusFlags {
    eNuxStatusFailsafe   = (1U << 0), /**< Fail-safe braking is active.    */
    eNuxStatusLowBattery = (1U << 1), /**< Traction pack below threshold.  */
    eNuxStatusCharging   = (1U << 2)  /**< On-board charging detected.     */
} ENuxStatusFlags_t;

/*----------------------------------------------------------------------------*/

/**
 * @struct SNuxCommand
 * @brief Command frame: transmitter -> vehicle.
 * @details @c crc is CRC-16/CCITT-FALSE over the first
 * @c offsetof(SNuxCommand_t, crc) bytes.
 */
typedef struct NUX_PACKED SNuxCommand {
    uint8_t  version;  /**< = NUX_PROTOCOL_VERSION                         */
    uint8_t  type;     /**< = eNuxFrameCommand                            */
    uint16_t seq;      /**< Rolling counter (0..65535) for miss detection.*/
    int16_t  steering; /**< NUX_AXIS_MIN..MAX, 0 = centre.                */
    int16_t  throttle; /**< NUX_AXIS_MIN..MAX, 0 = neutral, < 0 = reverse.*/
    uint8_t  lights;   /**< Bit mask of ENuxLightFlags_t.                 */
    uint16_t crc;      /**< CRC-16/CCITT-FALSE of the preceding bytes.    */
} SNuxCommand_t;

/*----------------------------------------------------------------------------*/

/**
 * @struct SNuxTelemetry
 * @brief Telemetry frame: vehicle -> transmitter.
 * @details @c crc is CRC-16/CCITT-FALSE over the first
 * @c offsetof(SNuxTelemetry_t, crc) bytes.
 */
typedef struct NUX_PACKED SNuxTelemetry {
    uint8_t  version;     /**< = NUX_PROTOCOL_VERSION                      */
    uint8_t  type;        /**< = eNuxFrameTelemetry                       */
    uint16_t seq;         /**< Echoes the command seq being acknowledged. */
    uint16_t battery_mv;  /**< Traction pack voltage, millivolts.         */
    uint8_t  battery_pct; /**< Traction pack charge estimate, 0..100 %.   */
    uint8_t  status;      /**< Bit mask of ENuxStatusFlags_t.             */
    uint16_t crc;         /**< CRC-16/CCITT-FALSE of the preceding bytes. */
} SNuxTelemetry_t;

/*----------------------------------------------------------------------------*/

/* Wire-size guards: catch accidental padding or field reordering at build time. */
_Static_assert(sizeof(SNuxCommand_t)   == 11U,
               "SNuxCommand_t must be 11 bytes on the wire");
_Static_assert(sizeof(SNuxTelemetry_t) == 10U,
               "SNuxTelemetry_t must be 10 bytes on the wire");

/**
 * @def NUX_CMD_FRAME_SIZE
 * @brief On-wire size of a command frame, in bytes.
 */
#define NUX_CMD_FRAME_SIZE      (sizeof(SNuxCommand_t))

/**
 * @def NUX_TLM_FRAME_SIZE
 * @brief On-wire size of a telemetry frame, in bytes.
 */
#define NUX_TLM_FRAME_SIZE      (sizeof(SNuxTelemetry_t))

/********************* Application Programming Interface *********************/

/**
 * @brief Stamp a command frame header and CRC, leaving it ready to send
 * @param[in,out] frame Command frame to finalise (payload fields pre-filled)
 * @param[in]     seq   Rolling sequence counter to embed
 * @returns Whether the frame was finalised.
 * @retval true  Header and CRC written.
 * @retval false @p frame is NULL.
 */
bool nuxCommandFinalize(SNuxCommand_t *frame, uint16_t seq);

/*----------------------------------------------------------------------------*/

/**
 * @brief Validate a received command frame (version, type and CRC)
 * @param[in] frame Candidate frame
 * @returns Whether the frame is acceptable.
 * @retval true  Frame is well-formed and the CRC matches.
 * @retval false Frame is malformed or the CRC is wrong.
 */
bool nuxCommandValid(SNuxCommand_t const *frame);

/*----------------------------------------------------------------------------*/

/**
 * @brief Stamp a telemetry frame header and CRC, leaving it ready to send
 * @param[in,out] frame Telemetry frame to finalise (payload fields pre-filled)
 * @param[in]     seq   Command sequence being acknowledged
 * @returns Whether the frame was finalised.
 * @retval true  Header and CRC written.
 * @retval false @p frame is NULL.
 */
bool nuxTelemetryFinalize(SNuxTelemetry_t *frame, uint16_t seq);

/*----------------------------------------------------------------------------*/

/**
 * @brief Validate a received telemetry frame (version, type and CRC)
 * @param[in] frame Candidate frame
 * @returns Whether the frame is acceptable.
 * @retval true  Frame is well-formed and the CRC matches.
 * @retval false Frame is malformed or the CRC is wrong.
 */
bool nuxTelemetryValid(SNuxTelemetry_t const *frame);

/******************************************************************************/
#endif //! NUX_PROTOCOL_H_
