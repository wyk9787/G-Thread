#ifndef _XCONTEXT_H_
#define _XCONTEXT_H_

#include <signal.h>
#include <stdio.h>

#include <ucontext.h>

/**
 * @class xcontext
 * @brief Lets a program rollback to a previous execution context.
 *
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 * @note  Adapted from code by Dan Piponi
 * <http://homepage.mac.com/sigfpe/Computing/continuations.html>
 */

#define NO_INLINE __attribute__((noinline))

class xcontext {
 public:
  /// @brief Save current calling context (i.e., current continuation).
  NO_INLINE void commit();

  /// @brief Restore the previously saved context.
  NO_INLINE void abort();

  /// @brief Initialize this with the highest pointer possible on the stack.
  NO_INLINE void initialize();

 private:
  static void *stacktop(void);

  /// A pointer to the base (highest address) of the stack.
  unsigned long *_pbos;

  /// How big can the stack be (in words).
  enum { MAX_STACK_SIZE = 1048576 };

  /// The saved registers, etc.
  ucontext_t _registers;

  /// Current saved stack size.
  int _stackSize;

  /// The saved stack contents.
  unsigned long _stack[MAX_STACK_SIZE];

  NO_INLINE void save_stack(unsigned long *pbos, unsigned long *ptos);

  NO_INLINE void getContext(void);

  NO_INLINE void restoreStack(int once_more);
};

#endif
