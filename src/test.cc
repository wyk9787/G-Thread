#include <iostream>
#include <vector>

#include "libgthread.hh"

#define THREAD_NUM 300

#define A_SIZE 30
#define B_SIZE 50

// Forward declaration
void *fn1(void *);

typedef struct blob {
  int a[A_SIZE];
  int b[B_SIZE];
  size_t c;
} blob_t;

void *fn1(void *arg) {
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

  printf("a =");
  for (int i = 0; i < A_SIZE; i++) {
    printf(" %d", b->a[i]);
  }
  printf("\n");

  printf("b =");
  for (int i = 0; i < B_SIZE; i++) {
    printf(" %d", b->b[i]);
  }
  printf("\n");
  printf("c = %zu\n", b->c);

  std::cerr << "Rollback count = " << *Gstm::rollback_count_ << std::endl;
}
