/**
 * @file autosar_os.h
 * @brief AUTOSAR OS abstraction layer mapping FreeRTOS constructs.
 *
 * AUTOSAR OS concepts modelled here:
 *
 *  | AUTOSAR Concept     | FreeRTOS Mapping                          |
 *  |---------------------|-------------------------------------------|
 *  | Basic Task          | FreeRTOS task, no event wait              |
 *  | Extended Task       | FreeRTOS task + EventGroup wait           |
 *  | ISR Category 1      | Direct vector, no OS calls                |
 *  | ISR Category 2      | Vector using FromISR APIs                 |
 *  | Resource (mutex)    | FreeRTOS Binary Semaphore / Mutex         |
 *  | Alarm               | FreeRTOS Software Timer                   |
 *  | Counter             | Tick count / hardware timer               |
 *
 * Rate Monotonic Schedulability:
 *   For n tasks: utilisation U = Σ(Ci/Ti) ≤ n(2^(1/n) - 1)
 *   Validated at startup via AutosarOS_CheckSchedulability().
 */

#ifndef AUTOSAR_OS_H
#define AUTOSAR_OS_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "event_groups.h"

/* ─── AUTOSAR Standard Return Type ─── */
typedef uint8_t StatusType;
#define E_OK            ((StatusType)0)
#define E_OS_ACCESS     ((StatusType)1)
#define E_OS_CALLEVEL   ((StatusType)2)
#define E_OS_ID         ((StatusType)3)
#define E_OS_LIMIT      ((StatusType)4)
#define E_OS_NOFUNC     ((StatusType)5)
#define E_OS_RESOURCE   ((StatusType)6)
#define E_OS_STATE      ((StatusType)7)
#define E_OS_VALUE      ((StatusType)8)

/* ─── Alarm / Counter ─── */
#define MAX_ALARMS  4

typedef void (*AlarmCallback_t)(void);

typedef struct {
    TimerHandle_t   timer;
    char            name[16];
    uint32_t        period_ms;
    AlarmCallback_t callback;
    bool            active;
} AutosarAlarm_t;

/* ─── Schedulability entry ─── */
typedef struct {
    const char *name;
    uint32_t    wcet_us;   /* worst-case execution time in microseconds */
    uint32_t    period_us; /* period in microseconds                    */
} RMSEntry_t;

/* ─── Public API ─── */

/**
 * @brief Initialise AUTOSAR OS layer (call before scheduler).
 */
void AutosarOS_Init(void);

/**
 * @brief Register and start an Alarm (periodic software timer).
 * @return E_OK or E_OS_LIMIT.
 */
StatusType AutosarOS_SetRelAlarm(const char     *name,
                                 uint32_t        period_ms,
                                 AlarmCallback_t cb);

/**
 * @brief Cancel an alarm by name.
 */
StatusType AutosarOS_CancelAlarm(const char *name);

/**
 * @brief Validate RMS schedulability for the given task set.
 *        Prints result over UART and returns true if schedulable.
 */
bool AutosarOS_CheckSchedulability(const RMSEntry_t *tasks, uint32_t n);

/**
 * @brief Print AUTOSAR OS mapping table over UART.
 */
void AutosarOS_PrintMapping(void);

#endif /* AUTOSAR_OS_H */
