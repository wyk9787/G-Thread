#ifndef GTHREAD_HH_
#define GTHREAD_HH_

#include <setjmp.h>
#include <unistd.h>
#include <unordered_set>

#include "stack_context.hh"

// Possibly change to template in the future
class GThread {
 public:
  GThread();

  // static GThread *GetInstance();

  // Possibly change to use Functional interface in the future
  void Create(void *(*start_routine)(void *), void *args);

  void Join();

  pid_t GetTid() { return tid_; }
  void *GetRetVal() { return retval_; }

 private:
  // Begins an atomic section
  void AtomicBegin();

  // Ends an atomic section
  void AtomicEnd();

  // Abort and rollbakc an atomic section
  void AtomicAbort();

  // Commit an atomic section
  bool AtomicCommit();

  jmp_buf env_;  // context
  pid_t tid_;
  pid_t predecessor_;     // its child pid
  void *retval_;          // return value from the function
  void *local_heap_;      // Local view of the global heap
  StackContext context_;  // stack context
};

#endif  // GTHREAD_HH_
