#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "color_log.hh"
#include "libgthread.hh"

#define THREAD_NUM 50

#define NUM 1000000

void *fn1(void *arg) {
  int a = 0;
  int b = 0;
  for (int i = 0; i < NUM; i++) {
    a += rand() % 100;
    b += rand() % 100000;
  }
  ColorLog("a = " << a);
  ColorLog("b = " << b);

  return nullptr;
}

int main() {
  srand(1);
  pthread_t threads[THREAD_NUM];

  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_create(&threads[i], NULL, fn1, NULL);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }
}
