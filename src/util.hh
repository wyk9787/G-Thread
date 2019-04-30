#ifndef UTIL_HH_
#define UTIL_HH_

#include <unistd.h>

#include "log.h"

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

// Color output

#define RED_LOG(x, pid) (INFO << "\033[1;31m" << pid << " " << x << "\033[0m\n")
#define GREEN_LOG(x, pid) \
  (INFO << "\033[1;32m" << pid << " " << x << "\033[0m\n")
#define YELLOW_LOG(x, pid) \
  (INFO << "\033[1;33m" << pid << " " << x << "\033[0m\n")
#define BLUE_LOG(x, pid) \
  (INFO << "\033[1;34m" << pid << " " << x << "\033[0m\n")
#define MAGENTA_LOG(x, pid) \
  (INFO << "\033[1;35m" << pid << " " << x << "\033[0m\n")
#define CYAN_LOG(x, pid) \
  (INFO << "\033[1;36m" << pid << " " << x << "\033[0m\n")

#define ColorLog(x)                    \
  ({                                   \
    pid_t pid = getpid();              \
    int color = pid % 6;               \
    printf("color code: %d\n", color); \
    switch (color) {                   \
      case 0:                          \
        RED_LOG(x, pid);               \
        break;                         \
      case 1:                          \
        GREEN_LOG(x, pid);             \
        break;                         \
      case 2:                          \
        YELLOW_LOG(x, pid);            \
        break;                         \
      case 3:                          \
        BLUE_LOG(x, pid);              \
        break;                         \
      case 4:                          \
        MAGENTA_LOG(x, pid);           \
        break;                         \
      case 5:                          \
        CYAN_LOG(x, pid);              \
        break;                         \
    }                                  \
  })

extern int shm_fd;         // File descriptor for shared memory object
extern void *local_heap;   // Local view of the state
extern void *global_heap;  // Latest committed state of the global_heap

#endif  // UTIL_HH_
