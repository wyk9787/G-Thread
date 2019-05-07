#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define THREAD_NUM 2

typedef struct args {
  int x;
  int y;
} arg_t;

void *fn2(void *arg) {
  arg_t *a = (arg_t *)arg;
  if (a->x == 1) {
    a->y = 0;
  }

  return NULL;
}

void *fn1(void *arg) {
  arg_t *a = (arg_t *)arg;
  if (a->y == 1) {
    a->x = 0;
  }

  return NULL;
}

int main() {
  arg_t *a = (arg_t *)malloc(sizeof(arg_t));
  a->x = 1;
  a->y = 1;

  pthread_t threads[THREAD_NUM];
  pthread_create(&threads[0], NULL, fn1, a);
  pthread_create(&threads[1], NULL, fn2, a);

  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }
  printf("x = %d\n", a->x);
  printf("y = %d\n", a->y);
  printf("-----------\n");
}
