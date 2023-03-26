#include <stdlib.h>
#include <FreeRTOS.h>

// Override malloc() and free() to use the memory manager from FreeRTOS.
// According to the documentation of libc, we also need to override
// calloc and realloc.
// See https://www.gnu.org/software/libc/manual/html_node/Replacing-malloc.html

void* malloc(size_t size) {
  return pvPortMalloc(size);
}

void free(void* ptr) {
  vPortFree(ptr);
}

void* calloc(size_t num, size_t size) {
  (void)(num);
  (void)(size);
  // Not supported
  return NULL;
}

void *pvPortRealloc(void *ptr, size_t xWantedSize);
void* realloc( void *ptr, size_t newSize) {
  return pvPortRealloc(ptr, newSize);
}
