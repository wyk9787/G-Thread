#include <iostream>
#include <vector>

#include "gstm.hh"
#include "gthread.hh"

#define THREAD_NUM 4

void *fn(void *arg) {
  int num = *(int *)arg;
  num++;
  std::cout << "This is test function " << num << std::endl;
  // sleep(5);
  return (void *)1;
}

int main() {
  // int *arr = (int *)malloc(4 * sizeof(int));
  // for (int i = 0; i < 4; i++) {
  // arr[i] = i;
  //}

  // for (int i = 0; i < 4; i++) {
  // printf("arr[i] = %d\n", arr[i]);
  //}

  Gstm::Initialize();
  GThread t;
  int *a = (int *)malloc(sizeof(int));
  t.Create(fn, a);
  t.Join();
  printf("Done!\n");
  void *ret = t.GetRetVal();
  printf("Receive ret: %lu\n", (uintptr_t)ret);
}
