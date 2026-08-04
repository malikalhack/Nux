/**
 * @file    main.c
 * @version 0.1.0
 * @authors Anton Chernov
 * @date    2026-08-04
 * @date    @showdate "%Y-%m-%d"
 *
 * @brief   Nux vehicle firmware entry point.
 */

/******************************** Included files ******************************/
#include "bsp.h"
#include "nux_sched.h"

/********************* Application Programming Interface *********************/

/** @fn main */
int main(void) {
    bspInit();
    (void)nuxSchedInit();

    /* Application tasks (radio RX, control, telemetry, lights) are added here. */

    (void)nuxSchedRun();

    /* nuxSchedRun() does not return; the loop is a safety net only. */
    for (;;) {
    }
}
/******************************************************************************/
