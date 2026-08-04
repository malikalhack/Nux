/**
 * @file    nux_sched.h
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-03
 * @date    @showdate "%Y-%m-%d"
 */

#ifndef NUX_SCHED_H_
#define NUX_SCHED_H_

/******************************** Included files ******************************/
#include <stdint.h>

/********************************* Definitions ********************************/

/*
 * Thin adapter over the frozen AcroSched library (REQ-NUX-072). Application
 * code links against this interface only and never includes the scheduler
 * headers directly, so the private scheduler stays decoupled from the public
 * Nux firmwares. The adapter also owns the 1 kHz SysTick counter that feeds the
 * scheduler (REQ-NUX-073) and re-exports the IPC event flags used to hand radio
 * frames to a task (REQ-NUX-074).
 */

/**
 * @brief System tick type, in scheduler ticks (1 kHz, wraps every ~49 days).
 */
typedef uint32_t NuxTick_t;

/**
 * @brief Task function signature: void fn(void *param).
 */
typedef void (*NuxTaskFn_t)(void *param);

/**
 * @brief Bit mask type for the event flags (up to 32 independent events).
 */
typedef uint32_t NuxEventBits_t;

/*----------------------------------------------------------------------------*/

/**
 * @enum ENuxSchedStatus
 * @brief Return status codes for the adapter API.
 */
typedef enum ENuxSchedStatus {
    eNuxSchedOk           = 0, /**< Operation completed successfully.        */
    eNuxSchedError        = 1, /**< Unspecified internal error.              */
    eNuxSchedInvalidParam = 2, /**< One or more parameters are invalid.      */
    eNuxSchedFull         = 3  /**< The task pool is full; no free slot.     */
} ENuxSchedStatus_t;

/*----------------------------------------------------------------------------*/

/**
 * @enum ENuxTaskMode
 * @brief Task execution modes exposed by the adapter.
 */
typedef enum ENuxTaskMode {
    eNuxTaskRealtime = 0, /**< Called on every dispatcher iteration.         */
    eNuxTaskOnetime  = 1, /**< Called once after a delay, then removed.      */
    eNuxTaskPeriodic = 2, /**< Called repeatedly at a fixed period.          */
    eNuxTaskLimited  = 3, /**< Called until a timeout expires, then removed. */
    eNuxTaskStandby  = 4, /**< Suspended for a given time, then resumed.     */
    eNuxTaskIdle     = 5  /**< Registered but never called.                  */
} ENuxTaskMode_t;

/*----------------------------------------------------------------------------*/

/**
 * @struct SNuxEventGroup
 * @brief Event group object for ISR-to-task signalling (radio RX, etc.).
 * @details Declare statically in application code; static storage duration
 * zero-initialises the bits, which is the required initial state.
 */
typedef struct SNuxEventGroup {
    volatile NuxEventBits_t bits; /**< Current event bits. */
} SNuxEventGroup_t;

/********************* Application Programming Interface *********************/

/**
 * @brief Initialise the scheduler and bind it to the adapter's 1 kHz tick
 * @details Must be called once before @ref nuxSchedRun. After this call the
 * SysTick ISR must invoke @ref nuxSchedTick at 1 kHz.
 * @returns Operation status.
 * @retval eNuxSchedOk    Scheduler initialised.
 * @retval eNuxSchedError Initialisation failed.
 */
ENuxSchedStatus_t nuxSchedInit(void);

/*----------------------------------------------------------------------------*/

/**
 * @brief Run the cooperative dispatcher loop
 * @details Does not return while the scheduler is running.
 * @returns Operation status.
 * @retval eNuxSchedError Scheduler was not initialised beforehand.
 */
ENuxSchedStatus_t nuxSchedRun(void);

/*----------------------------------------------------------------------------*/

/**
 * @brief Advance the scheduler time base by one tick
 * @details Call from the SysTick interrupt handler at 1 kHz.
 */
void nuxSchedTick(void);

/*----------------------------------------------------------------------------*/

/**
 * @brief Read the current scheduler tick count
 * @returns The tick counter value, in milliseconds since start.
 */
NuxTick_t nuxSchedNow(void);

/*----------------------------------------------------------------------------*/

/**
 * @brief Add a task to the scheduler pool
 * @param[in] dscr     User-defined task descriptor for later reference
 * @param[in] fn       Task function (must not be NULL)
 * @param[in] param    Argument passed to the task on each call (may be NULL)
 * @param[in] mode     Initial execution mode
 * @param[in] runTime  Period or timeout, in ticks
 * @param[in] priority Dispatch priority (higher runs first; 0 = round-robin)
 * @returns Operation status.
 * @retval eNuxSchedOk           Task added.
 * @retval eNuxSchedInvalidParam @p fn is NULL or a parameter is out of range.
 * @retval eNuxSchedFull         The task pool is full.
 */
ENuxSchedStatus_t nuxTaskAdd(
    uint8_t         dscr,
    NuxTaskFn_t     fn,
    void           *param,
    ENuxTaskMode_t  mode,
    NuxTick_t       runTime,
    uint8_t         priority
);

/*----------------------------------------------------------------------------*/

/**
 * @brief Add a task with an initial start delay
 * @param[in] dscr     User-defined task descriptor for later reference
 * @param[in] fn       Task function (must not be NULL)
 * @param[in] param    Argument passed to the task on each call (may be NULL)
 * @param[in] mode     Execution mode after the delay expires
 * @param[in] runTime  Period or timeout, in ticks
 * @param[in] delay    Initial delay before the first call, in ticks
 * @param[in] priority Dispatch priority (higher runs first; 0 = round-robin)
 * @returns Operation status.
 * @retval eNuxSchedOk           Task added.
 * @retval eNuxSchedInvalidParam @p fn is NULL or a parameter is out of range.
 * @retval eNuxSchedFull         The task pool is full.
 */
ENuxSchedStatus_t nuxTaskAddDelayed(
    uint8_t         dscr,
    NuxTaskFn_t     fn,
    void           *param,
    ENuxTaskMode_t  mode,
    NuxTick_t       runTime,
    NuxTick_t       delay,
    uint8_t         priority
);

/*----------------------------------------------------------------------------*/

/**
 * @brief Remove a task from the pool by its descriptor
 * @param[in] dscr User-defined task descriptor
 * @returns Operation status.
 * @retval eNuxSchedOk           Task removed.
 * @retval eNuxSchedInvalidParam No task with the given descriptor exists.
 */
ENuxSchedStatus_t nuxTaskKill(uint8_t dscr);

/*----------------------------------------------------------------------------*/

/**
 * @brief Remove a task from the pool by its ID
 * @param[in] id Task ID obtained from @ref nuxTaskId
 * @returns Operation status.
 * @retval eNuxSchedOk           Task removed.
 * @retval eNuxSchedInvalidParam No task with the given ID exists.
 */
ENuxSchedStatus_t nuxTaskKillById(uint8_t id);

/*----------------------------------------------------------------------------*/

/**
 * @brief Change the execution mode and timing of a task
 * @param[in] id      Task ID obtained from @ref nuxTaskId
 * @param[in] mode    New execution mode
 * @param[in] runTime New period or timeout, in ticks
 * @returns Operation status.
 * @retval eNuxSchedOk           Mode changed.
 * @retval eNuxSchedInvalidParam ID not found or mode out of range.
 */
ENuxSchedStatus_t nuxTaskSetMode(
    uint8_t         id,
    ENuxTaskMode_t  mode,
    NuxTick_t       runTime
);

/*----------------------------------------------------------------------------*/

/**
 * @brief Suspend a task for a time, then resume its previous mode
 * @param[in] id          Task ID obtained from @ref nuxTaskId
 * @param[in] standbyTime Suspension duration, in ticks
 * @returns Operation status.
 * @retval eNuxSchedOk           Task suspended.
 * @retval eNuxSchedInvalidParam No task with the given ID exists.
 */
ENuxSchedStatus_t nuxTaskStandby(uint8_t id, NuxTick_t standbyTime);

/*----------------------------------------------------------------------------*/

/**
 * @brief Resolve a task descriptor to its ID
 * @param[in] dscr User-defined task descriptor
 * @returns The task ID, or 0 if no task with that descriptor exists.
 */
uint8_t nuxTaskId(uint8_t dscr);

/*----------------------------------------------------------------------------*/

/**
 * @brief Set one or more event bits in an event group
 * @details Safe to call from an interrupt handler.
 * @param[in] group Event group (must not be NULL)
 * @param[in] bits  Bit mask of events to signal; already-set bits are kept
 */
void nuxEventSet(SNuxEventGroup_t *group, NuxEventBits_t bits);

/*----------------------------------------------------------------------------*/

/**
 * @brief Read the current event bits of an event group
 * @param[in] group Event group (must not be NULL)
 * @returns Snapshot of the event bits at the time of the call.
 */
NuxEventBits_t nuxEventGet(SNuxEventGroup_t const *group);

/*----------------------------------------------------------------------------*/

/**
 * @brief Clear one or more event bits in an event group
 * @param[in] group Event group (must not be NULL)
 * @param[in] bits  Bit mask of events to clear; other bits are kept
 */
void nuxEventClear(SNuxEventGroup_t *group, NuxEventBits_t bits);

/******************************************************************************/
#endif //! NUX_SCHED_H_
