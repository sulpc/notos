# cond test

## code
```c
#include "notos.h"

volatile uint32_t nos_timestamp = 0;   // increment by a tick isr
extern void       xprintf(const char* fmt, ...);

uint32_t nos_gettimestamp(void)
{
    return nos_timestamp;
}

static void print_ts(void)
{
    uint32_t systime = nos_gettimestamp();
    xprintf("%06d.%03d: ", systime / 1000, systime % 1000);
}

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
        print_ts(), xprintf("%s.%s set data %d\n", async_taskname(), __func__, local->i);
        fifo_put(local->i);
        nos_cond_signal(&cond);

        if (local->i >= 0xf) {
            print_ts(), xprintf("%s.%s finish.\n", async_taskname(), __func__);
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
        print_ts(), xprintf("%s.%s get data %d\n", async_taskname(), __func__, data);
        if (data >= 0xf) {
            print_ts(), xprintf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();
        }
    }
    async_function_end();
}

void nos_main()
{
    nos_cond_init(&cond);
    nos_task_create(task1, test_cond_producer, nullptr);
    nos_task_create(task2, test_cond_consumer, nullptr);

    while (true) {
        if (!nos_schedule()) {
            xprintf("no task yet.\n");
            break;
        }
    }
    xprintf("bye.\n");
}
```

## output

```
000334.239: task1.test_cond_producer set data 1
000334.239: task2.test_cond_consumer get data 1
000334.739: task1.test_cond_producer set data 2
000334.739: task2.test_cond_consumer get data 2
000335.239: task1.test_cond_producer set data 3
000335.239: task2.test_cond_consumer get data 3
000335.739: task1.test_cond_producer set data 4
000335.739: task2.test_cond_consumer get data 4
000336.239: task1.test_cond_producer set data 5
000336.239: task2.test_cond_consumer get data 5
000336.739: task1.test_cond_producer set data 6
000336.739: task2.test_cond_consumer get data 6
000337.239: task1.test_cond_producer set data 7
000337.239: task2.test_cond_consumer get data 7
000337.739: task1.test_cond_producer set data 8
000337.739: task2.test_cond_consumer get data 8
000338.239: task1.test_cond_producer set data 9
000338.239: task2.test_cond_consumer get data 9
000338.739: task1.test_cond_producer set data 10
000338.739: task2.test_cond_consumer get data 10
000339.239: task1.test_cond_producer set data 11
000339.239: task2.test_cond_consumer get data 11
000339.739: task1.test_cond_producer set data 12
000339.739: task2.test_cond_consumer get data 12
000340.239: task1.test_cond_producer set data 13
000340.239: task2.test_cond_consumer get data 13
000340.739: task1.test_cond_producer set data 14
000340.739: task2.test_cond_consumer get data 14
000341.239: task1.test_cond_producer set data 15
000341.239: task2.test_cond_consumer get data 15
000341.239: task2.test_cond_consumer finish.
000341.239: task1.test_cond_producer finish.
no task yet.
bye.
```
