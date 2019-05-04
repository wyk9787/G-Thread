#include <iostream>
#include <thread>
#include <vector>

#define THREAD_NUM 100

// Forward declaration
void *fn1(void *);
void *fn2(void *);

void *fn2(void *arg) {
  int num = *(int *)arg;
  num++;
  *(int *)arg = num;
  num++;
  *(int *)arg = num;
  num++;
  *(int *)arg = num;
  return nullptr;
}

void *fn1(void *arg) {
  int num = *(int *)arg;
  num++;
  *(int *)arg = num;
  num++;
  *(int *)arg = num;
  num++;
  *(int *)arg = num;

  std::thread *threads[THREAD_NUM];

  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i] = new std::thread(fn2, arg);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i]->join();
  }

  return nullptr;
}

int main() {
  std::thread *threads[THREAD_NUM];
  int *a = (int *)malloc(sizeof(int));
  *a = 0;

  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i] = new std::thread(fn1, a);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i]->join();
  }

  printf("a   = % d\n ", *a);
}
