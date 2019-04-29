#include <pthread.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../src/xcontext.h"

xcontext c;

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
  c.initialize();
  int a = 1;
  int b = 2;
  c.commit();
  // print_all(a, b);
  a++;
  b++;
  // printf("a: %d\n", a);
  print_all(a, b);
  c.abort();
  // func();
  // roll_back();
}
