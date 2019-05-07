#ifndef GSTM_HH_
#define GSTM_HH_

#include <pthread.h>
#include <signal.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "private_alloc.hh"
#include "share_alloc.hh"

// An internal class that provides useful utility functions for GThread to use
class Gstm {
  friend class GThread;

 public:
  Gstm() = delete;

  // An unordered_map that uses a custom allocator for private memory only
  using share_mapping_t =
      std::unordered_map<void*, size_t, std::hash<void*>, std::equal_to<void*>,
                         ShareAllocator<std::pair<void*, size_t>>>;

  // An unordered_map that uses a custom allocator for share memory only
  using private_mapping_t =
      std::unordered_map<void*, size_t, std::hash<void*>, std::equal_to<void*>,
                         PrivateAllocator<std::pair<void*, size_t>>>;

  // Initialize the internal system
  static void Initialize();

  // Wait until predecessor exited
  static void WaitExited(pid_t predecessor);

  // Checks whether local heap is concsistent with global heap
  static bool IsHeapConsistent();

  // Commit local heap view into global heap
  static void CommitHeap();

  // Handle segfaults
  static void SegfaultHandler(int signal, siginfo_t* info, void* ctx);

  // Keep track of how many rollbacks happened for the entire program
  static size_t* rollback_count_;

 private:
  // Handle reads and writes when a segfault happens
  static void HandleReads(void* page);
  static void HandleWrites(void* page);

  // Set up an inter-process mutex
  static void SetupInterProcessMutex();

  // Initialize both local and shared states being tracked by the system
  static void InitMapping();

  // Update the local view of memory to match the global view
  static void UpdateHeap();

  // A cross-process mutex that synchronizes the commit stage
  static pthread_mutex_t* mutex;

  // Reads and writes happened in current process
  static private_mapping_t* read_set_version;
  static private_mapping_t* write_set_version;

  // a local view of each page's version
  static private_mapping_t* local_page_version;

  // A global process-shared view of each page's version
  static share_mapping_t*
      global_page_version;  // A global process-shared page version mapping
};

#endif  // GSTM_HH_
