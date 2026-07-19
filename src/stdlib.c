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

__attribute__((error("stub"))) void _close(int fp) {
  __builtin_trap();
  (void) fp;
}

__attribute__((error("stub"))) void _fstat(int fildes, struct stat* buf) {
  __builtin_trap();
  (void) fildes;
  (void) buf;
}

__attribute__((error("stub"))) pid_t _getpid() {
  __builtin_trap();
}

__attribute__((error("stub"))) int _isatty(int fd) {
  __builtin_trap();
  (void) fd;
}

__attribute__((error("stub"))) int _kill(pid_t pid, int sig) {
  __builtin_trap();
  (void) pid;
  (void) sig;
}

__attribute__((error("stub"))) off_t _lseek(int fd, off_t offset, int whence) {
  __builtin_trap();
  (void) fd;
  (void) offset;
  (void) whence;
}

__attribute__((error("stub"))) ssize_t _read(int fd, void* buf, size_t count) {
  __builtin_trap();
  (void) fd;
  (void) buf;
  (void) count;
}

__attribute__((error("stub"))) ssize_t _write(int fd, void* buf, size_t count) {
  __builtin_trap();
  (void) fd;
  (void) buf;
  (void) count;
}

__attribute__((error("stub"))) void _exit(int status) {
  __builtin_trap();
  (void) status;
}
