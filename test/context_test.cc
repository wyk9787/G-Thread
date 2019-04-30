#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../src/stack_context.hh"

int global = 0;

void print_all(int a, int b) {
  printf("----------------\n");
  printf("a = %d\n", a);
  printf("b = %d\n", b);
  printf("----------------\n");
}

void func() {
  int a = 1;
  char b = 'a';
  int count = 0;
  // c.commit();
  print_all(a, b);
  a++;
  b++;
  count++;
  print_all(a, b);
  return;
}

// void roll_back() { c.abort(); }

int main() {
  StackContext s;
  int a = 1;
  int b = 2;
  s.SaveContext();
  print_all(a, b);
  a++;
  b++;
  global++;
  printf("global = %d\n", global);
  print_all(a, b);
  if (global != 3) {
    s.RestoreContext();
  }
  printf("Done\n");
  // func();
  // roll_back();
}
