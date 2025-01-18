# notos

NotOS is not a real os, it's only a async-function library, based on the stackless coroutine (see [Coroutines in C](https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html) by Simon Tatham), and provide some simple os-like apis.

The nos task could be created like:
```c
int period_ms = 500;
nos_task_create(task4, test_sleep, &period_ms);
```

The nos task is based on a so called **async-function** like:
```c
async_function(test_sleep)
{
    // get local data and argument of the async-function:
    struct _local {
        int i;
    }*  local     = (struct _local*)async_localdata(sizeof(struct _local));
    int period_ms = async_getarg(int);   // get argument

    async_function_start();
    // init the local data:
    local->i = 0;
    while (true) {
        // the main body of the async-function:
        local->i += 1;

        printf("%s.%s -> %d\n", async_taskname(), __func__, local->i);
        async_sleep(500);   // yield or sleep

        if (local->i >= 10) {
            printf("%s.%s finish.\n", async_taskname(), __func__);
            async_return();   // the async-function will return
        }
    }
    async_function_end();
}
```

The implementation of async-function is based on some tricks of C programming language, so some mandatory conventions must be followed:

- All async-function **must** be declared by `async_function()`. The main body of the async-function **must** be contained in a `while (true)` loop between `async_function_start()` and `async_function_end()`.
- **Never** use `switch` statment in async function.
- **Never** use local variables to save state information, use local data instead. Get a local data by `async_localdata()` before `async_function_start()`, and init it between `async_function_start()` and `while (true)`.
- Another async function **can and only can** be called in async function.
- A series of `async_*()` interfaces **can and only can** be called in async function.
- Normal non-blocking function can also be called in asynchronous functions.
- `async_return()` must be called when async-function is finished.
- `nos_sem_extpost()` can be used in `main` function to awake the nos task.

The main function could be like:

```c
int main() {
    nos_task_create(task1, func1, nullptr);
    nos_task_create(task2, func2, nullptr);
    while (nos_schedule()) ;
    printf("bye.\n");
}
```

More detailed descriptions of usage can be found in `main.c`.
