/**
 * @file    nux_sched.c
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-03
 * @date    @showdate "%Y-%m-%d"
 */

/******************************** Included files ******************************/
#include "nux_sched.h"
#include "acrosched.h"
#include "acrosched_ipc.h"

/****************************** Module variables ******************************/

/** @brief Free-running 1 kHz tick counter feeding the scheduler. */
static volatile NuxTick_t nux_sched_tick = 0U;

/********************************* Definitions ********************************/

/* The adapter forwards enum values straight through; guard that the Nux and
 * AcroSched encodings stay in lock-step, and that the tick width matches the
 * ABI of the pre-built library. */
_Static_assert((int)eNuxTaskRealtime == (int)eRealtime,        "mode mismatch");
_Static_assert((int)eNuxTaskOnetime  == (int)eOnetime,         "mode mismatch");
_Static_assert((int)eNuxTaskPeriodic == (int)ePeriodic,        "mode mismatch");
_Static_assert((int)eNuxTaskLimited  == (int)eLimitedLifetime, "mode mismatch");
_Static_assert((int)eNuxTaskStandby  == (int)eStandby,         "mode mismatch");
_Static_assert((int)eNuxTaskIdle     == (int)eIdle,            "mode mismatch");

_Static_assert((int)eNuxSchedOk           == (int)eAcroOk,           "st. mismatch");
_Static_assert((int)eNuxSchedError        == (int)eAcroError,        "st. mismatch");
_Static_assert((int)eNuxSchedInvalidParam == (int)eAcroInvalidParam, "st. mismatch");
_Static_assert((int)eNuxSchedFull         == (int)eAcroFull,         "st. mismatch");

_Static_assert(sizeof(NuxTick_t) == sizeof(AcroTick_t),
               "NuxTick_t must match the library tick width");
_Static_assert(sizeof(SNuxEventGroup_t) == sizeof(SAcroEventGroup_t),
               "SNuxEventGroup_t must mirror SAcroEventGroup_t");

/********************* Application Programming Interface *********************/

/** @fn nuxSchedInit */
ENuxSchedStatus_t nuxSchedInit(void) {
    return (ENuxSchedStatus_t)acroInit(&nux_sched_tick);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxSchedRun */
ENuxSchedStatus_t nuxSchedRun(void) {
    return (ENuxSchedStatus_t)acroRun();
}

/*----------------------------------------------------------------------------*/

/** @fn nuxSchedTick */
void nuxSchedTick(void) {
    nux_sched_tick++;
}

/*----------------------------------------------------------------------------*/

/** @fn nuxSchedNow */
NuxTick_t nuxSchedNow(void) {
    return nux_sched_tick;
}

/*----------------------------------------------------------------------------*/

/** @fn nuxTaskAdd */
ENuxSchedStatus_t nuxTaskAdd(
    uint8_t         dscr,
    NuxTaskFn_t     fn,
    void           *param,
    ENuxTaskMode_t  mode,
    NuxTick_t       runTime,
    uint8_t         priority
) {
    return (ENuxSchedStatus_t)acroAddTask(
        dscr, (AcroTask_t *)fn, param, (uint8_t)mode, runTime, priority);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxTaskAddDelayed */
ENuxSchedStatus_t nuxTaskAddDelayed(
    uint8_t         dscr,
    NuxTaskFn_t     fn,
    void           *param,
    ENuxTaskMode_t  mode,
    NuxTick_t       runTime,
    NuxTick_t       delay,
    uint8_t         priority
) {
    return (ENuxSchedStatus_t)acroAddTaskWithDelay(
        dscr, (AcroTask_t *)fn, param, (uint8_t)mode, runTime, delay, priority);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxTaskKill */
ENuxSchedStatus_t nuxTaskKill(uint8_t dscr) {
    return (ENuxSchedStatus_t)acroKillTask(dscr);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxTaskKillById */
ENuxSchedStatus_t nuxTaskKillById(uint8_t id) {
    return (ENuxSchedStatus_t)acroKillTaskById(id);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxTaskSetMode */
ENuxSchedStatus_t nuxTaskSetMode(
    uint8_t         id,
    ENuxTaskMode_t  mode,
    NuxTick_t       runTime
) {
    return (ENuxSchedStatus_t)acroSetTaskMode(id, (uint8_t)mode, runTime);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxTaskStandby */
ENuxSchedStatus_t nuxTaskStandby(uint8_t id, NuxTick_t standbyTime) {
    return (ENuxSchedStatus_t)acroPutTaskOnStandby(id, standbyTime);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxTaskId */
uint8_t nuxTaskId(uint8_t dscr) {
    return acroGetTaskId(dscr);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxEventSet */
void nuxEventSet(SNuxEventGroup_t *group, NuxEventBits_t bits) {
    acroEventSet((SAcroEventGroup_t *)group, bits);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxEventGet */
NuxEventBits_t nuxEventGet(SNuxEventGroup_t const *group) {
    return acroEventGet((SAcroEventGroup_t const *)group);
}

/*----------------------------------------------------------------------------*/

/** @fn nuxEventClear */
void nuxEventClear(SNuxEventGroup_t *group, NuxEventBits_t bits) {
    acroEventClear((SAcroEventGroup_t *)group, bits);
}

/******************************************************************************/
