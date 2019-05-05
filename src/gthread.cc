#include "gthread.hh"

#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <sstream>

#include "color_log.hh"
#include "gstm.hh"
#include "log.h"
#include "util.hh"

GThread::GThread() : tid_(getpid()), predecessor_(0), first_gthread_(false) {}

GThread::GThread(bool first)
    : tid_(getpid()), predecessor_(0), first_gthread_(first) {}

void GThread::Create(void *(*start_routine)(void *), void *args) {
  AtomicEnd();

  int child_pid = fork();
  if (child_pid < 0) {
    ColorLog("FORK FAILED!!! QUITING ...");
    exit(1);
  }

  if (child_pid > 0) {
    // Parent process
    predecessor_ = child_pid;
    if (!first_gthread_) {
      ColorLog("<fork>\t\tchild pid: " << child_pid);
    }
    AtomicBegin();
    return;
  } else {
    // Child process

    tid_ = getpid();

    AtomicBegin();
    // Execute thread function
    retval_ = start_routine(args);
    AtomicEnd();

    exit(0);

    return;
  }
}

void GThread::AtomicBegin() {
  if (!first_gthread_) {
    ColorLog("<a.beg>");
  }

  // Save the context
  context_.SaveContext();

  // Clear the local version mappings
  Gstm::read_set_version->clear();
  Gstm::write_set_version->clear();
  Gstm::local_page_version->clear();

  pthread_mutex_lock(Gstm::mutex);
  for (const auto &p : *Gstm::global_page_version) {
    Gstm::local_page_version->insert(p);
  }
  pthread_mutex_unlock(Gstm::mutex);

  // Turn off all permission on the local heap
  REQUIRE(mprotect(local_heap, HEAP_SIZE, PROT_NONE) == 0)
      << "mprotect failed: " << strerror(errno);

  return;
}

void GThread::AtomicEnd() {
  if (!first_gthread_) {
    ColorLog("<a.end>");
  }
  if (!AtomicCommit()) {
    AtomicAbort();
  }
}

bool GThread::AtomicCommit() {
  // If we haven't read or written anything
  // we don't have to wait or commitUpdate local view of memory and return
  // true
  if (Gstm::read_set_version->empty() && Gstm::write_set_version->empty()) {
    if (!first_gthread_) {
      ColorLog("<com.S>\t\tNo read & write");
    }
    Gstm::UpdateHeap();
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
    ColorLog("<com.S>\t\tconsistent heap");
    commited = true;
  }
  if (commited == false) {
    *Gstm::rollback_count_ = *Gstm::rollback_count_ + 1;
  }
  pthread_mutex_unlock(Gstm::mutex);

  return commited;
}

void GThread::AtomicAbort() {
  ColorLog("<ROLLLLLLLLLL BACK!>");
  // Throw away changes
  REQUIRE(madvise(local_heap, HEAP_SIZE, MADV_DONTNEED) == 0)
      << "madvise failed: " << strerror(errno);
  context_.RestoreContext();
}

void GThread::Join() {
  AtomicEnd();
  Gstm::WaitExited(predecessor_);
  AtomicBegin();
}
