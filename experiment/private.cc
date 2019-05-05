#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../src/color_log.hh"

#define HEAP_SIZE 512 * 1024

void *local_heap;
void *share_heap;

int main() {
  int shm_fd = open("tmp", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    perror("open");
    exit(2);
  }

  if (ftruncate(shm_fd, HEAP_SIZE) != 0) {
    perror("ftruncate");
    exit(2);
  }

  local_heap =
      mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, shm_fd, 0);
  if (local_heap == MAP_FAILED) {
    fputs("mmap failed! Giving up.\n", stderr);
    exit(2);
  }

  share_heap =
      mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (share_heap == MAP_FAILED) {
    fputs("mmap failed! Giving up.\n", stderr);
    exit(2);
  }

  int child_pid = fork();
  if (child_pid < 0) {
    perror("fork");
  }

  if (child_pid) {
    int *a = (int *)local_heap;
    *a = 117;
    ColorLog("Parent: a = " << *a);
    int *b = (int *)share_heap;
    *b = 118;
    ColorLog("Parent: b = " << *b);
    ColorLog("Parent: a = " << *a);
    int status;
    wait(&status);
    ColorLog("Parent: b = " << *b);
    ColorLog("Parent: a = " << *a);
    munmap(local_heap, HEAP_SIZE);
    local_heap = mmap(local_heap, HEAP_SIZE, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_FIXED, shm_fd, 0);
    ColorLog("Parent: b = " << *b);
    ColorLog("Parent: a = " << *a);
  } else {
    sleep(2);
    ColorLog("Child: b = " << *(int *)local_heap);
    ColorLog("Child: a = " << *(int *)share_heap);
  }
}
