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

  int child_pid = fork();
  if (child_pid < 0) {
    perror("fork");
  }

  if (child_pid) {
    int *a = (int *)local_heap;
    *a = 117;
    ColorLog("Parent: " << *a);
    int status;
    wait(&status);
    ColorLog("Parent: " << *a);
  } else {
    sleep(2);
    int *a = (int *)local_heap;
    ColorLog("Child: " << *a);
    *a = 118;
  }
}
