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
        xprintf("%s.%s -> (%d,%d)\n", async_taskname(), __func__, local->x, local->y);

        if (local->x == 5) {
            xprintf("%s.%s finish.\n", async_taskname(), __func__);
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
        xprintf("%s.%s 1 -> %d\n", async_taskname(), __func__, local->i);
        async_yeild();

        xprintf("%s.%s async_call\n", async_taskname(), __func__);
        async_call(test_acall_sub, nullptr);

        local->i += 2;
        xprintf("%s.%s 2 -> %d\n", async_taskname(), __func__, local->i);

        if (local->i >= 5) {
            xprintf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();
        } else {
            async_yeild();
        }
    }
    async_function_end();
}

void nos_main()
{
    nos_task_create(task, test_acall_base, nullptr);

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
task.test_acall_base 1 -> 1
task.test_acall_base async_call
task.test_acall_sub -> (1,2)
task.test_acall_sub -> (2,4)
task.test_acall_sub -> (3,6)
task.test_acall_sub -> (4,8)
task.test_acall_sub -> (5,10)
task.test_acall_sub finish.
task.test_acall_base 2 -> 3
task.test_acall_base 1 -> 4
task.test_acall_base async_call
task.test_acall_sub -> (1,2)
task.test_acall_sub -> (2,4)
task.test_acall_sub -> (3,6)
task.test_acall_sub -> (4,8)
task.test_acall_sub -> (5,10)
task.test_acall_sub finish.
task.test_acall_base 2 -> 6
task.test_acall_base finish.
no task yet.
bye.
```
