#ifndef GTHREAD_HH_
#define GTHREAD_HH_

#include <setjmp.h>
#include <unistd.h>
#include <unordered_set>

#include "stack_context.hh"
#include "util.hh"

struct gthread_t {
  pid_t tid;
  void *retval;
};

// Possibly change to template in the future
class GThread {
 public:
  GThread() = delete;

  // Possibly change to use Functional interface in the future
  static void Create(gthread_t *t, void *(*start_routine)(void *), void *args);

  static void Join(gthread_t t);

  // Begins an atomic section
  static void AtomicBegin();

  static void InitGThread();
  static bool first_gthread_;

 private:
  // Ends an atomic section
  static void AtomicEnd();

  // Abort and rollbakc an atomic section
  static void AtomicAbort();

  // Commit an atomic section
  static bool AtomicCommit();

  static void InitStackContext();

  static pid_t tid_;
  static pid_t predecessor_;     // its child pid
  static StackContext context_;  // stack context
};

#endif  // GTHREAD_HH_
