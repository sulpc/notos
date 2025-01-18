# mutex test

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
        print_ts(), xprintf("%s lock\n", async_taskname());
        for (local->n = 0; local->n < 3; local->n++) {
            local->i++;
            print_ts(), xprintf("%s.%s : (%d,%d)\n", async_taskname(), __func__, local->n, local->i);
            async_sleep(500);
        }
        print_ts(), xprintf("%s unlock\n\n", async_taskname());
        nos_mutex_unlock(&mutex);
    }
    async_function_end();
}


void nos_main()
{
    nos_mutex_init(&mutex);
    nos_task_create(task1, test_mutex, nullptr);
    nos_task_create(task2, test_mutex, nullptr);
    nos_task_create(task3, test_mutex, nullptr);

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
debug nos 5

000508.376: task1 lock
000508.376: task1.test_mutex : (0,1)
000508.876: task1.test_mutex : (1,2)
000509.376: task1.test_mutex : (2,3)
000509.876: task1 unlock

000509.876: task2 lock
000509.876: task2.test_mutex : (0,1)
000510.376: task2.test_mutex : (1,2)
000510.876: task2.test_mutex : (2,3)
000511.376: task2 unlock

000511.376: task3 lock
000511.376: task3.test_mutex : (0,1)
000511.876: task3.test_mutex : (1,2)
000512.376: task3.test_mutex : (2,3)
000512.876: task3 unlock

000512.876: task1 lock
000512.876: task1.test_mutex : (0,4)
000513.376: task1.test_mutex : (1,5)
000513.876: task1.test_mutex : (2,6)
000514.376: task1 unlock

000514.376: task2 lock
000514.376: task2.test_mutex : (0,4)
000514.876: task2.test_mutex : (1,5)
000515.376: task2.test_mutex : (2,6)
000515.876: task2 unlock

000515.876: task3 lock
000515.876: task3.test_mutex : (0,4)
000516.376: task3.test_mutex : (1,5)
000516.876: task3.test_mutex : (2,6)
000517.376: task3 unlock

000517.376: task1 lock
000517.376: task1.test_mutex : (0,7)
000517.876: task1.test_mutex : (1,8)
000518.376: task1.test_mutex : (2,9)
000518.876: task1 unlock

000518.876: task2 lock
000518.876: task2.test_mutex : (0,7)
000519.376: task2.test_mutex : (1,8)
000519.876: task2.test_mutex : (2,9)
000520.376: task2 unlock

000520.376: task3 lock
000520.376: task3.test_mutex : (0,7)
000520.876: task3.test_mutex : (1,8)
000521.376: task3.test_mutex : (2,9)
000521.876: task3 unlock

000521.876: task1 lock
000521.876: task1.test_mutex : (0,10)
000522.376: task1.test_mutex : (1,11)
000522.876: task1.test_mutex : (2,12)
000523.376: task1 unlock

000523.376: task2 lock
000523.376: task2.test_mutex : (0,10)
000523.876: task2.test_mutex : (1,11)
000524.376: task2.test_mutex : (2,12)
000524.876: task2 unlock

000524.876: task3 lock
000524.876: task3.test_mutex : (0,10)
000525.376: task3.test_mutex : (1,11)
000525.876: task3.test_mutex : (2,12)
000526.376: task3 unlock

000526.376: task1 lock
000526.376: task1.test_mutex : (0,13)
000526.876: task1.test_mutex : (1,14)
000527.376: task1.test_mutex : (2,15)
000527.876: task1 unlock

000527.876: task2 lock
000527.876: task2.test_mutex : (0,13)
000528.376: task2.test_mutex : (1,14)
000528.876: task2.test_mutex : (2,15)
000529.376: task2 unlock

000529.376: task3 lock
000529.376: task3.test_mutex : (0,13)
000529.876: task3.test_mutex : (1,14)
000530.376: task3.test_mutex : (2,15)
000530.876: task3 unlock
```
