#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <sys/stat.h>
#include <unistd.h>

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
  void* ptr = malloc(num * size);
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

// Implement functions required by libc as stubs
// These functions aren't linked into the final binary

__attribute__((error("stub"))) void _close(__attribute__((unused)) int fp) {
  __builtin_trap();
}

__attribute__((error("stub"))) void _fstat(__attribute__((unused)) int fildes, __attribute__((unused)) struct stat* buf) {
  __builtin_trap();
}

__attribute__((error("stub"))) pid_t _getpid() {
  __builtin_trap();
}

__attribute__((error("stub"))) int _isatty(__attribute__((unused)) int fd) {
  __builtin_trap();
}

__attribute__((error("stub"))) int _kill(__attribute__((unused)) pid_t pid, __attribute__((unused)) int sig) {
  __builtin_trap();
}

__attribute__((error("stub"))) off_t _lseek(__attribute__((unused)) int fd,
                                            __attribute__((unused)) off_t offset,
                                            __attribute__((unused)) int whence) {
  __builtin_trap();
}

__attribute__((error("stub"))) ssize_t _read(__attribute__((unused)) int fd,
                                             __attribute__((unused)) void* buf,
                                             __attribute__((unused)) size_t count) {
  __builtin_trap();
}

__attribute__((error("stub"))) ssize_t _write(__attribute__((unused)) int fd,
                                              __attribute__((unused)) void* buf,
                                              __attribute__((unused)) size_t count) {
  __builtin_trap();
}

__attribute__((error("stub"))) void _exit(__attribute__((unused)) int status) {
  __builtin_trap();
}
