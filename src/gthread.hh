#ifndef GTHREAD_HH_
#define GTHREAD_HH_

#include <setjmp.h>
#include <unistd.h>
#include <unordered_set>

// Possibly change to template in the future
class GThread {
 public:
  GThread();

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

  // Back up the stack till the cur_stack
  void BackupStack();

  jmp_buf env_;  // context
  pid_t tid_;
  pid_t predecessor_;  // its child pid
  void *retval_;       // return value from the function
  volatile void
      *stack_backup_;  // backup memory for the stack we want to roll back to
  volatile void *cur_stack_;  // the location on the stack we want to store to
  volatile size_t cur_stack_size_;  // The current stack size
  void *local_heap_;                // Local view of the global heap
};

#endif  // GTHREAD_HH_
