/*
 * Copyright (c) 2026, vaelin.zheng@gmail.com
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _RTS_TASK_H_
#define _RTS_TASK_H_
#if defined(KCONFIG_HEADER_FILE)
#if defined(__C51__) || defined(__ARMCC_VERSION)
#define _RTS_MACRO_TO_STR_IMPL(s) #s
#define _RTS_MACRO_TO_STR(s)      _RTS_MACRO_TO_STR_IMPL(s)
#include _RTS_MACRO_TO_STR(KCONFIG_HEADER_FILE)
#undef _RTS_MACRO_TO_STR
#undef _RTS_MACRO_TO_STR_IMPL
#else
#include KCONFIG_HEADER_FILE
#endif
#endif

#if (CONFIG_RTS_TASK_ENABLE)
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define RTS_TASK_VERSION_MAJOR 0
#define RTS_TASK_VERSION_MINOR 1
#define RTS_TASK_VERSION_PATCH 0
#define RTS_TASK_VERSION ((RTS_TASK_VERSION_MAJOR << 16) | (RTS_TASK_VERSION_MINOR << 8) | RTS_TASK_VERSION_PATCH)

    typedef struct
    {
        uint16_t pc;
        uint8_t flags;
        uint32_t tick;
    } rts_task_t;

    typedef enum
    {
        RTS_TASK_WAITING = 0,
        RTS_TASK_DONE    = 1,
    } rts_task_state_t;

#define RTS_TASK_FLAG_TIMEOUT ((uint8_t)0x01)

#define RTS_TASK(name)        rts_task_state_t name

#define RTS_TASK_SET_PC()   \
    __task__.pc = __LINE__; \
    case __LINE__:

#define RTS_TASK_YIELD()     \
    __task__.pc = __LINE__;  \
    return RTS_TASK_WAITING; \
    case __LINE__:

#define RTS_TASK_WAIT_UNTIL(cond)    \
    do                               \
    {                                \
        RTS_TASK_SET_PC();           \
        if(!(cond))                  \
        {                            \
            return RTS_TASK_WAITING; \
        }                            \
    } while(0)

#define RTS_TASK_WAIT_UNTIL_TIMEOUT(cond, timeout_ms)                                  \
    do                                                                                 \
    {                                                                                  \
        __task__.flags &= (uint8_t)~RTS_TASK_FLAG_TIMEOUT;                             \
        __task__.tick = rts_task_now_ms();                                             \
        RTS_TASK_SET_PC();                                                             \
        if(!(cond))                                                                    \
        {                                                                              \
            if((uint32_t)(rts_task_now_ms() - __task__.tick) < (uint32_t)(timeout_ms)) \
            {                                                                          \
                return RTS_TASK_WAITING;                                               \
            }                                                                          \
            __task__.flags |= RTS_TASK_FLAG_TIMEOUT;                                   \
        }                                                                              \
    } while(0)

#define RTS_TASK_DELAY(ms)                                                 \
    do                                                                     \
    {                                                                      \
        __task__.tick = rts_task_now_ms();                                 \
        RTS_TASK_SET_PC();                                                 \
        if((uint32_t)(rts_task_now_ms() - __task__.tick) < (uint32_t)(ms)) \
        {                                                                  \
            return RTS_TASK_WAITING;                                       \
        }                                                                  \
    } while(0)

#define RTS_TASK_BEGIN()                  \
    {                                     \
        static rts_task_t __task__ = {0}; \
        switch(__task__.pc)               \
        {                                 \
        case 0:

#define RTS_TASK_END()    \
    }                     \
    __task__.pc    = 0;   \
    __task__.flags = 0;   \
    __task__.tick  = 0;   \
    return RTS_TASK_DONE; \
    }

    void rts_task_advance_ms_isr(uint32_t ms);
    uint32_t rts_task_now_ms(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_RTS_TASK_ENABLE */
#endif /* _RTS_TASK_H_ */
