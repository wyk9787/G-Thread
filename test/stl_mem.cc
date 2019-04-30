#include <stdlib.h>
#include <memory>
#include <string>
#include <unordered_map>

#include "../src/private_alloc.h"
#include "../src/util.hh"

int main() {
  init_heap();
  int *a = (int *)malloc(48);
  std::unordered_map<void *, size_t, std::hash<void *>, std::equal_to<void *>,
                     GAllocator<std::pair<void *, size_t>>>
      m;
  m.insert({(void *)123321312, 999});
}
