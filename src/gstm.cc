#include "gstm.hh"

#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <memory>

#include "log.h"
#include "util.hh"

// Initialize static members of Gstm
void* Gstm::stack_top = nullptr;
void* Gstm::stack_backup = nullptr;
void* Gstm::cur_stack = nullptr;
size_t Gstm::cur_stack_size = 0;
pthread_mutex_t* Gstm::mutex = nullptr;
std::unordered_map<void*, size_t> Gstm::read_set_version;
std::unordered_map<void*, size_t> Gstm::write_set_version;
std::unordered_map<void*, size_t> Gstm::local_page_version;
Gstm::page_version_map_t* Gstm::global_page_version = nullptr;

void Gstm::Initialize() {
  // Initialize global heap and globals mapping with PROT_NONE permission
  global_heap =
      mmap(NULL, HEAP_SIZE, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  REQUIRE(global_heap != MAP_FAILED) << "mmap failed: " << strerror(errno);

  // Set up segfault handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = Gstm::SegfaultHandler;
  sa.sa_flags = SA_SIGINFO;
  REQUIRE(sigaction(SIGSEGV, &sa, NULL) == 0)
      << "sigaction failed: " << strerror(errno);

  // Set up inter-process lock

  // This memory has to be accessed by different processes
  mutex = reinterpret_cast<pthread_mutex_t*>(
      mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
           -1, 0));
  REQUIRE(mutex != MAP_FAILED) << "mmap failed: " << strerror(errno);

  // Create a sharing mapping to back the global version map
  void* buffer = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  REQUIRE(buffer != MAP_FAILED) << "mmap failed: " << strerror(errno);
  global_page_version = new (buffer) page_version_map_t();
}

void Gstm::HandleReads(void* page) {
  size_t version_num = 1;
  if (local_page_version.find(page) != local_page_version.end()) {
    version_num = local_page_version[page];
  }
  read_set_version.insert({page, version_num});
  mprotect(page, PAGE_SIZE, PROT_READ);
}

void Gstm::HandleWrites(void* page) {
  size_t version_num = 0;
  if (local_page_version.find(page) != local_page_version.end()) {
    version_num = local_page_version[page];
  }
  write_set_version.insert({page, version_num});

  // Unmap the original mapping
  REQUIRE(munmap(page, PAGE_SIZE) == 0) << "munmap failed: " << strerror(errno);

  // Create a new mmaping to host the new data
  void* ret = mmap(page, PAGE_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  REQUIRE(ret != MAP_FAILED) << "mmap failed: " << strerror(errno);

  mprotect(page, PAGE_SIZE, PROT_READ | PROT_WRITE);
}

void Gstm::SegfaultHandler(int signal, siginfo_t* info, void* ctx) {
  void* addr = info->si_addr;
  void* page = (void*)ROUND_DOWN((uintptr_t)addr, PAGE_SIZE);

  if (read_set_version.find(page) != read_set_version.end()) {
    // If this is the second time this page enters the segfualt handler, it has
    // to be a write
    HandleWrites(page);
  } else {
    // If this is the first time that accesses this page, we consider it as a
    // read
    HandleReads(page);
  }

  if (write_set_version.find(page) != write_set_version.end()) {
    // If this is the third time that triggers the fault, this is an actual
    // segfault
    FATAL << "Segmentation Fault at " << addr;
  }
}

void Gstm::WaitExited(pid_t predecessor) {
  int status;
  REQUIRE(waitpid(predecessor, &status, 0) == predecessor)
      << "waitpid failed: " << strerror(errno);
  REQUIRE(WIFEXITED(status))
      << "waitpid returned with an abnormal exit status: "
      << WEXITSTATUS(status);
}

__attribute__((noinline)) void* Gstm::GetSP() {
  return __builtin_frame_address(0);
}

bool Gstm::IsHeapConsistent() {
  // Check for read set:
  //   If any page in the read set that is also in the global page set does not
  //   match the version number with the one in the global page set, then we
  //   have to rollback
  for (const auto& p : read_set_version) {
    if (global_page_version->find(p.first) != global_page_version->end() &&
        global_page_version->at(p.first) != p.second) {
      return false;
    }
  }

  // TODO: Revisit the logic here
  // Check for write set:
  //   If any page in the write set has also shown in the global page set, then
  //   we have to roll back
  for (const auto& p : write_set_version) {
    if (global_page_version->find(p.first) != global_page_version->end()) {
      return false;
    }
  }

  return true;
}

void Gstm::CommitHeap() {
  for (const auto& p : write_set_version) {
    size_t offset = reinterpret_cast<uintptr_t>(p.first) -
                    reinterpret_cast<uintptr_t>(local_heap);
    void* global_pos = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(global_heap) + offset);
    memcpy(global_pos, p.first, PAGE_SIZE);

    // Update the page version number
    global_page_version->insert(p);
  }
}
