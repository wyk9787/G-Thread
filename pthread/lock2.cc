#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <iostream>

#define THREAD_NUM 10

#define A_SIZE 30
#define B_SIZE 50

#define DOUBLE

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Forward declaration
void *fn1(void *);
void *fn2(void *);

typedef struct blob {
  int a[A_SIZE];
  int b[B_SIZE];
  size_t c;
} blob_t;

void *fn2(void *arg) {
  blob_t *b = (blob_t *)arg;
  for (int i = 0; i < A_SIZE; i++) {
    b->a[i] = b->a[i] + 1;
  }
  for (int i = 0; i < B_SIZE; i++) {
    b->b[i] = b->b[i] + 1;
  }
  b->c = b->c + 1;

  return nullptr;
}

void *fn1(void *arg) {
  blob_t *b = (blob_t *)arg;
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < A_SIZE; i++) {
    b->a[i] = b->a[i] + 1;
  }
  for (int i = 0; i < B_SIZE; i++) {
    b->b[i] = b->b[i] + 1;
  }
  b->c = b->c + 1;

#if defined(DOUBLE)
  pthread_t threads[THREAD_NUM];
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_create(&threads[i], NULL, fn2, b);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }
#endif
  pthread_mutex_unlock(&mutex);

  return nullptr;
}

void print_blob(blob_t *b) {
  printf("a = ");
  for (int i = 0; i < A_SIZE; i++) {
    printf(" %d", b->a[i]);
  }
  printf("\n");

  printf("b = ");
  for (int i = 0; i < B_SIZE; i++) {
    printf(" %d", b->b[i]);
  }
  printf("\n");
}

int main() {
  pthread_t threads[THREAD_NUM];

  blob_t *b = (blob_t *)malloc(sizeof(blob_t));
  memset(b, 0, sizeof(blob_t));

  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_create(&threads[i], NULL, fn1, b);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }

  print_blob(b);

  std::cout << "c = " << b->c << std::endl;
}
