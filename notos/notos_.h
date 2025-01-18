/**
 * @file notos_.h
 * @author sulpc
 * @brief private header of notos, not for user
 *
 * @copyright Copyright (c) 2025
 */
#ifndef _NOTOS__H_
#define _NOTOS__H_

#include "util_queue.h"
#include "util_misc.h"

#define _nos_malloc(size) malloc(size)
#define _nos_free(ptr)    free(ptr)

// clang-format off
#define _tco_init(bp)                  bp = 0;
#define _tco_begin(bp)                 switch(bp) { case 0:
#define _tco_set(bp)                   bp = __LINE__; case __LINE__:
#define _tco_yeild(bp)                 bp = __LINE__; return; case __LINE__:
#define _tco_yeild_after(bp, action)   bp = __LINE__; action; return; case __LINE__:
#define _tco_end()                     }
// clang-format on

#define _nos_sem_post(_sem)                                                                                            \
    do {                                                                                                               \
        (_sem)->cnt = ((_sem)->cnt >= (_sem)->max) ? (_sem)->cnt : (_sem)->cnt + 1;                                    \
        if (!util_queue_empty(&(_sem)->wait_list)) {                                                                   \
            (_sem)->cnt--;                                                                                             \
            util_queue_node_t* _qnode = (_sem)->wait_list.next;                                                        \
            util_queue_remove(_qnode);                                                                                 \
            util_queue_insert(_nos_task_ready, _qnode);                                                                \
        }                                                                                                              \
    } while (0)

typedef struct _nos_task    _nos_task_t;
typedef struct _nos_context _nos_context_t;
typedef void (*_async_func_t)(_nos_task_t*, void*);

struct _nos_task {
    const char*       name;
    _nos_context_t*   context;
    uint32_t          ts;
    util_queue_node_t qnode;
};

struct _nos_context {
    int             bp;   // coroutine break point
    _async_func_t   func;
    void*           localdata;
    void*           arg;
    _nos_context_t* prev;
};

extern util_queue_node_t* _nos_task_ready;
extern util_queue_node_t* _nos_task_delay;
extern int                _nos_task_num;

#endif
