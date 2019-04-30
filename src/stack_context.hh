#ifndef STACKCONTEXT_
#define STACKCONTEXT_

#include <setjmp.h>
#include <stdlib.h>

#define NO_INLINE __attribute__((noinline))

// Adapted from Johnson Sadun's stacky_context.{c/h}
class StackContext {
 public:
  StackContext();

  void SaveContext();
  void RestoreContext();
  void DestroyContext();

 private:
  void CompleteSave(void* top_of_stack);
  void Phase2Save();
  void CompleteRestore(volatile void* padding);
  void GetStackBottom();

  sigjmp_buf state_;
  void* stack_;
  size_t stack_size_;

  static void* bottom_of_stack_;
};

#endif  // STACKCONTEXT_
