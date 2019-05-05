#include "stack_context.hh"

#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "color_log.hh"
#include "log.h"
#include "util.hh"

// Initialize static class members
void* StackContext::bottom_of_stack_ = nullptr;
bool StackContext::initialized_;

void StackContext::GetStackBottom() {
  // Here we figure out the bottom of the stack, which should be completely
  // stable across all of the program's execution (unless there is some weird
  // stack switching going on). To make sure we don't miss anything, just look
  // for the stack mapping and find where it ends.

  // TODO: Try to not call malloc here
  FILE* maps = fopen("/proc/self/maps", "r");
  char* line = NULL;
  size_t len = 0;
  bool found = false;
  void* beginning;
  void* end;
  while (getline(&line, &len, maps) != -1) {
    size_t line_len = strlen(line);
    if (line_len >= 8 && strcmp(line + line_len - 8, "[stack]\n") == 0) {
      if (sscanf(line, "%p-%p", &beginning, &end) == 2) {
        found = true;
        break;
      }
    }
  }
  free(line);
  if (!found) {
    exit(3);
  }

  // Stacks grow down, so we want the end of the mapping
  bottom_of_stack_ = end;
}

StackContext::StackContext() : stack_(nullptr) {
  if (!initialized_) {
    GetStackBottom();
    initialized_ = true;
  }
}

NO_INLINE void StackContext::CompleteSave(void* top_of_stack) {
  DestroyContext();
  stack_size_ = (uintptr_t)bottom_of_stack_ - (uintptr_t)top_of_stack;
  if (stack_ != nullptr) {
    REQUIRE(munmap(stack_, ROUND_UP(stack_size_, PAGE_SIZE)) == 0)
        << "munmap failed: " << strerror(errno);
  }
  stack_ = mmap(NULL, ROUND_UP(stack_size_, PAGE_SIZE), PROT_READ | PROT_WRITE,
                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  REQUIRE(stack_ != MAP_FAILED) << "mmap failed: " << strerror(errno);

  memset(stack_, ROUND_UP(stack_size_, PAGE_SIZE), 0);
  memcpy(stack_, top_of_stack, stack_size_);
}

NO_INLINE void StackContext::Phase2Save() {
  // We record the top of the stack in an inner function to make sure that we
  // save any state that save_context might have had. __builtin_frame_address(0)
  // should capture an endpoint containing all of save_context's state and a
  // correct return pointer without running into issues of complete_save
  // trampling the stack it is trying to save.
  // Q? : Why is there an extra function call here? Is it necessary?
  CompleteSave(__builtin_frame_address(0));
}

NO_INLINE void StackContext::SaveContext() {
  // First, we need to save the CPU state.
  if (!sigsetjmp(state_, 1)) {
    // NOTO: This comments make it look like this save_context will be called
    // multiple times, but only first ever return 0. It returns 0 if it didn't
    // just siglongjmp here. If setjmp returns 0, this is the first time we are
    // calling (i.e. we are saving, not restoring). Continue doing the save by
    // saving the stack, but do it in a different function so that we can be
    // more sure that we aren't messing around in the very part of the stack
    // that we are trying to save.
    Phase2Save();
  } else {
    ColorLog("<Rollback.S>");
  }
}

void StackContext::CompleteRestore(volatile void* padding) {
  // Restore the stack
  memcpy(reinterpret_cast<void*>(
             (reinterpret_cast<uintptr_t>(bottom_of_stack_) - stack_size_)),
         stack_, stack_size_);
  ColorLog("Before longjmp");
  // And restore the rest of the machine state
  siglongjmp(state_, 1);
}

void StackContext::RestoreContext() {
  volatile void* padding = nullptr;
  // When we finally do a longjmp, we're going to rewind the stack pointer to
  // where it was previously. However, before we do that, we need to restore the
  // actual stack. Therefore, for everything to work, we can't be using that
  // stack space for our local variables; the function that will do the actual
  // restore needs to be running with a stack frame beyond the recorded stack
  // top. Therefore, use alloca to push us past that point.
  if (reinterpret_cast<uintptr_t>(bottom_of_stack_) -
          reinterpret_cast<uintptr_t>(__builtin_frame_address(0)) <
      stack_size_) {
    padding = alloca(stack_size_ -
                     (reinterpret_cast<uintptr_t>(bottom_of_stack_) -
                      reinterpret_cast<uintptr_t>(__builtin_frame_address(0))));
  }

  // Pass the padding to complete restore so that it won't get optimized away.
  CompleteRestore(padding);

  // This is unreachable, but doing some stuff here should help prevent the call
  // to CompleteRestore from being optimized as a tail call
  ((volatile char*)padding)[0] = 0;
}

void StackContext::DestroyContext() {
  if (stack_ != nullptr) {
    REQUIRE(munmap(stack_, ROUND_UP(stack_size_, PAGE_SIZE)) == 0)
        << "munmap failed: " << strerror(errno);
  }
}
