#include <iostream>
#include <vector>

#include "gstm.hh"
#include "gthread.hh"

#define THREAD_NUM 4

void *fn(void *arg) {
  int num = *(int *)arg;
  num++;
  return (void *)1;
}

int main() {
  Gstm::Initialize();
  GThread t;
  int *a = (int *)malloc(sizeof(int));
  *a = 117;
  t.Create(fn, a);
  t.Join();
  // std::cout << "Done" << std::endl;
  void *ret = t.GetRetVal();
  Gstm::Finalize();
}
