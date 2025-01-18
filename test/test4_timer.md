# timer test

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


// test 4: timer
static void on_timer(void* arg)
{
    const char* name = (const char*)arg;
    print_ts(), xprintf("on_timer %s\n", name);
}


void nos_main()
{
    nos_timer_start(on_timer, "Timer A", 500, 10, nullptr);
    nos_timer_start(on_timer, "Timer B", 1000, 10, nullptr);

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
000196.551: on_timer Timer A
000197.051: on_timer Timer A
000197.051: on_timer Timer B
000197.551: on_timer Timer A
000198.051: on_timer Timer A
000198.051: on_timer Timer B
000198.551: on_timer Timer A
000199.051: on_timer Timer A
000199.051: on_timer Timer B
000199.551: on_timer Timer A
000200.051: on_timer Timer A
000200.051: on_timer Timer B
000200.551: on_timer Timer A
000201.051: on_timer Timer A
000201.051: on_timer Timer B
000202.051: on_timer Timer B
000203.051: on_timer Timer B
000204.051: on_timer Timer B
000205.051: on_timer Timer B
000206.051: on_timer Timer B
no task yet.
bye.
```
