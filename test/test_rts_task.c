/*
 * Copyright (c) 2026, vaelin.zheng@gmail.com
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

#define CONFIG_RTS_TASK_ENABLE 1
#define CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC 1

#include "../rts_task.c"

#include "unity.h"
#include "rts_task.h"

static uint8_t g_cond;

static unsigned g_seq_steps;

static uint8_t g_wait_reached;

static uint32_t g_delay_ms;
static uint8_t g_delay_done;

static uint8_t g_delay_once_first_done;
static uint8_t g_delay_twice_done;

static uint32_t g_timeout_ms;
static uint8_t g_timeout_flag;
static uint8_t g_flags_on_entry;
static uint8_t g_wait_done;

static uint8_t g_first_timeout;
static uint8_t g_second_timeout;

static uint8_t g_set_pc_before;
static uint8_t g_set_pc_after;

static uint32_t g_delay_b_ms;
static uint8_t g_delay_b_done;

static unsigned g_periodic_iterations;
static unsigned g_periodic_yield_iterations;
static unsigned g_periodic_wait_checks;
static unsigned g_periodic_wait_completions;
static uint32_t g_periodic_timeout_ms;
static unsigned g_periodic_timeout_completions;
static uint8_t g_periodic_timeout_flag;

static RTS_TASK(task_sequence(void))
{
    RTS_TASK_BEGIN();
    g_seq_steps++;
    RTS_TASK_YIELD();
    g_seq_steps++;
    RTS_TASK_YIELD();
    g_seq_steps++;
    RTS_TASK_END();
}

static RTS_TASK(task_wait_until(void))
{
    RTS_TASK_BEGIN();
    RTS_TASK_WAIT_UNTIL(g_cond);
    g_wait_reached++;
    RTS_TASK_END();
}

static RTS_TASK(task_delay_once(void))
{
    RTS_TASK_BEGIN();
    RTS_TASK_DELAY(g_delay_ms);
    g_delay_done++;
    RTS_TASK_END();
}

static RTS_TASK(task_delay_twice(void))
{
    RTS_TASK_BEGIN();
    RTS_TASK_DELAY(100);
    g_delay_once_first_done++;
    RTS_TASK_DELAY(50);
    g_delay_twice_done++;
    RTS_TASK_END();
}

static RTS_TASK(task_wait_timeout(void))
{
    RTS_TASK_BEGIN();
    g_flags_on_entry = __task__.flags;
    RTS_TASK_WAIT_UNTIL_TIMEOUT(g_cond, g_timeout_ms);
    g_timeout_flag = (uint8_t)((__task__.flags & RTS_TASK_FLAG_TIMEOUT) != 0u);
    g_wait_done++;
    RTS_TASK_END();
}

static RTS_TASK(task_timeout_then_success(void))
{
    RTS_TASK_BEGIN();
    RTS_TASK_WAIT_UNTIL_TIMEOUT(g_cond, g_timeout_ms);
    g_first_timeout = (uint8_t)((__task__.flags & RTS_TASK_FLAG_TIMEOUT) != 0u);
    RTS_TASK_WAIT_UNTIL_TIMEOUT(1, g_timeout_ms);
    g_second_timeout = (uint8_t)((__task__.flags & RTS_TASK_FLAG_TIMEOUT) != 0u);
    RTS_TASK_END();
}

static RTS_TASK(task_set_pc(void))
{
    RTS_TASK_BEGIN();
    g_set_pc_before++;
    RTS_TASK_SET_PC();
    if(!g_cond)
    {
        return RTS_TASK_WAITING;
    }
    g_set_pc_after++;
    RTS_TASK_END();
}

typedef struct
{
    unsigned attempts;
    unsigned completed;
} task_arg_t;

/* Arguments are not stored in __task__, so bind them to a local before
   RTS_TASK_BEGIN() to keep that local valid on every resume. */
static RTS_TASK(task_with_argument(void *arg))
{
    task_arg_t *ctx = (task_arg_t *)arg;

    RTS_TASK_BEGIN();
    ctx->attempts++;
    RTS_TASK_WAIT_UNTIL(ctx->completed != 0u);
    ctx->attempts++;
    RTS_TASK_END();
}

static RTS_TASK(task_delay_b(void))
{
    RTS_TASK_BEGIN();
    RTS_TASK_DELAY(g_delay_b_ms);
    g_delay_b_done++;
    RTS_TASK_END();
}

static RTS_TASK(task_periodic(void))
{
    RTS_TASK_BEGIN();
    while(1)
    {
        g_periodic_iterations++;
        RTS_TASK_DELAY(100u);
    }
    RTS_TASK_END();
}

static RTS_TASK(task_periodic_yield(void))
{
    RTS_TASK_BEGIN();
    while(1)
    {
        g_periodic_yield_iterations++;
        RTS_TASK_YIELD();
    }
    RTS_TASK_END();
}

static RTS_TASK(task_periodic_wait(void))
{
    RTS_TASK_BEGIN();
    while(1)
    {
        g_periodic_wait_checks++;
        RTS_TASK_WAIT_UNTIL(g_cond);
        g_periodic_wait_completions++;
        g_cond = 0u;
    }
    RTS_TASK_END();
}

static RTS_TASK(task_periodic_timeout(void))
{
    RTS_TASK_BEGIN();
    while(1)
    {
        RTS_TASK_WAIT_UNTIL_TIMEOUT(g_cond, g_periodic_timeout_ms);
        g_periodic_timeout_flag =
            (uint8_t)((__task__.flags & RTS_TASK_FLAG_TIMEOUT) != 0u);
        g_periodic_timeout_completions++;
        g_cond = 0u;
    }
    RTS_TASK_END();
}

static void reset_clock(void)
{
    rts_task_advance_ms_isr(0u - rts_task_now_ms());
}

void setUp(void)
{
    reset_clock();

    g_cond                  = 0;
    g_seq_steps             = 0;
    g_wait_reached          = 0;
    g_delay_ms              = 100;
    g_delay_done            = 0;
    g_delay_once_first_done = 0;
    g_delay_twice_done      = 0;
    g_timeout_ms            = 100;
    g_timeout_flag          = 0;
    g_flags_on_entry        = 0;
    g_wait_done             = 0;
    g_first_timeout         = 0;
    g_second_timeout        = 0;
    g_set_pc_before         = 0;
    g_set_pc_after          = 0;
    g_delay_b_ms            = 200;
    g_delay_b_done          = 0;
    g_periodic_iterations   = 0;
    g_periodic_yield_iterations = 0;
    g_periodic_wait_checks      = 0;
    g_periodic_wait_completions = 0;
    g_periodic_timeout_ms       = 100u;
    g_periodic_timeout_completions = 0;
    g_periodic_timeout_flag     = 0u;
}

void tearDown(void)
{
}

static void test_clock_advances_by_isr(void)
{
    TEST_ASSERT_EQUAL_UINT32(0u, rts_task_now_ms());

    rts_task_advance_ms_isr(1234u);
    TEST_ASSERT_EQUAL_UINT32(1234u, rts_task_now_ms());

    rts_task_advance_ms_isr(0u);
    TEST_ASSERT_EQUAL_UINT32(1234u, rts_task_now_ms());
}

static void test_clock_wraps_around(void)
{
    rts_task_advance_ms_isr(0xFFFFFFFFu);
    TEST_ASSERT_EQUAL_UINT32(0xFFFFFFFFu, rts_task_now_ms());

    rts_task_advance_ms_isr(1u);
    TEST_ASSERT_EQUAL_UINT32(0u, rts_task_now_ms());
}

static void test_yield_resumes_after_each_yield(void)
{
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_sequence());
    TEST_ASSERT_EQUAL_UINT(1u, g_seq_steps);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_sequence());
    TEST_ASSERT_EQUAL_UINT(2u, g_seq_steps);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_sequence());
    TEST_ASSERT_EQUAL_UINT(3u, g_seq_steps);
}

static void test_task_restarts_from_beginning_after_done(void)
{
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_sequence());
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_sequence());
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_sequence());
    TEST_ASSERT_EQUAL_UINT(3u, g_seq_steps);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_sequence());
    TEST_ASSERT_EQUAL_UINT(4u, g_seq_steps);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_sequence());
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_sequence());
    TEST_ASSERT_EQUAL_UINT(6u, g_seq_steps);
}

static void test_task_accepts_argument(void)
{
    task_arg_t ctx = {0u, 0u};

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_with_argument(&ctx));
    TEST_ASSERT_EQUAL_UINT(1u, ctx.attempts);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_with_argument(&ctx));
    TEST_ASSERT_EQUAL_UINT(1u, ctx.attempts);

    ctx.completed = 1u;
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_with_argument(&ctx));
    TEST_ASSERT_EQUAL_UINT(2u, ctx.attempts);
}

static void test_wait_until_blocks_while_condition_false(void)
{
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_until());
    TEST_ASSERT_EQUAL_UINT(0u, g_wait_reached);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_until());
    TEST_ASSERT_EQUAL_UINT(0u, g_wait_reached);

    g_cond = 1;
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_until());
    TEST_ASSERT_EQUAL_UINT(1u, g_wait_reached);
}

static void test_wait_until_true_does_not_yield(void)
{
    g_cond = 1;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_until());
    TEST_ASSERT_EQUAL_UINT(1u, g_wait_reached);
}

static void test_set_pc_resumes_at_checkpoint(void)
{
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_set_pc());
    TEST_ASSERT_EQUAL_UINT(1u, g_set_pc_before);
    TEST_ASSERT_EQUAL_UINT(0u, g_set_pc_after);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_set_pc());
    TEST_ASSERT_EQUAL_UINT(1u, g_set_pc_before);
    TEST_ASSERT_EQUAL_UINT(0u, g_set_pc_after);

    g_cond = 1;
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_set_pc());
    TEST_ASSERT_EQUAL_UINT(1u, g_set_pc_before);
    TEST_ASSERT_EQUAL_UINT(1u, g_set_pc_after);
}

static void test_delay_waits_for_full_duration(void)
{
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_once());

    rts_task_advance_ms_isr(99u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_once());
    TEST_ASSERT_EQUAL_UINT(0u, g_delay_done);

    rts_task_advance_ms_isr(1u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_delay_once());
    TEST_ASSERT_EQUAL_UINT(1u, g_delay_done);
}

static void test_periodic_task_repeats_after_each_delay(void)
{
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic());
    TEST_ASSERT_EQUAL_UINT(1u, g_periodic_iterations);

    rts_task_advance_ms_isr(99u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic());
    TEST_ASSERT_EQUAL_UINT(1u, g_periodic_iterations);

    rts_task_advance_ms_isr(1u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic());
    TEST_ASSERT_EQUAL_UINT(2u, g_periodic_iterations);

    rts_task_advance_ms_isr(100u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic());
    TEST_ASSERT_EQUAL_UINT(3u, g_periodic_iterations);
}

static void test_periodic_task_repeats_after_each_yield(void)
{
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_yield());
    TEST_ASSERT_EQUAL_UINT(1u, g_periodic_yield_iterations);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_yield());
    TEST_ASSERT_EQUAL_UINT(2u, g_periodic_yield_iterations);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_yield());
    TEST_ASSERT_EQUAL_UINT(3u, g_periodic_yield_iterations);
}

static void test_periodic_task_waits_again_when_condition_resets(void)
{
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_wait());
    TEST_ASSERT_EQUAL_UINT(1u, g_periodic_wait_checks);
    TEST_ASSERT_EQUAL_UINT(0u, g_periodic_wait_completions);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_wait());
    TEST_ASSERT_EQUAL_UINT(1u, g_periodic_wait_checks);

    g_cond = 1u;
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_wait());
    TEST_ASSERT_EQUAL_UINT(2u, g_periodic_wait_checks);
    TEST_ASSERT_EQUAL_UINT(1u, g_periodic_wait_completions);

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_wait());
    TEST_ASSERT_EQUAL_UINT(2u, g_periodic_wait_checks);

    g_cond = 1u;
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_wait());
    TEST_ASSERT_EQUAL_UINT(3u, g_periodic_wait_checks);
    TEST_ASSERT_EQUAL_UINT(2u, g_periodic_wait_completions);
}

static void test_periodic_task_restarts_timeout_and_clears_flag(void)
{
    g_periodic_timeout_ms = 10u;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_timeout());
    TEST_ASSERT_EQUAL_UINT(0u, g_periodic_timeout_completions);

    rts_task_advance_ms_isr(10u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_timeout());
    TEST_ASSERT_EQUAL_UINT(1u, g_periodic_timeout_completions);
    TEST_ASSERT_EQUAL_UINT(1u, g_periodic_timeout_flag);

    rts_task_advance_ms_isr(5u);
    g_cond = 1u;
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_timeout());
    TEST_ASSERT_EQUAL_UINT(2u, g_periodic_timeout_completions);
    TEST_ASSERT_EQUAL_UINT(0u, g_periodic_timeout_flag);

    rts_task_advance_ms_isr(9u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_timeout());
    TEST_ASSERT_EQUAL_UINT(2u, g_periodic_timeout_completions);

    rts_task_advance_ms_isr(1u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_periodic_timeout());
    TEST_ASSERT_EQUAL_UINT(3u, g_periodic_timeout_completions);
    TEST_ASSERT_EQUAL_UINT(1u, g_periodic_timeout_flag);
}

static void test_zero_delay_completes_immediately(void)
{
    g_delay_ms = 0u;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_delay_once());
    TEST_ASSERT_EQUAL_UINT(1u, g_delay_done);
}

static void test_delay_uses_clock_wraparound_safe_arithmetic(void)
{
    rts_task_advance_ms_isr(0xFFFFFFF0u);
    g_delay_ms = 100;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_once());

    rts_task_advance_ms_isr(0x50u);
    TEST_ASSERT_EQUAL_UINT32(0x40u, rts_task_now_ms());
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_once());
    TEST_ASSERT_EQUAL_UINT(0u, g_delay_done);

    rts_task_advance_ms_isr(20u);
    TEST_ASSERT_EQUAL_UINT32(0x54u, rts_task_now_ms());
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_delay_once());
    TEST_ASSERT_EQUAL_UINT(1u, g_delay_done);
}

static void test_consecutive_delays_restart_timer(void)
{
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_twice());

    rts_task_advance_ms_isr(100u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_twice());
    TEST_ASSERT_EQUAL_UINT(1u, g_delay_once_first_done);
    TEST_ASSERT_EQUAL_UINT(0u, g_delay_twice_done);

    rts_task_advance_ms_isr(49u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_twice());

    rts_task_advance_ms_isr(1u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_delay_twice());
    TEST_ASSERT_EQUAL_UINT(1u, g_delay_once_first_done);
    TEST_ASSERT_EQUAL_UINT(1u, g_delay_twice_done);
}

static void test_tasks_keep_independent_state(void)
{
    g_delay_ms   = 100;
    g_delay_b_ms = 200;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_once());
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_b());

    rts_task_advance_ms_isr(100u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_delay_once());
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_delay_b());
    TEST_ASSERT_EQUAL_UINT(1u, g_delay_done);
    TEST_ASSERT_EQUAL_UINT(0u, g_delay_b_done);

    rts_task_advance_ms_isr(100u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_delay_b());
    TEST_ASSERT_EQUAL_UINT(1u, g_delay_b_done);
}

static void test_wait_until_timeout_times_out(void)
{
    g_timeout_ms = 100;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(0u, g_flags_on_entry);
    TEST_ASSERT_EQUAL_UINT(0u, g_wait_done);

    rts_task_advance_ms_isr(99u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(0u, g_wait_done);

    rts_task_advance_ms_isr(1u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(1u, g_wait_done);
    TEST_ASSERT_EQUAL_UINT(1u, g_timeout_flag);
}

static void test_wait_until_timeout_handles_clock_wraparound(void)
{
    rts_task_advance_ms_isr(0xFFFFFFF0u);
    g_timeout_ms = 32u;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_timeout());

    rts_task_advance_ms_isr(31u);
    TEST_ASSERT_EQUAL_UINT32(0x0Fu, rts_task_now_ms());
    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_timeout());

    rts_task_advance_ms_isr(1u);
    TEST_ASSERT_EQUAL_UINT32(0x10u, rts_task_now_ms());
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(1u, g_timeout_flag);
}

static void test_wait_until_timeout_condition_wins_before_deadline(void)
{
    g_timeout_ms = 100;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_timeout());

    rts_task_advance_ms_isr(50u);
    g_cond = 1;
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(1u, g_wait_done);
    TEST_ASSERT_EQUAL_UINT(0u, g_timeout_flag);
}

static void test_wait_until_timeout_condition_wins_at_deadline(void)
{
    g_timeout_ms = 100u;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_timeout());

    rts_task_advance_ms_isr(100u);
    g_cond = 1u;
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(1u, g_wait_done);
    TEST_ASSERT_EQUAL_UINT(0u, g_timeout_flag);
}

static void test_wait_until_timeout_condition_wins_after_deadline(void)
{
    g_timeout_ms = 100;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_timeout());

    rts_task_advance_ms_isr(150u);
    g_cond = 1;
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(1u, g_wait_done);
    TEST_ASSERT_EQUAL_UINT(0u, g_timeout_flag);
}

static void test_wait_until_timeout_zero_expires_immediately(void)
{
    g_timeout_ms = 0;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(1u, g_wait_done);
    TEST_ASSERT_EQUAL_UINT(1u, g_timeout_flag);
}

static void test_timeout_flag_is_cleared_by_next_wait(void)
{
    g_timeout_ms = 10;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_timeout_then_success());

    rts_task_advance_ms_isr(10u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_timeout_then_success());
    TEST_ASSERT_EQUAL_UINT(1u, g_first_timeout);
    TEST_ASSERT_EQUAL_UINT(0u, g_second_timeout);
}

static void test_end_clears_state_for_next_run(void)
{
    g_timeout_ms = 100;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_timeout());
    rts_task_advance_ms_isr(100u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(1u, g_timeout_flag);

    g_wait_done  = 0;
    g_timeout_flag = 0;

    TEST_ASSERT_EQUAL_INT(RTS_TASK_WAITING, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(0u, g_flags_on_entry);
    TEST_ASSERT_EQUAL_UINT(0u, g_wait_done);

    rts_task_advance_ms_isr(100u);
    TEST_ASSERT_EQUAL_INT(RTS_TASK_DONE, task_wait_timeout());
    TEST_ASSERT_EQUAL_UINT(1u, g_wait_done);
    TEST_ASSERT_EQUAL_UINT(1u, g_timeout_flag);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_clock_advances_by_isr);
    RUN_TEST(test_clock_wraps_around);
    RUN_TEST(test_yield_resumes_after_each_yield);
    RUN_TEST(test_task_restarts_from_beginning_after_done);
    RUN_TEST(test_task_accepts_argument);
    RUN_TEST(test_wait_until_blocks_while_condition_false);
    RUN_TEST(test_wait_until_true_does_not_yield);
    RUN_TEST(test_set_pc_resumes_at_checkpoint);
    RUN_TEST(test_delay_waits_for_full_duration);
    RUN_TEST(test_periodic_task_repeats_after_each_delay);
    RUN_TEST(test_periodic_task_repeats_after_each_yield);
    RUN_TEST(test_periodic_task_waits_again_when_condition_resets);
    RUN_TEST(test_periodic_task_restarts_timeout_and_clears_flag);
    RUN_TEST(test_zero_delay_completes_immediately);
    RUN_TEST(test_delay_uses_clock_wraparound_safe_arithmetic);
    RUN_TEST(test_consecutive_delays_restart_timer);
    RUN_TEST(test_tasks_keep_independent_state);
    RUN_TEST(test_wait_until_timeout_times_out);
    RUN_TEST(test_wait_until_timeout_handles_clock_wraparound);
    RUN_TEST(test_wait_until_timeout_condition_wins_before_deadline);
    RUN_TEST(test_wait_until_timeout_condition_wins_at_deadline);
    RUN_TEST(test_wait_until_timeout_condition_wins_after_deadline);
    RUN_TEST(test_wait_until_timeout_zero_expires_immediately);
    RUN_TEST(test_timeout_flag_is_cleared_by_next_wait);
    RUN_TEST(test_end_clears_state_for_next_run);

    return UNITY_END();
}
