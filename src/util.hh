#ifndef UTIL_HH_
#define UTIL_HH_

#include <unistd.h>
#include <iostream>

#include "log.h"

// Round a value x up to the next multiple of y
#define ROUND_UP(x, y) ((x) % (y) == 0 ? (x) : (x) + ((y) - (x) % (y)))

// Round a value x down to the next multiple of y
#define ROUND_DOWN(x, y) ((x) % (y) == 0 ? (x) : (x) - ((x) % (y)))

#define MAX_STACK_SIZE (PAGE_SIZE * 10)
#define HEAP_SIZE 512 * 1024
#define SUBHEAP_SIZE (HEAP_SIZE / NUM_SUBHEAP)
#define NUM_SUBHEAP 1024
#define GLOBAL_SIZE 512 * 1024 * 1024
#define PAGE_SIZE 0x1000
#define SET_JPM_MAGIC 117
#define MIN_ALIGNED_SIZE 16

#define NDEBUG

inline std::ostream &ProcessChooseColor();

// Color output

#if defined(NDEBUG)

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define END "\033[0m" << std::endl

#define ColorLog ProcessChooseColor() << getpid() << ": "

#else

#define ColorLog
#define END

#endif

inline std::ostream &ProcessChooseColor() {
  pid_t pid = getpid();
  int color = pid % 6;
  switch (color) {
    case 0:
      return std::cerr << RED;
      break;
    case 1:
      return std::cerr << GREEN;
      break;
    case 2:
      return std::cerr << YELLOW;
      break;
    case 3:
      return std::cerr << BLUE;
      break;
    case 4:
      return std::cerr << MAGENTA;
      break;
    case 5:
      return std::cerr << CYAN;
      break;
  }
  return std::cerr;
}

extern int shm_fd;         // File descriptor for shared memory object
extern void *local_heap;   // Local view of the state
extern void *global_heap;  // Latest committed state of the global_heap

void init_heap();  // Initialize both local and global_heap

#endif  // UTIL_HH_
