#include <iostream>
#include <vector>

#include "gstm.hh"
#include "gthread.hh"

#define THREAD_NUM 100

void *fn(void *arg) {
  int num = *(int *)arg;
  num++;
  *(int *)arg = num;
  num++;
  *(int *)arg = num;
  num++;
  *(int *)arg = num;

  return nullptr;
}

int main() {
  Gstm::Initialize();
  // GThread *t = GThread::GetInstance();
  GThread threads[THREAD_NUM];
  int *a = (int *)malloc(sizeof(int));
  *a = 0;

  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].Create(fn, a);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].Join();
  }

  printf("a = %d\n", *a);
  // void *ret = t.GetRetVal();
  Gstm::Finalize();
}
