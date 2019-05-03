#include <pthread.h>
#include <iostream>
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

  pthread_t threads[THREAD_NUM];
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_create(&threads[i], NULL, fn2, arg);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }
  return nullptr;
}

int main() {
  pthread_t threads[THREAD_NUM];
  int *a = (int *)malloc(sizeof(int));
  *a = 0;

  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_create(&threads[i], NULL, fn1, a);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("a = %d\n", *a);
}
