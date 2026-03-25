#ifndef LATENCY_PROFILER_H
#define LATENCY_PROFILER_H

/*=============================================================================
 * latency_profiler.h — Week 2
 *
 * Measures context switch latency using the DWT cycle counter.
 *
 * How it works:
 *   - A high-priority probe task records a DWT timestamp before blocking
 *   - On wakeup it reads DWT again — delta = scheduling latency
 *   - Samples are stored and bucketed into a histogram
 *   - `lp_dump_csv()` prints CSV over UART for Python to plot
 *
 * Histogram buckets (microseconds):
 *   [0-5)  [5-10)  [10-20)  [20-50)  [50-100)  [100-200)  [200+)
 *=============================================================================*/

#include <stdint.h>

#define LP_MAX_SAMPLES      256
#define LP_BUCKET_COUNT     7
#define LP_CPU_CLOCK_HZ     50000000UL   /* matches FreeRTOSConfig.h */

typedef struct {
    uint32_t min_us;
    uint32_t max_us;
    uint32_t sum_us;
    uint32_t count;
    uint32_t buckets[LP_BUCKET_COUNT];  /* histogram bins */
    uint16_t samples[LP_MAX_SAMPLES];   /* raw µs values  */
    uint32_t sample_idx;
} lp_stats_t;

void lp_init(void);
void lp_record(uint32_t cycles_elapsed);
void lp_print_stats(void);     /* formatted table over UART   */
void lp_dump_csv(void);        /* CSV dump for Python plotter */
void lp_reset(void);

#endif /* LATENCY_PROFILER_H */
