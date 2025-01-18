#include "notos.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>


uint32_t nos_gettimestamp(void)
{
    static struct timeval startup;
    static bool           inited = false;
    struct timeval        tv;

    if (!inited) {
        gettimeofday(&startup, nullptr);
        inited = true;
        return 0;
    } else {
        gettimeofday(&tv, nullptr);
        return (tv.tv_sec - startup.tv_sec) * 1000 + (tv.tv_usec - startup.tv_usec) / 1000;
    }
}

static void print_ts(void)
{
    uint32_t systime = nos_gettimestamp();
    printf("%06d.%03d: ", systime / 1000, systime % 1000);
}


// test 1: yeild
async_function(test_yeild)
{
    struct _local {
        int i;
    }*  local = (struct _local*)async_localdata(sizeof(struct _local));
    int max   = async_getarg(int);

    async_function_start();
    local->i = 0;
    while (true) {
        local->i += 1;

        printf("%s.%s -> %d\n", async_taskname(), __func__, local->i);

        if (local->i >= max) {
            printf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();
        } else {
            async_yeild();
        }
    }
    async_function_end();
}


// test 2: async_call
async_function(test_acall_sub)
{
    struct _local {
        int x;
        int y;
    }* local = (struct _local*)async_localdata(sizeof(struct _local));

    async_function_start();
    local->x = 0;
    local->y = 0;
    while (true) {
        local->x += 1;
        local->y += 2;
        printf("%s.%s -> (%d,%d)\n", async_taskname(), __func__, local->x, local->y);

        if (local->x == 5) {
            printf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();
        } else {
            async_yeild();
        }
    }
    async_function_end();
}

async_function(test_acall_base)
{
    struct _local {
        int i;
    }* local = (struct _local*)async_localdata(sizeof(struct _local));

    async_function_start();
    local->i = 0;
    while (true) {
        local->i += 1;
        printf("%s.%s 1 -> %d\n", async_taskname(), __func__, local->i);
        async_yeild();

        printf("%s.%s async_call\n", async_taskname(), __func__);
        async_call(test_acall_sub, nullptr);

        local->i += 2;
        printf("%s.%s 2 -> %d\n", async_taskname(), __func__, local->i);

        if (local->i >= 5) {
            printf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();
        } else {
            async_yeild();
        }
    }
    async_function_end();
}


// test 3: sleep
async_function(test_sleep)
{
    struct _local {
        int i;
    }*  local     = (struct _local*)async_localdata(sizeof(struct _local));
    int period_ms = async_getarg(int);

    async_function_start();
    local->i = 0;
    while (true) {
        local->i += 1;

        print_ts(), printf("%s.%s -> %d\n", async_taskname(), __func__, local->i);
        async_sleep(period_ms);

        if (local->i >= 10) {
            print_ts(), printf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();
        }
    }
    async_function_end();
}


// test 4: timer
static void on_timer(void* arg)
{
    const char* name = (const char*)arg;
    print_ts(), printf("on_timer %s\n", name);
}


// test 5: mutex
nos_mutex_t mutex;
async_function(test_mutex)
{
    struct _local {
        int i;
        int n;
    }* local = (struct _local*)async_localdata(sizeof(struct _local));

    async_function_start();
    local->i = 0;
    while (true) {
        nos_mutex_lock(&mutex);
        print_ts(), printf("%s lock\n", async_taskname());
        for (local->n = 0; local->n < 3; local->n++) {
            local->i++;
            print_ts(), printf("%s.%s : (%d,%d)\n", async_taskname(), __func__, local->n, local->i);
            async_sleep(500);
        }
        print_ts(), printf("%s unlock\n\n", async_taskname());
        nos_mutex_unlock(&mutex);
    }
    async_function_end();
}


// test 6: cond, producer & consumer
nos_cond_t cond;
int        fifo_data[4], fifo_rd = 0, fifo_wr = 0;
#define fifo_wrap(idx) idx = (idx == 4) ? 0 : idx
#define fifo_get(pv)   *(pv) = fifo_data[fifo_rd++], fifo_wrap(fifo_rd)
#define fifo_put(v)    fifo_data[fifo_wr++] = v, fifo_wrap(fifo_wr)

async_function(test_cond_producer)
{
    struct _local {
        int i;
    }* local = (struct _local*)async_localdata(sizeof(struct _local));

    async_function_start();
    local->i = 0;
    while (true) {
        async_sleep(500);
        local->i++;
        print_ts(), printf("%s.%s set data %d\n", async_taskname(), __func__, local->i);
        fifo_put(local->i);
        nos_cond_signal(&cond);

        if (local->i >= 0xf) {
            print_ts(), printf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();
        }
    }
    async_function_end();
}

async_function(test_cond_consumer)
{
    int data;
    async_function_start();
    while (true) {
        nos_cond_wait(&cond);
        fifo_get(&data);
        print_ts(), printf("%s.%s get data %d\n", async_taskname(), __func__, data);
        if (data >= 0xf) {
            print_ts(), printf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();
        }
    }
    async_function_end();
}


// test 7: cond, external singal
void test_cond_producer_external(void* arg)
{
    static int i = 0;
    i++;
    print_ts(), printf("%s set data %d\n", __func__, i);
    fifo_put(i);
    nos_cond_extsignal(&cond);
}


int main(int argc, char* argv[])
{
    int tc = argc > 1 ? argv[1][0] - '0' : 0;

    // test 1: yeild
    if (tc == 1) {
        nos_task_create(task1, test_yeild, 10);
        nos_task_create(task2, test_yeild, 20);
    }

    // test 2: async_call
    if (tc == 2) {
        nos_task_create(task, test_acall_base, nullptr);
    }

    // test 3: sleep
    if (tc == 3) {
        int period_ms = 500;
        nos_task_create(task, test_sleep, period_ms);
    }

    // test 4: timer
    if (tc == 4) {
        nos_timer_start(on_timer, "Timer A", 500, 10, nullptr);
        nos_timer_start(on_timer, "Timer B", 1000, 10, nullptr);
    }

    // test 5: mutex
    if (tc == 5) {
        nos_mutex_init(&mutex);
        nos_task_create(task1, test_mutex, nullptr);
        nos_task_create(task2, test_mutex, nullptr);
        nos_task_create(task3, test_mutex, nullptr);
    }

    // test 6: cond, producer & consumer
    if (tc == 6) {
        nos_cond_init(&cond);
        nos_task_create(task1, test_cond_producer, nullptr);
        nos_task_create(task2, test_cond_consumer, nullptr);
    }

    // test 7: cond, external singal
    if (tc == 7) {
        nos_cond_init(&cond);
        nos_task_create(task1, test_cond_consumer, nullptr);
        nos_task_create(task2, test_cond_consumer, nullptr);
        nos_timer_start(test_cond_producer_external, nullptr, 500, 20, nullptr);
    }

    while (true) {
        if (!nos_schedule()) {
            printf("no task yet.\n");
            break;
        }
    }
    printf("bye.\n");
}
