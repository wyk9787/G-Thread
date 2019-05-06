#include "gthread.hh"

#include <pthread.h>
#include <string.h>
#include <sys/mman.h>
#include <sstream>

#include "color_log.hh"
#include "gstm.hh"
#include "log.h"
#include "util.hh"
// Initialize static variables
pid_t GThread::tid_ = 0;
pid_t GThread::predecessor_ = 0;
StackContext GThread::context_;

void GThread::InitGThread() {
  tid_ = getpid();
  Gstm::Initialize();
  context_.InitStackContext();
}

void GThread::Create(gthread_t *t, void *(*start_routine)(void *), void *args) {
  AtomicEnd();

  int child_pid = fork();
  if (child_pid < 0) {
    ColorLog("FORK FAILED!!! QUITING ...");
    exit(1);
  }

  if (child_pid > 0) {
    // Parent process
    t->tid = child_pid;
    predecessor_ = child_pid;
    ColorLog("<fork>\t\tchild pid: " << child_pid);
    AtomicBegin();
    return;
  } else {
    // Child process
    tid_ = getpid();
    predecessor_ = 0;

    AtomicBegin();
    // Execute thread function
    start_routine(args);
    AtomicEnd();

    _exit(0);

    return;
  }
}

void GThread::print_map(int i) {
  // std::cerr << "No. " << i << std::endl;
  // std::cerr << "---------------------START--------------------" << std::endl;
  // for (const auto &p : *Gstm::local_page_version) {
  // std::cerr << p.first << ": " << p.second << std::endl;
  //}
  // std::cerr << "---------------------END--------------------" << std::endl;
}

void GThread::AtomicBegin() {
  ColorLog("<a.beg>");

  // Save the context
  context_.SaveContext();

  // Clear the local version mappings
  Gstm::read_set_version->clear();
  Gstm::write_set_version->clear();

  print_map(0);
  Gstm::local_page_version->clear();

  // std::cerr << "---------------------START--------------------" << std::endl;
  pthread_mutex_lock(Gstm::mutex);
  for (const auto &p : *Gstm::global_page_version) {
    Gstm::local_page_version->insert(p);
    // std::cerr << getpid() << ": " << p.first << ": " << p.second <<
    // std::endl;
  }
  pthread_mutex_unlock(Gstm::mutex);
  // std::cerr << "---------------------END--------------------" << std::endl;

  // Turn off all permission on the local heap
  REQUIRE(mprotect(local_heap, HEAP_SIZE, PROT_NONE) == 0)
      << "mprotect failed: " << strerror(errno);

  print_map(1);

  return;
}

void GThread::AtomicEnd() {
  print_map(2);
  ColorLog("<a.end>");
  if (!AtomicCommit()) {
    print_map(3);
    AtomicAbort();
  }
  print_map(4);
}

bool GThread::AtomicCommit() {
  // If we haven't read or written anything
  // we don't have to wait or commitUpdate local view of memory and return
  // true
  if (Gstm::read_set_version->empty() && Gstm::write_set_version->empty()) {
    ColorLog("<com.S>\t\tNo read & write");
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

void GThread::Join(gthread_t t) {
  AtomicEnd();
  print_map(5);
  Gstm::WaitExited(t.tid);
  print_map(6);
  AtomicBegin();
}
