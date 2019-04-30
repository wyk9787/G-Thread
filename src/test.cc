#include <iostream>
#include <vector>

#include "gstm.hh"
#include "gthread.hh"

#define THREAD_NUM 4

int total_times = 0;

void *fn(void *arg) {
  int num = *(int *)arg;
  num++;
  std::cout << "This is test function " << num << std::endl;
  std::cout << "Total times = " << total_times++ << std::endl;
  return (void *)1;
}

int main() {
  Gstm::Initialize();
  GThread t;
  int *a = (int *)malloc(sizeof(int));
  t.Create(fn, a);
  t.Join();
  printf("Done!\n");
  void *ret = t.GetRetVal();
  printf("Receive ret: %lu\n", (uintptr_t)ret);
}
