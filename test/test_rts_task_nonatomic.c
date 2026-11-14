/*
 * Copyright (c) 2026, vaelin.zheng@gmail.com
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

#define CONFIG_RTS_TASK_ENABLE 1
#define CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC 0

void rts_task_test_tick_int_disable(void);
void rts_task_test_tick_int_enable(void);

#define RTS_TASK_TICK_INT_DISABLE rts_task_test_tick_int_disable()
#define RTS_TASK_TICK_INT_ENABLE  rts_task_test_tick_int_enable()

#include "../rts_task.c"

#include "unity.h"

static unsigned g_disable_calls;
static unsigned g_enable_calls;
static unsigned g_hook_order;

void rts_task_test_tick_int_disable(void)
{
    g_disable_calls++;
    g_hook_order = (g_hook_order == 0u) ? 1u : 3u;
}

void rts_task_test_tick_int_enable(void)
{
    g_enable_calls++;
    g_hook_order = (g_hook_order == 1u) ? 2u : 4u;
}

void setUp(void)
{
    rts_task_advance_ms_isr(0u - rts_task_now_ms());

    g_disable_calls = 0u;
    g_enable_calls  = 0u;
    g_hook_order    = 0u;
}

void tearDown(void)
{
}

static void test_clock_read_disables_and_restores_interrupts(void)
{
    rts_task_advance_ms_isr(1234u);

    TEST_ASSERT_EQUAL_UINT(0u, g_disable_calls);
    TEST_ASSERT_EQUAL_UINT(0u, g_enable_calls);

    TEST_ASSERT_EQUAL_UINT32(1234u, rts_task_now_ms());
    TEST_ASSERT_EQUAL_UINT(1u, g_disable_calls);
    TEST_ASSERT_EQUAL_UINT(1u, g_enable_calls);
    TEST_ASSERT_EQUAL_UINT(2u, g_hook_order);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_clock_read_disables_and_restores_interrupts);
    return UNITY_END();
}
