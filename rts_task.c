/*
 * Copyright (c) 2026, vaelin.zheng@gmail.com
 *
 * SPDX-License-Identifier: MIT
 */

#include "rts_task.h"
#if (CONFIG_RTS_TASK_ENABLE)

#if defined(__C51__)
#undef CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC
#define CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC 0
static data volatile uint32_t rts_task_clock_ms = 0;
#else
static volatile uint32_t rts_task_clock_ms = 0;
#endif

#ifndef CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC
#error "CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC is not defined"
#endif

#if (CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC == 0)
#ifdef CONFIG_RTS_TASK_TICK_INT_HEADER_FILE
#include CONFIG_RTS_TASK_TICK_INT_HEADER_FILE
#endif
#ifndef RTS_TASK_TICK_INT_DISABLE
#error "CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC is 0, RTS_TASK_TICK_INT_DISABLE must be defined"
#endif
#ifndef RTS_TASK_TICK_INT_ENABLE
#error "CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC is 0, RTS_TASK_TICK_INT_ENABLE must be defined"
#endif
#else
#define RTS_TASK_TICK_INT_DISABLE do{} while(0)
#define RTS_TASK_TICK_INT_ENABLE  do{} while(0)
#endif

void rts_task_advance_ms_isr(uint32_t ms)
{
    rts_task_clock_ms += ms;
}

uint32_t rts_task_now_ms(void)
{
#if CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC
    return rts_task_clock_ms;
#else
    uint32_t now;

    RTS_TASK_TICK_INT_DISABLE;
    now = rts_task_clock_ms;
    RTS_TASK_TICK_INT_ENABLE;

    return now;
#endif
}

#endif /* CONFIG_RTS_TASK_ENABLE */
