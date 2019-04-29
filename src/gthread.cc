#include "gthread.hh"

#include <pthread.h>
#include <string.h>
#include <sys/mman.h>

#include "gstm.hh"
#include "log.h"
#include "util.hh"

GThread::GThread() : tid_(getpid()) { AtomicBegin(); }

void GThread::Create(void *(*start_routine)(void *), void *args) {
  AtomicEnd();

  int child_pid = fork();
  REQUIRE(child_pid >= 0) << "fork failed: " << strerror(errno);

  if (child_pid > 0) {
    // Parent process
    predecessor_ = child_pid;
    INFO << "Child pid = " << child_pid;

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
  // Use a local variable to get the location in stack where we want to save
  int tmp;
  Gstm::cur_stack = &tmp;
  INFO << "cur_stack = " << Gstm::cur_stack;
  Gstm::cur_stack = Gstm::GetSP();
  INFO << "cur_stack = " << Gstm::cur_stack;

  Gstm::cur_stack_size = reinterpret_cast<uintptr_t>(Gstm::stack_top) -
                         reinterpret_cast<uintptr_t>(Gstm::cur_stack);
  BackupStack();

  // Commit the context
  // This code may return more than once:
  //  1. The first time: a normal return
  //  2. If the commit fails, we enter AtomicAbort(), and there will be a
  //  longjmp function which will return again from here
  if (setjmp(env_)) {
    // Copy the stack back
    memcpy(Gstm::cur_stack, Gstm::stack_backup, Gstm::cur_stack_size);
    INFO << "TID=" << tid_ << ", Rolling back ...";
  } else {
    INFO << "setjmp";
  }

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

  REQUIRE(mprotect(local_heap, PAGE_SIZE, PROT_NONE) == 0)
      << "mprotect failed: " << strerror(errno);

  return;
}

void GThread::AtomicEnd() {
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
    commited = true;
  }
  pthread_mutex_unlock(Gstm::mutex);

  return commited;
}

void GThread::AtomicAbort() {
  // Rollback to where setjmp happens
  longjmp(env_, SET_JPM_MAGIC);
}

void GThread::Join() {
  AtomicEnd();
  Gstm::WaitExited(predecessor_);
  AtomicBegin();
}

void GThread::BackupStack() {
  // Page aligned size
  size_t backup_size =
      Gstm::cur_stack_size / PAGE_SIZE * PAGE_SIZE == Gstm::cur_stack_size
          ? Gstm::cur_stack_size
          : (Gstm::cur_stack_size / PAGE_SIZE + 1) * PAGE_SIZE;
  Gstm::stack_backup = mmap(NULL, backup_size, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  REQUIRE(Gstm::stack_backup != MAP_FAILED)
      << "mmap failed: " << strerror(errno);

  // Save the entire stack space so far.
  memcpy(Gstm::stack_backup, Gstm::cur_stack, Gstm::cur_stack_size);
}
