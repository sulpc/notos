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

        xprintf("%s.%s -> %d\n", async_taskname(), __func__, local->i);

        if (local->i >= max) {
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
    nos_task_create(task1, test_yeild, 10);
    nos_task_create(task2, test_yeild, 20);

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
task1.test_yeild -> 1
task2.test_yeild -> 1
task1.test_yeild -> 2
task2.test_yeild -> 2
task1.test_yeild -> 3
task2.test_yeild -> 3
task1.test_yeild -> 4
task2.test_yeild -> 4
task1.test_yeild -> 5
task2.test_yeild -> 5
task1.test_yeild -> 6
task2.test_yeild -> 6
task1.test_yeild -> 7
task2.test_yeild -> 7
task1.test_yeild -> 8
task2.test_yeild -> 8
task1.test_yeild -> 9
task2.test_yeild -> 9
task1.test_yeild -> 10
task1.test_yeild finish.
task2.test_yeild -> 10
task2.test_yeild -> 11
task2.test_yeild -> 12
task2.test_yeild -> 13
task2.test_yeild -> 14
task2.test_yeild -> 15
task2.test_yeild -> 16
task2.test_yeild -> 17
task2.test_yeild -> 18
task2.test_yeild -> 19
task2.test_yeild -> 20
task2.test_yeild finish.
no task yet.
bye.
```
