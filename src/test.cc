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
  // GThread *t = GThread::GetInstance();
  GThread t1, t2;
  int *a = (int *)malloc(sizeof(int));
  *a = 117;
  t1.Create(fn, a);
  t2.Create(fn, a);
  t1.Join();
  t2.Join();
  printf("a = %d\n", *a);
  // void *ret = t.GetRetVal();
  Gstm::Finalize();
}
