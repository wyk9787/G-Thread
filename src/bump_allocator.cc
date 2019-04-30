#include <assert.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "util.hh"

// Define the extern variables
int shm_fd;
void* local_heap;
void* global_heap;

// A list of subheaps that's maintained by each process
void* subheaps[NUM_SUBHEAP];

void init_heap() {
  // Open a memory object to backup the file
  shm_fd = open("shm_object", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("open");
    exit(2);
  }

  if (ftruncate(shm_fd, HEAP_SIZE) != 0) {
    perror("ftruncate");
    exit(2);
  }

  // Allocate memory for the global heap
  local_heap =
      mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, shm_fd, 0);
  if (local_heap == MAP_FAILED) {
    fputs("mmap failed! Giving up.\n", stderr);
    exit(2);
  }

  // Build up subheaps
  void* cur = local_heap;
  for (size_t i = 0; i < NUM_SUBHEAP; i++) {
    subheaps[i] = cur;
    cur = (void*)((uintptr_t)cur + SUBHEAP_SIZE);
  }

  // Initialize global heap and globals mapping with PROT_NONE permission
  global_heap =
      mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  REQUIRE(global_heap != MAP_FAILED) << "mmap failed: " << strerror(errno);
}

/**
 * Allocate space on the heap.
 * \param size  The minimium number of bytes that must be allocated
 * \returns     A pointer to the beginning of the allocated space.
 *              This function may return NULL when an error occurs.
 */
void* xxmalloc(size_t size) {
  char buf[100];
  sprintf(buf, "Malloc %zu bytes\n", size);
  fputs(buf, stderr);

  pid_t pid = getpid();
  size_t index = pid % NUM_SUBHEAP;
  void* ret = subheaps[index];

  size_t alligned_size = ROUND_UP(size, MIN_ALIGNED_SIZE);

  // TODO: Add check to see if the subheap has depleted

  // Advance the subheap pointer
  subheaps[index] = (void*)((uintptr_t)subheaps[index] + alligned_size);

  return ret;
}

/**
 * Free space occupied by a heap object.
 * \param ptr   A pointer somewhere inside the object that is being freed
 */
void xxfree(void* ptr) {
  // TODO: Actually free things

  // Don't free NULL!
  if (ptr == NULL) return;
}

/**
 * Get the available size of an allocated object
 * \param ptr   A pointer somewhere inside the allocated object
 * \returns     The number of bytes available for use in this object
 */
size_t xxmalloc_usable_size(void* ptr) {
  // TODO: Use a bitmap to keep track of stuff
  return 0;
}
