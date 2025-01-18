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

        print_ts(), xprintf("%s.%s -> %d\n", async_taskname(), __func__, local->i);
        async_sleep(period_ms);

        if (local->i >= 10) {
            print_ts(), xprintf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();
        }
    }
    async_function_end();
}


void nos_main()
{
    int period_ms = 500;
    nos_task_create(task, test_sleep, period_ms);

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
000122.121: task.test_sleep -> 1
000122.622: task.test_sleep -> 2
000123.122: task.test_sleep -> 3
000123.622: task.test_sleep -> 4
000124.122: task.test_sleep -> 5
000124.622: task.test_sleep -> 6
000125.122: task.test_sleep -> 7
000125.622: task.test_sleep -> 8
000126.122: task.test_sleep -> 9
000126.622: task.test_sleep -> 10
000127.122: task.test_sleep finish.
no task yet.
bye.
```
