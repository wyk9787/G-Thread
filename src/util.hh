#ifndef UTIL_HH_
#define UTIL_HH_

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// Round a value x down to the next multiple of y
#define ROUND_DOWN(x, y) ((x) % (y) == 0 ? (x) : (x) - ((x) % (y)))

#define HEAP_SIZE 512 * 1024
#define SUBHEAP_SIZE (HEAP_SIZE / NUM_SUBHEAP)
#define NUM_SUBHEAP 1024
#define GLOBAL_SIZE 512 * 1024 * 1024
#define PAGE_SIZE 0x1000
#define SET_JPM_MAGIC 117
#define MIN_ALIGNED_SIZE 16

extern int shm_fd;         // File descriptor for shared memory object
extern void *local_heap;   // Local view of the state
extern void *global_heap;  // Latest committed state of the global_heap

#endif  // UTIL_HH_
