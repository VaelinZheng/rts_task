/*
 * Copyright (c) 2026, vaelin.zheng@gmail.com
 *
 * SPDX-License-Identifier: MIT
 */

#define CONFIG_RTS_TASK_ENABLE 0

#include "../rts_task.c"

#ifdef RTS_TASK
#error "RTS_TASK must not be defined when the framework is disabled"
#endif

#ifdef RTS_TASK_BEGIN
#error "RTS_TASK_BEGIN must not be defined when the framework is disabled"
#endif

int main(void)
{
    return 0;
}
