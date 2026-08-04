/**
 * @file    nux_protocol.c
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-03
 * @date    @showdate "%Y-%m-%d"
 */

/******************************** Included files ******************************/
#include "nux_protocol.h"
#include "nux_crc.h"

/********************* Application Programming Interface *********************/

/** @fn nuxCommandFinalize */
bool nuxCommandFinalize(SNuxCommand_t *frame, uint16_t seq) {
    bool ret_val = false;

    if (frame != NULL) {
        frame->version = NUX_PROTOCOL_VERSION;
        frame->type    = (uint8_t)eNuxFrameCommand;
        frame->seq     = seq;
        frame->crc     = nuxCrc16(frame, offsetof(SNuxCommand_t, crc));
        ret_val = true;
    }
    return ret_val;
}

/*----------------------------------------------------------------------------*/

/** @fn nuxCommandValid */
bool nuxCommandValid(SNuxCommand_t const *frame) {
    bool ret_val = false;

    if (
        (frame != NULL)                          &&
        (frame->version == NUX_PROTOCOL_VERSION) &&
        (frame->type == (uint8_t)eNuxFrameCommand)
    ) {
        ret_val = (frame->crc == nuxCrc16(frame, offsetof(SNuxCommand_t, crc)));
    }
    return ret_val;
}

/*----------------------------------------------------------------------------*/

/** @fn nuxTelemetryFinalize */
bool nuxTelemetryFinalize(SNuxTelemetry_t *frame, uint16_t seq) {
    bool ret_val = false;

    if (frame != NULL) {
        frame->version = NUX_PROTOCOL_VERSION;
        frame->type    = (uint8_t)eNuxFrameTelemetry;
        frame->seq     = seq;
        frame->crc     = nuxCrc16(frame, offsetof(SNuxTelemetry_t, crc));
        ret_val = true;
    }
    return ret_val;
}

/*----------------------------------------------------------------------------*/

/** @fn nuxTelemetryValid */
bool nuxTelemetryValid(SNuxTelemetry_t const *frame) {
    bool ret_val = false;

    if (
        (frame != NULL)                          &&
        (frame->version == NUX_PROTOCOL_VERSION) &&
        (frame->type == (uint8_t)eNuxFrameTelemetry)
    ) {
        ret_val = (frame->crc == nuxCrc16(frame, offsetof(SNuxTelemetry_t, crc)));
    }
    return ret_val;
}

/******************************************************************************/
