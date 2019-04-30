#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../src/log.h"

#define MAGIC 117
#define NUM_ITERATIONS 2

// xcontext c;

void *stack_base;
jmp_buf env;
void *stack_so_far;
void *stack_backup;
size_t stack_size;
void *local_test;

int global = 123;

void *GetSP();

void print_all(int a, char b) {
  printf("----------------\n");
  printf("a = %d\n", a);
  printf("b = %c\n", b);
  printf("global = %d\n", global);
  printf("----------------\n");
}

__attribute__((noinline)) void stack_cp(void *des, void *src, size_t len) {
  printf("Len = %zu\n", len);
  char *char_des = (char *)des;
  char *char_src = (char *)src;
  for (size_t i = 0; i < len; i++) {
    *(char_des++) = *(char_src++);
  }
}

int test() {
  int tmp;
  // stack_so_far = GetSP();
  stack_so_far = &tmp;
  // printf("stack_so_far: %p\n", stack_so_far);
  stack_size = (uintptr_t)stack_base - (uintptr_t)stack_so_far;
  printf("stack size: %zu\n", stack_size);
  stack_backup =
      mmap(NULL, stack_size / 4096 * 4096 + 4096, PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (stack_backup == MAP_FAILED) {
    perror("mmap");
    exit(2);
  }

  stack_cp(stack_backup, stack_so_far, stack_size);
  if (setjmp(env)) {
    return MAGIC;
  } else {
    return 0;
  }
}

void func() {
  int a = 1;
  char b = 'a';
  int count = 0;
  // c.commit();
  print_all(a, b);
  while (count < NUM_ITERATIONS) {
    a++;
    b++;
    global *= 2;
    count++;
    print_all(a, b);
    return;
  }
}

// void roll_back() { c.abort(); }

int main() {
  int a = 1;
  int b = 2;
  // print_all(a, b);
  a++;
  b++;
  // printf("a: %d\n", a);
  print_all(a, b);
  // func();
  // roll_back();
}

__attribute__((noinline)) void *GetSP() { return __builtin_frame_address(0); }

//__attribute__((constructor)) void init() {
// int tmp;
// stack_base = &tmp;
// printf("stack_base: %p\n", stack_base);
//}
