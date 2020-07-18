# Memory analysis
## FreeRTOS heap and task stack
FreeRTOS statically allocate its own heap buffer in a global variable named `ucHeap`. This is an aray of *uint8_t*. Its size is specified by the definition `configTOTAL_HEAP_SIZE` in *FreeRTOSConfig.h*
FreeRTOS uses this buffer to allocate memory for tasks stack and all the RTOS object created during runtime (timers, mutexes,...).

The function `xPortGetFreeHeapSize()` returns the amount of memory available in this *ucHeap* buffer. If this value reaches 0, FreeRTOS runs out of memory.

```
NRF_LOG_INFO("Free heap : %d", xPortGetFreeHeapSize());
```


The function `uxTaskGetSystemState()` fetches some information about the running tasks like its name and the minimum amount of stack space that has remained for the task since the task was created:

```
TaskStatus_t tasksStatus[10]
auto nb = uxTaskGetSystemState(tasksStatus, 10, NULL);
for (int i = 0; i < nb; i++) {
  NRF_LOG_INFO("Task [%s] - %d", tasksStatus[i].pcTaskName, tasksStatus[i].usStackHighWaterMark);
```


## Global heap
Heap is used for **dynamic memory allocation (malloc() / new)**. NRF SDK defaults the heap size to 8KB. The size of the heap can be specified by defining `__HEAP_SIZE=8192` in *src/CMakeLists.txt*:

```
add_definitions(-D__HEAP_SIZE=8192)
``` 

You can trace the dynamic memory allocation by using the flag `--wrap` of the linker. When this flag is enabled, the linker will replace the calls to a specific function by a call to __wrap_the_function(). For example, if you specify `-Wl,-wrap,malloc` in the linker flags, the linker will replace all calls to `void* malloc(size_t)` by calls to `void* __wrap_malloc(size_t)`. This is a function you'll have to define in your code. In this function, you can call `__real_malloc()` to call the actual `malloc()' function.

This technic allows you to wrap all calls to malloc() with you own code.

In *src/CMakeLists.txt*:

```
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        ...
        LINK_FLAGS "-Wl,-wrap,malloc ..."
        ...
        )

```

In *main.cpp*:

```
uint32_t totalMalloc = 0;
extern "C" {
    extern void* __real_malloc(size_t s);
    void *__wrap_malloc(size_t s) {
      totalMalloc += s;
      return __real_malloc(s);
    }
}
```
This function sums all the memory that is allocated during the runtime. You can monitor or log this value. You can also place breakpoints in this function to determine where the dynamic memory allocation occurs in your code.


# Global stack
The stack is used to allocate memory used by functions : **parameters and local variables**. NRF SDK defaults the heap size to 8KB. The size of the heap can be specified by defining `__STACK_SIZE=8192` in *src/CMakeLists.txt*:
                                                                                             
```
add_definitions(-D__STACK_SIZE=8192)
``` 

*NOTE*: FreeRTOS uses its own stack buffer. Thus, the global stack is only used for main() and IRQ handlers. It should be possible to reduce its size to a much lower value.

**NOTE**: [?] How to track the global stack usage? 

#LittleVGL buffer
*TODO*

#NimBLE buffers
*TODO*

#Tools
 - https://github.com/eliotstock/memory : display the memory usage (FLASH/RAM) using the .map file from GCC.