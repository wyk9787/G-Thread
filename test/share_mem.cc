#include <unistd.h>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../src/gallocator.hh"
#include "../src/log.h"

int main() {
  void* alloc_mem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  REQUIRE(alloc_mem != MAP_FAILED) << "mmap failed: " << strerror(errno);

  std::unordered_map<std::string, int, std::hash<std::string>,
                     std::equal_to<std::string>,
                     GAllocator<std::pair<std::string, int>>>* v =
      new (alloc_mem)
          std::unordered_map<std::string, int, std::hash<std::string>,
                             std::equal_to<std::string>,
                             GAllocator<std::pair<std::string, int>>>();

  int child_pid = fork();
  if (child_pid) {
    // parent process
    sleep(2);
    (*v)["parent"] = 1;
    for (const auto& p : *v) {
      INFO << p.first << " " << p.second;
    }
  } else {
    // child process
    (*v)["child"] = 2;
    v->insert({"hello", 1});
    v->insert({"hi", 2});
    v->insert({"haha", 3});
    v->insert({"heihei", 4});
  }
}
