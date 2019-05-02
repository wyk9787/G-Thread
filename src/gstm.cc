#include "gstm.hh"

#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <memory>

#include "color_log.hh"
#include "log.h"
#include "util.hh"

// Initialize static members of Gstm
pthread_mutex_t* Gstm::mutex = nullptr;
Gstm::private_mapping_t Gstm::read_set_version;
Gstm::private_mapping_t Gstm::write_set_version;
Gstm::private_mapping_t Gstm::local_page_version;
Gstm::share_mapping_t* Gstm::global_page_version = nullptr;

void Gstm::Initialize() {
  GlobalHeapInit();

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
  global_page_version = new (buffer) share_mapping_t();
}

void Gstm::HandleReads(void* page) {
  size_t version_num = 0;
  if (local_page_version.find(page) != local_page_version.end()) {
    version_num = local_page_version[page];
  }
  ColorLog("<read>\t\t" << page << " version: " << version_num);

  read_set_version[page] = version_num;
  mprotect(page, PAGE_SIZE, PROT_READ);
}

void Gstm::HandleWrites(void* page) {
  size_t version_num = 1;
  if (local_page_version.find(page) != local_page_version.end()) {
    version_num = local_page_version[page] + 1;
  }

  ColorLog("<write>\t\t" << page << " version: " << version_num);

  write_set_version[page] = version_num;

  // Unmap the original mapping
  REQUIRE(munmap(page, PAGE_SIZE) == 0) << "munmap failed: " << strerror(errno);

  // Create a new mmaping to host the new data
  void* ret = mmap(page, PAGE_SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  REQUIRE(ret != MAP_FAILED) << "mmap failed: " << strerror(errno);
}

void Gstm::SegfaultHandler(int signal, siginfo_t* info, void* ctx) {
  void* addr = info->si_addr;
  void* page = (void*)ROUND_DOWN((uintptr_t)addr, PAGE_SIZE);

  // If the memory does not come from bump allocator
  if ((uintptr_t)page < (uintptr_t)local_heap ||
      (uintptr_t)page > ((uintptr_t)local_heap) + HEAP_SIZE) {
    ColorLog("REAL segfault at " << page);
    exit(1);
  }

  if (read_set_version.find(page) != read_set_version.end()) {
    // If this is the second time this page enters the segfualt handler, it has
    // to be a write
    HandleWrites(page);
    return;
  } else {
    // If this is the first time that accesses this page, we consider it as a
    // read
    HandleReads(page);
    return;
  }

  if (write_set_version.find(page) != write_set_version.end()) {
    // If this is the third time that triggers the fault, this is an actual
    // segfault
    ColorLog("REAL segfault at " << page);
    exit(1);
  }
}

void Gstm::WaitExited(pid_t predecessor) {
  // This is the first process. There is no predecessor
  if (predecessor == 0) {
    return;
  }
  int status;
  REQUIRE(waitpid(predecessor, &status, 0) == predecessor)
      << "waitpid failed: " << strerror(errno);
}

bool Gstm::IsHeapConsistent() {
  // Check for read set:
  //   If any page in the read set that is also in the global page set does not
  //   match the version number with the one in the global page set, then we
  //   have to rollback
  for (const auto& p : read_set_version) {
    if (global_page_version->find(p.first) != global_page_version->end() &&
        global_page_version->at(p.first) != p.second) {
      ColorLog("<com.F>\t\tread "
               << p.first << " local version:" << p.second
               << ", global version: " << global_page_version->at(p.first));
      return false;
    }
  }

  return true;
}

void Gstm::CommitHeap() {
  for (const auto& p : write_set_version) {
    // Since the local_heap and the
    size_t offset = reinterpret_cast<uintptr_t>(p.first) -
                    reinterpret_cast<uintptr_t>(local_heap);
    void* global_pos = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(global_heap) + offset);
    memcpy(global_pos, p.first, PAGE_SIZE);
    ColorLog("<copy>\t\t" << p.first << " to " << global_pos);

    // Update the page version number
    (*global_page_version)[p.first] = p.second;
    ColorLog("<commit>\t\t" << p.first << " version: " << p.second);
  }
}

void Gstm::Finalize() {
  REQUIRE(munmap(local_heap, HEAP_SIZE) == 0)
      << "munmap failed: " << strerror(errno);
  REQUIRE(munmap(global_heap, HEAP_SIZE) == 0)
      << "munmap failed: " << strerror(errno);
}
