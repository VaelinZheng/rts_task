#ifndef _RTS_TASK_H_
#define _RTS_TASK_H_
#ifdef RTS_TASK_CONFIG_FILE
#    include RTS_TASK_CONFIG_FILE
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

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

#if !defined(RTS_TASK_CLOCK_IS_ATOMIC)
#define RTS_TASK_CLOCK_IS_ATOMIC 1
#endif

void rts_task_advance_ms(uint32_t ms);
uint32_t rts_task_now_ms(void);

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

#ifdef __cplusplus
}
#endif

#endif /* _RTS_TASK_H_ */
