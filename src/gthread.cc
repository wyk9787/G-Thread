#include "gthread.hh"

#include <pthread.h>
#include <string.h>
#include <sys/mman.h>

#include "gstm.hh"
#include "log.h"
#include "util.hh"

GThread::GThread() : tid_(getpid()), predecessor_(0) { AtomicBegin(); }

void GThread::Create(void *(*start_routine)(void *), void *args) {
  AtomicEnd();

  int child_pid = fork();
  REQUIRE(child_pid >= 0) << "fork failed: " << strerror(errno);

  if (child_pid > 0) {
    // Parent process
    predecessor_ = child_pid;

    AtomicBegin();
    return;
  } else {
    // Child process

    tid_ = getpid();

    AtomicBegin();
    // Execute thread function
    retval_ = start_routine(args);
    AtomicEnd();

    Gstm::Finalize();

    exit(0);

    return;
  }
}

void GThread::AtomicBegin() {
  ColorLog("<atomic begin>");
  // Clear the local version mappings
  Gstm::read_set_version.clear();
  Gstm::write_set_version.clear();
  Gstm::local_page_version.clear();

  // Copy the global version mapping to local
  pthread_mutex_lock(Gstm::mutex);
  for (const auto &p : *Gstm::global_page_version) {
    Gstm::local_page_version.insert(p);
  }
  pthread_mutex_unlock(Gstm::mutex);

  // Turn off all permission on the local heap
  REQUIRE(mprotect(local_heap, HEAP_SIZE, PROT_NONE) == 0)
      << "mprotect failed: " << strerror(errno);

  context_.SaveContext();

  return;
}

void GThread::AtomicEnd() {
  ColorLog("<atomic end>");
  if (!AtomicCommit()) {
    AtomicAbort();
  }
}

bool GThread::AtomicCommit() {
  // If we haven't read or written anything
  // we don't have to wait or commitUpdate local view of memory and return
  // true
  if (Gstm::read_set_version.empty() && Gstm::write_set_version.empty()) {
    // TODO: What do we need to update here?
    // Gstm::UpdateHeap();
    ColorLog("<commit succeeded>\t\tNo read & write");
    return true;
  }

  // Wait for immediate predecessor to complete
  Gstm::WaitExited(predecessor_);

  // Now try to commit state. If and only if we succeed, return true

  // Lock to make sure only one process is commiting at a time
  pthread_mutex_lock(Gstm::mutex);
  bool commited = false;
  if (Gstm::IsHeapConsistent()) {
    Gstm::CommitHeap();
    ColorLog("<commit succeeded>\t\tHeap Consistent");
    commited = true;
  }
  pthread_mutex_unlock(Gstm::mutex);

  return commited;
}

void GThread::AtomicAbort() {
  ColorLog("<rollback>");
  context_.RestoreContext();
}

void GThread::Join() {
  AtomicEnd();
  Gstm::WaitExited(predecessor_);
  AtomicBegin();
}
