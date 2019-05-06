#ifndef UTIL_HH_
#define UTIL_HH_

#include <unistd.h>
#include <iostream>
#include <sstream>

#include "log.h"

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// Round a value x down to the next multiple of y
#define ROUND_DOWN(x, y) ((x) % (y) == 0 ? (x) : (x) - ((x) % (y)))

#define MAX_STACK_SIZE (PAGE_SIZE * 20)
#define HEAP_SIZE 512 * 1024
#define SUBHEAP_SIZE (HEAP_SIZE / NUM_SUBHEAP)
#define NUM_SUBHEAP 16
#define GLOBAL_SIZE 512 * 1024 * 1024
#define PAGE_SIZE 0x1000
#define MIN_ALIGNED_SIZE 16

//#define LOGPRINT

extern int shm_fd;         // File descriptor for shared memory object
extern void* local_heap;   // Local view of the state
extern void* global_heap;  // Latest committed state of the global_heap

void GlobalHeapInit();

#endif  // UTIL_HH_
