
#include "rts_task.h"

#if defined(__C51__)
static data volatile uint32_t rts_task_clock_ms = 0;
#else
static volatile uint32_t rts_task_clock_ms = 0;
#endif

void rts_task_advance_ms(uint32_t ms)
{
    rts_task_clock_ms += ms;
}

uint32_t rts_task_now_ms(void)
{
#if RTS_TASK_CLOCK_IS_ATOMIC
    return rts_task_clock_ms;
#else
    uint32_t now;

    RTS_TASK_CLOCK_LOCK();
    now = rts_task_clock_ms;
    RTS_TASK_CLOCK_UNLOCK();

    return now;
#endif
}

#endif
