# external cond test

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


void test_cond_producer_external(void* arg)
{
    static int i = 0;
    i++;
    print_ts(), xprintf("%s set data %d\n", __func__, i);
    fifo_put(i);
    nos_cond_extsignal(&cond);
}


void nos_main()
{
    nos_cond_init(&cond);
    nos_task_create(task1, test_cond_consumer, nullptr);
    nos_task_create(task2, test_cond_consumer, nullptr);
    nos_timer_start(test_cond_producer_external, nullptr, 500, 20, nullptr);

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
000445.376: test_cond_producer_external set data 1
000445.376: task1.test_cond_consumer get data 1
000445.876: test_cond_producer_external set data 2
000445.876: task2.test_cond_consumer get data 2
000446.376: test_cond_producer_external set data 3
000446.376: task1.test_cond_consumer get data 3
000446.876: test_cond_producer_external set data 4
000446.876: task2.test_cond_consumer get data 4
000447.376: test_cond_producer_external set data 5
000447.376: task1.test_cond_consumer get data 5
000447.876: test_cond_producer_external set data 6
000447.876: task2.test_cond_consumer get data 6
000448.376: test_cond_producer_external set data 7
000448.376: task1.test_cond_consumer get data 7
000448.876: test_cond_producer_external set data 8
000448.876: task2.test_cond_consumer get data 8
000449.376: test_cond_producer_external set data 9
000449.376: task1.test_cond_consumer get data 9
000449.876: test_cond_producer_external set data 10
000449.876: task2.test_cond_consumer get data 10
000450.376: test_cond_producer_external set data 11
000450.376: task1.test_cond_consumer get data 11
000450.876: test_cond_producer_external set data 12
000450.876: task2.test_cond_consumer get data 12
000451.376: test_cond_producer_external set data 13
000451.376: task1.test_cond_consumer get data 13
000451.876: test_cond_producer_external set data 14
000451.876: task2.test_cond_consumer get data 14
000452.376: test_cond_producer_external set data 15
000452.376: task1.test_cond_consumer get data 15
000452.376: task1.test_cond_consumer finish.
000452.876: test_cond_producer_external set data 16
000452.876: task2.test_cond_consumer get data 16
000452.876: task2.test_cond_consumer finish.
000453.376: test_cond_producer_external set data 17
000453.876: test_cond_producer_external set data 18
000454.376: test_cond_producer_external set data 19
000454.876: test_cond_producer_external set data 20
no task yet.
bye.
```
