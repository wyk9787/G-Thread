#ifndef GTHREAD_HH_
#define GTHREAD_HH_

#include <pthread.h>
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

  // Create a thread
  static void Create(gthread_t *t, void *(*start_routine)(void *), void *args);

  // Join a thread
  static void Join(gthread_t t);

  // Begins an atomic section
  static void AtomicBegin();

  // Ends an atomic section
  static void AtomicEnd();

  // Initialize GThread
  static void InitGThread();

 private:
  // Abort and rollbakc an atomic section
  static void AtomicAbort();

  // Commit an atomic section
  static bool AtomicCommit();

  // Initialize stack context
  static void InitStackContext();

  static pid_t tid_;             // tid of current processor
  static pid_t predecessor_;     // predecessor's process id
  static StackContext context_;  // stack context
};

#endif  // GTHREAD_HH_
