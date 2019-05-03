#include <assert.h>
#include <iostream>

#include "libgthread.hh"
#include "log.h"

#define THREAD_NUM 50

#define A_SIZE 800
#define B_SIZE 500

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
  for (int i = 0; i < A_SIZE; i++) {
    b->a[i] = b->a[i] + 1;
  }
  for (int i = 0; i < B_SIZE; i++) {
    b->b[i] = b->b[i] + 1;
  }
  b->c = b->c + 1;

  GThread threads[THREAD_NUM];

  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].Create(fn2, b);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].Join();
  }

  return nullptr;
}

void verify(blob_t *b) {
  int expected = THREAD_NUM * THREAD_NUM;
  for (int i = 0; i < A_SIZE; i++) {
    REQUIRE(b->a[i] == expected)
        << "Expect: " << expected << ", b->a[" << i << "]: " << b->a[i];
  }
  for (int i = 0; i < A_SIZE; i++) {
    REQUIRE(b->b[i] == expected)
        << "Expect: " << expected << ", b->b[" << i << "]: " << b->b[i];
  }
  REQUIRE(b->c == (size_t)expected)
      << "Expect: " << expected << ", b->c: " << b->c;
}

int main() {
  GThread threads[THREAD_NUM];

  blob_t *b = (blob_t *)malloc(sizeof(blob_t));
  memset(b, sizeof(blob_t), 0);

  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].Create(fn1, b);
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].Join();
  }

  verify(b);

  std::cerr << "Rollback count = " << *Gstm::rollback_count_ << std::endl;
}
