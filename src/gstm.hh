#ifndef GSTM_HH_
#define GSTM_HH_

#include <pthread.h>
#include <signal.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "private_alloc.hh"
#include "share_alloc.hh"

class Gstm {
  friend class GThread;

 public:
  using share_mapping_t =
      std::unordered_map<void*, size_t, std::hash<void*>, std::equal_to<void*>,
                         ShareAllocator<std::pair<void*, size_t>>>;
  using private_mapping_t =
      std::unordered_map<void*, size_t, std::hash<void*>, std::equal_to<void*>,
                         PrivateAllocator<std::pair<void*, size_t>>>;

  static void Initialize();
  static void WaitExited(pid_t predecessor);
  static bool IsHeapConsistent();
  static void CommitHeap();
  static void SegfaultHandler(int signal, siginfo_t* info, void* ctx);

 private:
  // Handle reads and writes when a segfault happens
  static void HandleReads(void* page);
  static void HandleWrites(void* page);

  static void* GetSP();

  static void* stack_top;  // Top of the program's stack
  static void*
      stack_backup;  // backup memory for the stack we want to roll back to
  static void* cur_stack;  // the location on the stack we want to store to
  static size_t cur_stack_size;  // The current stack size
  static pthread_mutex_t*
      mutex;  // A cross-process mutex that synchronizes the commit stage
  static private_mapping_t read_set_version;
  static private_mapping_t write_set_version;
  static private_mapping_t local_page_version;

  static share_mapping_t*
      global_page_version;  // A global process-shared page version mapping
};

#endif  // GSTM_HH_
