#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>

// Override malloc() and free() to use the memory manager from FreeRTOS.
// According to the documentation of libc, we also need to override
// calloc and realloc.
// See https://www.gnu.org/software/libc/manual/html_node/Replacing-malloc.html

void* malloc(size_t size) {
  return pvPortMalloc(size);
}

void* __wrap_malloc(size_t size) {
  return malloc(size);
}

void* __wrap__malloc_r(struct _reent* reent, size_t size) {
  (void) reent;
  return malloc(size);
}

void free(void* ptr) {
  vPortFree(ptr);
}

void __wrap_free(void* ptr) {
  free(ptr);
}

void* calloc(size_t num, size_t size) {
  void *ptr = malloc(num * size);
  if (ptr) {
    memset(ptr, 0, num * size);
  }
  return ptr;
}

void* __wrap_calloc(size_t num, size_t size) {
  return calloc(num, size);
}

void* pvPortRealloc(void* ptr, size_t xWantedSize);

void* realloc(void* ptr, size_t newSize) {
  return pvPortRealloc(ptr, newSize);
}

void* __wrap_realloc(void* ptr, size_t newSize) {
  return realloc(ptr, newSize);
}
