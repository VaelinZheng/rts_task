# rts_task

[![Build status](https://github.com/VaelinZheng/rts_task/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/VaelinZheng/rts_task/actions/workflows/ci.yml)

`rts_task` is a small, cooperative C task/coroutine helper for embedded
systems. A task runs until it reaches a yield, wait, or delay macro, returns
`RTS_TASK_WAITING`, and resumes from that point the next time it is called.
The project is licensed under MIT; see [LICENSE](LICENSE).

## Why rts_task?

- **No dedicated stack per task.** Each task is a stackless coroutine that
  stores a small resume state instead of a separate task stack or saved
  execution context. A call uses the normal C call stack temporarily and
  returns when it yields, waits, or delays.
- **Suitable for small MCUs.** The library has no dynamic allocation,
  preemptive context switching, or built-in scheduler, keeping its runtime
  footprint small.
- **RTOS-like sequential style.** Write task logic as ordinary C statements
  and use `RTS_TASK_YIELD()`, `RTS_TASK_WAIT_UNTIL()`, or `RTS_TASK_DELAY()` to
  suspend and resume it without manually implementing a state machine.
- **Simple cooperative scheduling.** Integrate tasks into an existing main
  loop or scheduler and advance the millisecond clock from a periodic tick.

`rts_task` is not a full RTOS: tasks are not preempted, and there are no task
priorities or per-task stacks. Every task must be called regularly, and a task
that does not yield, wait, or delay continues to occupy the caller until it
returns.

## Configuration

Enable the framework with `CONFIG_RTS_TASK_ENABLE=1`. Configure
`CONFIG_RTS_TASK_TYPE_U32_IS_ATOMIC` for the target:

- Set it to `1` when reads of the 32-bit value is atomic.
- Set it to `0` when they are not. Provide
  `RTS_TASK_TICK_INT_DISABLE` and `RTS_TASK_TICK_INT_ENABLE` macros, normally
  through the header selected by `CONFIG_RTS_TASK_TICK_INT_HEADER_FILE`.

The Kconfig options are in [Kconfig](Kconfig). If your build generates a
configuration header, set `KCONFIG_HEADER_FILE` to its path; the path and
filename can match your project's conventions. Otherwise, provide the
`CONFIG_*` definitions through the build system.

## Example

The example toggles an LED every 500 ms. Replace `board_led_toggle()`,
`board_init()`, and `SysTick_Handler()` with the matching functions for the
target board.

```c
#include "rts_task.h"

static RTS_TASK(blink_task(void))
{
    RTS_TASK_BEGIN();

    while(1)
    {
        board_led_toggle();
        RTS_TASK_DELAY(500u);
    }

    RTS_TASK_END();
}

void SysTick_Handler(void)
{
    rts_task_advance_ms_isr(1u);
}

int main(void)
{
    board_init();

    for(;;)
    {
        blink_task();
    }
}
```

Call `rts_task_advance_ms_isr()` from the periodic tick interrupt with the
number of elapsed milliseconds. The main loop (or scheduler) must call each
task regularly; the task is not a separate thread.

## Task macros

- `RTS_TASK_BEGIN()` and `RTS_TASK_END()` delimit a task body.
- `RTS_TASK_YIELD()` returns `RTS_TASK_WAITING` and resumes after the yield.
- `RTS_TASK_WAIT_UNTIL(condition)` waits until the condition is true.
- `RTS_TASK_DELAY(ms)` waits for the specified number of milliseconds.
- `RTS_TASK_WAIT_UNTIL_TIMEOUT(condition, timeout_ms)` waits for the condition
  or sets `RTS_TASK_FLAG_TIMEOUT` when the timeout expires. The condition is
  checked first, so it wins if true when the task resumes, including at or
  after the timeout deadline.
- `RTS_TASK_SET_PC()` sets a resume point without yielding. Use it before a
  manual `return RTS_TASK_WAITING`.

Each task function has its own static coroutine state and can only represent
one active task instance. Call the same function again to resume that task;
different task functions have independent state.
For a long-running periodic task, put its work inside `while(1)` so it repeats
after each yield or delay. A finite task can omit the loop and finish with
`RTS_TASK_END()` to return `RTS_TASK_DONE`.

## Important notes

- Automatic local variables are not preserved across calls that yield or wait.
  Keep persistent state in a caller-owned context object or other persistent
  storage. For a task declared with `void *arg`, pass a valid context pointer
  on every call and keep that context alive for the task's lifetime.
- Always finish the task body with `RTS_TASK_END()`. It resets the program
  counter and returns `RTS_TASK_DONE`; calling the task again starts it over.
- The tick counter is 32-bit milliseconds and wraps around approximately every
  49.7 days. Unsigned elapsed-time arithmetic handles normal wraparound, but
  delay and timeout intervals must be shorter than a full counter cycle.
- A task only checks conditions and timeouts when it is called. If the
  scheduler stops calling it, it cannot resume even though the tick continues
  advancing.

## Unit tests

The Unity tests are built from the `test` directory. CMake downloads Unity
v2.6.0 with FetchContent during configuration; no Unity source is stored in
this repository.

```sh
cmake -S test -B build -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```
