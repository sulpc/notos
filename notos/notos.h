#ifndef _NOTOS_H_
#define _NOTOS_H_

#include "notos_.h"

/*****************************************************************************
 *                             ASYNC FUNCTION
 *****************************************************************************/

#define async_function(func_name) void func_name(_nos_task_t* _task, void* _arg)
#define async_function_start()    _tco_begin(_task->context->bp)
#define async_function_end()      _tco_end()

#define async_getarg(type) (type)(_task->context->arg)
#define async_taskname()   (_task->name)
#define async_localdata(size)                                                                                          \
    (_task->context->localdata ? _task->context->localdata : (_task->context->localdata = _nos_malloc(size)))

#define async_yeild()             _tco_yeild(_task->context->bp)
#define async_yeild_after(action) _tco_yeild_after(_task->context->bp, action)

#define async_call(_func, _arg)                                                                                        \
    do {                                                                                                               \
        _nos_context_t* _context = _nos_malloc(sizeof(_nos_context_t));                                                \
        _context->bp             = 0;                                                                                  \
        _context->func           = _func;                                                                              \
        _context->arg            = (void*)(_arg);                                                                      \
        _context->localdata      = nullptr;                                                                            \
        _context->prev           = _task->context;                                                                     \
        async_yeild_after(_task->context = _context);                                                                  \
    } while (0)

#define async_return()                                                                                                 \
    do {                                                                                                               \
        _nos_context_t* _context = _task->context;                                                                     \
        _task->context           = _context->prev;                                                                     \
        _nos_free(_context->localdata);                                                                                \
        _nos_free(_context);                                                                                           \
        if (_task->context == nullptr) {                                                                               \
            util_queue_remove(&_task->qnode);                                                                          \
            _nos_task_num--;                                                                                           \
        }                                                                                                              \
        return;                                                                                                        \
    } while (0)

#define async_sleep(_nms)                                                                                              \
    do {                                                                                                               \
        uint32_t _systime = nos_gettimestamp();                                                                        \
        _task->ts         = _nms + _systime;                                                                           \
        /* printf("%06d.%03d: %s sleep\n", _systime / 1000, _systime % 1000, _task->name); */                          \
        util_queue_remove(&_task->qnode);                                                                              \
        util_queue_insert(_nos_task_delay, &_task->qnode);                                                             \
        async_yeild();                                                                                                 \
    } while (0)

/*****************************************************************************
 *                                 NOS
 *****************************************************************************/

#define nos_task_create(_name, _func, _arg)                                                                            \
    do {                                                                                                               \
        static _nos_task_t _name##_st = {.name = #_name};                                                              \
        _name##_st.context            = _nos_malloc(sizeof(_nos_context_t));                                           \
        _name##_st.context->bp        = 0;                                                                             \
        _name##_st.context->func      = _func;                                                                         \
        _name##_st.context->arg       = (void*)(_arg);                                                                 \
        _name##_st.context->localdata = nullptr;                                                                       \
        _name##_st.context->prev      = nullptr;                                                                       \
        util_queue_insert(_nos_task_ready, &_name##_st.qnode);                                                         \
        _nos_task_num++;                                                                                               \
    } while (0)

/**
 * @brief nos schedule, should be called by user in a busy loop
 *
 * @return bool: false mean no more tasks to schedule (all tasks have been exited or deadlocked)
 */
bool nos_schedule();

/**
 * @brief implement by user, get the current timestamp (e.g. milliseconds since startup)
 *
 * @return uint32_t
 */
extern uint32_t nos_gettimestamp(void);

/*****************************************************************************
 *                                SEMAPHORE
 *****************************************************************************/

typedef struct {
    int               cnt;
    int               max;
    util_queue_node_t wait_list;
} nos_sem_t;

#define nos_sem_init(_sem, _init, _max)                                                                                \
    do {                                                                                                               \
        (_sem)->cnt = _init;                                                                                           \
        (_sem)->max = _max;                                                                                            \
        util_queue_init(&(_sem)->wait_list);                                                                           \
    } while (0)

#define nos_sem_post(_sem)                                                                                             \
    do {                                                                                                               \
        _nos_sem_post(_sem);                                                                                           \
        async_yeild();                                                                                                 \
    } while (0)

#define nos_sem_extpost(_sem)                                                                                          \
    do {                                                                                                               \
        _nos_sem_post(_sem);                                                                                           \
    } while (0)

#define nos_sem_wait(_sem)                                                                                             \
    do {                                                                                                               \
        if ((_sem)->cnt > 0) {                                                                                         \
            (_sem)->cnt--;                                                                                             \
        } else {                                                                                                       \
            util_queue_remove(&_task->qnode);                                                                          \
            util_queue_insert(&(_sem)->wait_list, &_task->qnode);                                                      \
            async_yeild();                                                                                             \
        }                                                                                                              \
    } while (0)

/*****************************************************************************
 *                                  MUTEX
 *****************************************************************************/

#define nos_mutex_t              nos_sem_t
#define nos_mutex_init(_mutex)   nos_sem_init(_mutex, 1, 1)
#define nos_mutex_lock(_mutex)   nos_sem_wait(_mutex)
#define nos_mutex_unlock(_mutex) nos_sem_post(_mutex)

/*****************************************************************************
 *                                 CONDITION
 *****************************************************************************/

#define nos_cond_t                nos_sem_t
#define nos_cond_init(_cond)      nos_sem_init(_cond, 0, 1)
#define nos_cond_wait(_cond)      nos_sem_wait(_cond)
#define nos_cond_signal(_cond)    nos_sem_post(_cond)
#define nos_cond_extsignal(_cond) nos_sem_extpost(_cond)

/*****************************************************************************
 *                                   TIMER
 *****************************************************************************/

typedef struct nos_timer* nos_timer_t;
typedef void (*timer_callback_t)(void*);

void nos_timer_start(timer_callback_t cb, void* arg, uint32_t period, uint32_t repeat, nos_timer_t* timer);
void nos_timer_stop(nos_timer_t* timer);

#endif
