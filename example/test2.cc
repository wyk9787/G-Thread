#include <iostream>
#include <vector>

#include "gstm.hh"
#include "gthread.hh"

#define THREAD_NUM 30

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
    b->a[i]++;
  }
  for (int i = 0; i < B_SIZE; i++) {
    b->b[i]++;
  }
  b->c = b->c + 1;

  return nullptr;
}

int main() {
  GThread threads[THREAD_NUM];

  blob_t *b = malloc(sizeof(blob_t));
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
  printf("c = %d\n", b->c);

  std::cerr << "Rollback count = " << *Gstm::rollback_count_ << std::endl;
}
