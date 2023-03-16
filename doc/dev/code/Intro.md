# Introduction to the code

This page is meant to guide you through the source code, so you can find the relevant files for what you're working on.

## FreeRTOS

Infinitime is based on FreeRTOS, a real-time operating system.
FreeRTOS provides several quality of life abstractions (for example easy software timers)
and most importantly supports multiple tasks.
If you want to read up on real-time operating systems, you can look [here](https://www.freertos.org/implementation/a00002.html) and [here](https://www.freertos.org/features.html).
The main "process" creates at least one task and then starts the FreeRTOS task scheduler.
This main "process" is the standard main() function inside [main.cpp](/src/main.cpp).
The task scheduler is responsible for giving every task enough cpu time.
As there is only one core on the SoC of the PineTime, real concurrency is impossible and the scheduler has to swap tasks in and out to emulate it.

### Tasks

Tasks are created by calling `xTaskCreate` and passing a function with the signature `void functionName(void*)`.
For more info on task creation see the [FreeRTOS Documentation](https://www.freertos.org/a00125.html).
In our case, main calls `systemTask.Start()`, which creates the **"MAIN" task**.
The function running inside that task is `SystemTask::Work()`.
You may also see this task being referred to as the **work task**.
Both functions are located inside [systemtask/SystemTask.cpp](/src/systemtask/SystemTask.cpp). `SystemTask::Work()` initializes all the driver and controller objects.
It also starts the **task "displayapp"**, which is responsible for launching and running apps, controlling the screen and handling touch events (or forwarding them to the active app).
You can find the "displayapp" task inside [displayapp/DisplayApp.cpp](/src/displayapp/DisplayApp.cpp).
There are also other tasks that are responsible for Bluetooth ("ll" and "ble" inside [libs/mynewt-nimble/porting/npl/freertos/src/nimble_port_freertos.c](/src/libs/mynewt-nimble/porting/npl/freertos/src/nimble_port_freertos.c))
and periodic tasks like heartrate measurements ([heartratetask/HeartRateTask.cpp](/src/heartratetask/HeartRateTask.cpp)).

While it is possible for you to create your own task when you need it, it is recommended to just add functionality to `SystemTask::Work()` if possible.
If you absolutely need to create another task, try to estimate how much [stack space](https://www.freertos.org/FAQMem.html#StackSize) (in words/4-byte packets)
it will need instead of just typing in a large-ish number.
You can use `configMINIMAL_STACK_SIZE` which is currently set to 120 words.

## Controllers

Controllers in InfiniTime are singleton objects that can provide access to certain resources to apps.
Some of them interface with drivers, others are the driver for the resource.
The resources provided don't have to be hardware-based.
They are declared in main.cpp and initialized in [systemtask/SystemTask.cpp](/src/systemtask/SystemTask.cpp).
Some controllers can be passed by reference to apps that need access to the resource (for example vibration motor).
They reside in [components/](/src/components/) inside their own subfolder.

## Apps

For more detail see the [Apps page](./Apps.md)

## Bluetooth

Header files with short documentation for the functions are inside [libs/mynewt-nimble/nimble/host/include/host/](/src/libs/mynewt-nimble/nimble/host/include/host/).
