#include <iostream>
#include <vector>

#include "gstm.hh"
#include "gthread.hh"

#define THREAD_NUM 4

void *fn(void *arg) {
  int num = *(int *)arg;
  num++;
  *(int *)arg = num;
  return (void *)1;
}

int main() {
  Gstm::Initialize();
  GThread *t = GThread::GetInstance();
  int *a = (int *)malloc(sizeof(int));
  *a = 117;
  t->Create(fn, a);
  t->Join();
  printf("a = %d\n", *a);
  void *ret = t->GetRetVal();
  while (1)
    ;
  Gstm::Finalize();
}
