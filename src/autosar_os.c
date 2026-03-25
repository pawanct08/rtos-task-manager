#include "autosar_os.h"
#include "uart_cli.h"
#include <stdbool.h>
#include <string.h>

static AutosarAlarm_t alarms[MAX_ALARMS];
static int alarm_count = 0;

void AutosarOS_Init(void) {
    for (int i = 0; i < MAX_ALARMS; i++) {
        alarms[i].active = false;
    }
}

static void TimerCallback(TimerHandle_t xTimer) {
    AlarmCallback_t cb = (AlarmCallback_t)pvTimerGetTimerID(xTimer);
    if (cb) {
        cb();
    }
}

StatusType AutosarOS_SetRelAlarm(const char *name, uint32_t period_ms, AlarmCallback_t cb) {
    if (alarm_count >= MAX_ALARMS) return E_OS_LIMIT;

    AutosarAlarm_t *a = &alarms[alarm_count++];
    strncpy(a->name, name, sizeof(a->name) - 1);
    a->name[sizeof(a->name) - 1] = '\0';
    a->period_ms = period_ms;
    a->callback = cb;
    a->active = true;

    a->timer = xTimerCreate("AlarmTimer", pdMS_TO_TICKS(period_ms), pdTRUE, (void *)cb, TimerCallback);
    if (a->timer != NULL) {
        xTimerStart(a->timer, 0);
        return E_OK;
    }
    return E_OS_VALUE;
}

StatusType AutosarOS_CancelAlarm(const char *name) {
    for (int i = 0; i < alarm_count; i++) {
        if (alarms[i].active && strncmp(alarms[i].name, name, sizeof(alarms[i].name)) == 0) {
            xTimerStop(alarms[i].timer, 0);
            alarms[i].active = false;
            return E_OK;
        }
    }
    return E_OS_ID;
}

bool AutosarOS_CheckSchedulability(const RMSEntry_t *tasks, uint32_t n) {
    if (n == 0) return true;

    double u_total = 0.0;
    for (uint32_t i = 0; i < n; i++) {
        u_total += ((double)tasks[i].wcet_us / (double)tasks[i].period_us);
    }

    /* Approximation of n(2^(1/n) - 1) up to n=8 */
    double bounds[] = { 1.0, 0.828, 0.779, 0.756, 0.743, 0.734, 0.728, 0.724 };
    double limit = (n <= 8) ? bounds[n - 1] : 0.693; // 0.693 goes to ln(2)

    return (u_total <= limit);
}
