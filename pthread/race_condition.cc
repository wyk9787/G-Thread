#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define THREAD_NUM 20

typedef struct args {
  int x;
  int y;
} arg_t;

void *fn1(void *arg) {
  arg_t *a = (arg_t *)arg;
  a->x++;
  a->y++;

  return NULL;
}

int main() {
  arg_t *a = (arg_t *)malloc(sizeof(arg_t));
  a->x = 0;
  a->y = 0;

  pthread_t threads[THREAD_NUM];
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_create(&threads[i], NULL, fn1, a);
  }

  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }

  if (a->x != THREAD_NUM || a->y != THREAD_NUM) {
    printf("x = %d\n", a->x);
    printf("y = %d\n", a->y);
  }
  printf("-----------\n");
}
