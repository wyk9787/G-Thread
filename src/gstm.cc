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
pthread_mutex_t* Gstm::mutex = nullptr;
Gstm::private_mapping_t Gstm::read_set_version;
Gstm::private_mapping_t Gstm::write_set_version;
Gstm::private_mapping_t Gstm::local_page_version;
Gstm::share_mapping_t* Gstm::global_page_version = nullptr;

void Gstm::Initialize() {
  init_heap();

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
  // ColorLog("<read>\t\t" +
  // std::to_string(reinterpret_cast<unsigned long long>(page)) +
  //" version=" + std::to_string(version_num));
  ColorLog("<read>");
  char buf[100];
  sprintf(buf, "At %p\n", page);
  fputs(buf, stderr);

  read_set_version[page] = version_num;
  mprotect(page, PAGE_SIZE, PROT_READ);
}

void Gstm::HandleWrites(void* page) {
  size_t version_num = 0;
  if (local_page_version.find(page) != local_page_version.end()) {
    version_num = local_page_version[page];
  }
  // ColorLog("<write>\t\t" +
  // std::to_string(reinterpret_cast<unsigned long long>(page)) +
  //" version=" + std::to_string(version_num));
  ColorLog("<write>");
  char buf[100];
  sprintf(buf, "At %p\n", page);
  fputs(buf, stderr);

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
  INFO << "Entering segfault handler";
  void* addr = info->si_addr;
  void* page = (void*)ROUND_DOWN((uintptr_t)addr, PAGE_SIZE);

  // If the memory does not come from bump allocator
  if ((uintptr_t)page < (uintptr_t)local_heap ||
      (uintptr_t)page > ((uintptr_t)local_heap) + HEAP_SIZE) {
    ColorLog("REAL Segfault at " +
             std::to_string(reinterpret_cast<uintptr_t>(page)));
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
    ColorLog("REAL Segfault at " +
             std::to_string(reinterpret_cast<uintptr_t>(page)));
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
  // REQUIRE(WIFEXITED(status))
  //<< "waitpid returned with an abnormal exit status: "
  //<< WEXITSTATUS(status);
}

bool Gstm::IsHeapConsistent() {
  // Check for read set:
  //   If any page in the read set that is also in the global page set does not
  //   match the version number with the one in the global page set, then we
  //   have to rollback
  for (const auto& p : read_set_version) {
    if (global_page_version->find(p.first) != global_page_version->end() &&
        global_page_version->at(p.first) != p.second) {
      ColorLog("<commit failed>\t\tread " +
               *static_cast<std::string*>(p.first) + " " +
               std::to_string(p.second));
      return false;
    }
  }

  // TODO: Revisit the logic here
  // Check for write set:
  //   If any page in the write set has also shown in the global page set, then
  //   we have to roll back
  for (const auto& p : write_set_version) {
    if (global_page_version->find(p.first) != global_page_version->end()) {
      ColorLog("<commit failed>\t\twrite " +
               *static_cast<std::string*>(p.first) + " " +
               std::to_string(p.second));
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

    // Update the page version number
    global_page_version->insert(p);
  }
}

void Gstm::Finalize() {
  REQUIRE(munmap(local_heap, HEAP_SIZE) == 0)
      << "munmap failed: " << strerror(errno);
}
