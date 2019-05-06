#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "../src/log.h"

#define THREAD_NUM 50
#define SECONDARY_THREAD_NUM 50

#define A_SIZE 1000
#define B_SIZE 1500

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
  pthread_mutex_lock(&mutex);
  blob_t *b = (blob_t *)arg;
  for (int i = 0; i < A_SIZE; i++) {
    b->a[i] = b->a[i] + 1;
  }
  for (int i = 0; i < B_SIZE; i++) {
    b->b[i] = b->b[i] + 1;
  }
  b->c = b->c + 1;
  pthread_mutex_unlock(&mutex);

  return nullptr;
}

void *fn1(void *arg) {
  pthread_mutex_lock(&mutex);
  blob_t *b = (blob_t *)arg;
  for (int i = 0; i < A_SIZE; i++) {
    b->a[i] = b->a[i] + 1;
  }
  for (int i = 0; i < B_SIZE; i++) {
    b->b[i] = b->b[i] + 1;
  }
  b->c = b->c + 1;
  pthread_mutex_unlock(&mutex);

#if defined(DOUBLE)
  pthread_t threads[SECONDARY_THREAD_NUM];
  for (int i = 0; i < SECONDARY_THREAD_NUM; i++) {
    pthread_create(&threads[i], NULL, fn2, b);
  }
  for (int i = 0; i < SECONDARY_THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }
#endif

  return nullptr;
}

void verify(blob_t *b) {
#if defined(DOUBLE)
  int expected = THREAD_NUM * (SECONDARY_THREAD_NUM + 1);
#else
  int expected = THREAD_NUM;
#endif
  for (int i = 0; i < A_SIZE; i++) {
    REQUIRE(b->a[i] == expected)
        << "Expect: " << expected << ", b->a[" << i << "]: " << b->a[i];
  }
  for (int i = 0; i < B_SIZE; i++) {
    REQUIRE(b->b[i] == expected)
        << "Expect: " << expected << ", b->b[" << i << "]: " << b->b[i];
  }
  REQUIRE(b->c == (size_t)expected)
      << "Expect: " << expected << ", b->c: " << b->c;
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

  verify(b);
  std::cout << "c = " << b->c << std::endl;
}
