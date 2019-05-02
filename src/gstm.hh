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
  static void Finalize();
  static void WaitExited(pid_t predecessor);
  static bool IsHeapConsistent();
  static void CommitHeap();
  static void SegfaultHandler(int signal, siginfo_t* info, void* ctx);

 private:
  // Handle reads and writes when a segfault happens
  static void HandleReads(void* page);
  static void HandleWrites(void* page);

  static void SetupInterProcessMutex();

  static pthread_mutex_t*
      mutex;  // A cross-process mutex that synchronizes the commit stage
  static private_mapping_t read_set_version;
  static private_mapping_t write_set_version;
  static private_mapping_t local_page_version;

  static share_mapping_t*
      global_page_version;  // A global process-shared page version mapping
};

#endif  // GSTM_HH_
