#include "notos.h"

#define NOS_INFINITE 0xffffffff
#define NOS_MAGIC    0x10242048

struct nos_timer {
    uint32_t          valid_flag;
    uint32_t          ts;
    uint32_t          period;
    uint32_t          max;
    timer_callback_t  func;
    void*             arg;
    util_queue_node_t qnode;
};

static util_queue_node_t  nos_timer_list     = util_queue_init_static(&nos_timer_list);
static util_queue_node_t  nos_ready_list_bak = util_queue_init_static(&nos_ready_list_bak);
static util_queue_node_t  nos_ready_list_b   = util_queue_init_static(&nos_ready_list_b);
static util_queue_node_t  nos_delay_list     = util_queue_init_static(&nos_delay_list);
static int                nos_timer_num      = 0;
static util_queue_node_t* _nos_task_schedule = &nos_ready_list_bak;
util_queue_node_t*        _nos_task_ready    = &nos_ready_list_b;
util_queue_node_t*        _nos_task_delay    = &nos_delay_list;
int                       _nos_task_num      = 0;


void nos_timer_start(timer_callback_t cb, void* arg, uint32_t period, uint32_t repeat, nos_timer_t* timer)
{
    if (cb == nullptr || period == 0) {
        return;
    }

    struct nos_timer* _timer = _nos_malloc(sizeof(struct nos_timer));

    if (_timer) {
        _timer->ts         = nos_gettimestamp() + period;
        _timer->period     = period;
        _timer->max        = (repeat == 0) ? NOS_INFINITE : repeat;
        _timer->func       = cb;
        _timer->arg        = arg;
        _timer->valid_flag = NOS_MAGIC;
        nos_timer_num++;
        util_queue_insert(&nos_timer_list, &_timer->qnode);
        if (timer) {
            *timer = _timer;
        }
    }
}

static void _nos_timer_stop(struct nos_timer* _timer)
{
    nos_timer_num--;
    util_queue_remove(&_timer->qnode);
    _timer->valid_flag = 0;
    _nos_free(_timer);
}

void nos_timer_stop(nos_timer_t* timer)
{
    if (timer) {
        if ((*timer)->valid_flag == NOS_MAGIC) {
            _nos_timer_stop(*timer);
        }
        *timer = nullptr;
    }
}

bool nos_schedule()
{
    uint32_t systime = nos_gettimestamp();

    // if the task_ready_schedule list is empty, ...
    if (util_queue_empty(_nos_task_schedule) && !util_queue_empty(_nos_task_ready)) {
        util_swap(util_queue_node_t*, _nos_task_schedule, _nos_task_ready);
    }

    // process the ready tasks
    util_queue_foreach_safe(node, _nos_task_schedule)
    {
        _nos_task_t* task = util_containerof(_nos_task_t, qnode, node);
        task->context->func(task, task->context->arg);
    }

    // process the delay tasks
    util_queue_foreach_safe(node, _nos_task_delay)
    {
        _nos_task_t* task = util_containerof(_nos_task_t, qnode, node);
        if (task->ts <= systime) {
            // printf("%06d.%03d: %s wake\n", systime / 1000, systime % 1000, task->name);
            util_queue_remove(&task->qnode);
            util_queue_insert(_nos_task_ready, &task->qnode);
        }
    }

    // insert the new activated task to the front of task list
    util_queue_foreach_safe(node, _nos_task_schedule)
    {
        util_queue_remove(node);
        util_queue_insert(_nos_task_ready, node);
    }
    util_swap(util_queue_node_t*, _nos_task_schedule, _nos_task_ready);

    // process the timers
    util_queue_foreach_safe(node, &nos_timer_list)
    {
        struct nos_timer* timer = util_containerof(struct nos_timer, qnode, node);
        if (timer->ts <= systime) {
            timer->func(timer->arg);
            if (timer->max != NOS_INFINITE) {
                timer->max--;
            }
            if (timer->max == 0) {
                _nos_timer_stop(timer);
            } else {
                timer->ts += timer->period;
            }
        }
    }

    return _nos_task_num + nos_timer_num;
}
