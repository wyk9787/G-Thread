#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <unistd.h>

#include "../src/log.h"

#define MAGIC 117

void *stack_base;
void *stack_so_far;
void *stack_backup;
size_t stack_size;
int global = 123;

void *GetSP();

void print_all(int a, char b) {
  printf("----------------\n");
  printf("a = %d\n", a);
  printf("b = %c\n", b);
  printf("global = %d\n", global);
  printf("----------------\n");
}

void test() {
  int tmp;
  stack_so_far = GetSP();
  register unsigned long sp;
  __asm__ __volatile__("movq %%rsp, %0" : "=r"(sp));
  printf("SP: %p\n", (void *)sp);

  memcpy(stack_so_far, stack_backup, stack_size);
  // stack_so_far = &tmp;
  printf("stack_so_far: %p\n", stack_so_far);
  stack_size = (uintptr_t)stack_base - (uintptr_t)stack_so_far;
  printf("stack size: %zu\n", stack_size);
  stack_backup =
      mmap(NULL, stack_size / 4096 * 4096 + 4096, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (stack_backup == MAP_FAILED) {
    perror("mmap");
    exit(2);
  }

  memcpy(stack_backup, stack_so_far, stack_size);
}

int main() {
  stack_base = (void *)*(uintptr_t *)GetSP();
  printf("stack_base: %p\n", stack_base);
  int a = 1;
  char b = 'a';
  print_all(a, b);
  printf("a: %p\n", &a);
  printf("b: %p\n", &b);

  ucontext_t context;
  getcontext(&context);

  test();

  if (global == 123) {
    a++;
    b++;
    global *= 2;
    print_all(a, b);
    memcpy(stack_so_far, stack_backup, stack_size);
    setcontext(&context);
  } else {
    printf("After rolling back...\n");
    print_all(a, b);
  }
  printf("Done\n");
}

__attribute__((noinline)) void *GetSP() { return __builtin_frame_address(0); }

//__attribute__((constructor)) void init() {
// int tmp;
// stack_base = &tmp;
// printf("stack_base: %p\n", stack_base);
//}
