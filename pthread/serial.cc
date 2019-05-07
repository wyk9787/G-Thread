#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "../src/log.h"

#define THREAD_NUM 200

#define A_SIZE 10000
#define B_SIZE 15000

typedef struct blob {
  int a[A_SIZE];
  int b[B_SIZE];
  size_t c;
} blob_t;

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

int main() {
  blob_t *b = (blob_t *)malloc(sizeof(blob_t));
  memset(b, 0, sizeof(blob_t));

  for (int t = 0; t < THREAD_NUM; t++) {
    for (int i = 0; i < A_SIZE; i++) {
      b->a[i] = b->a[i] + 1;
    }
    for (int i = 0; i < B_SIZE; i++) {
      b->b[i] = b->b[i] + 1;
    }
    b->c = b->c + 1;
  }

  verify(b);
  std::cout << "c = " << b->c << std::endl;
}
