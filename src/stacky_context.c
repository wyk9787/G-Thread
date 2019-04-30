#include "stacky_context.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <setjmp.h>

// Inspired by http://homepage.mac.com/sigfpe/Computing/continuations.html, but rewritten from scratch

static void* bottom_of_stack;

void destroy_context(stacky_context_t *context) {
	free(context->stack);
}

__attribute__((noinline)) static void complete_save(stacky_context_t* context, void* top_of_stack) {
	context->stack_size = bottom_of_stack - top_of_stack;
	context->stack = malloc(context->stack_size);
	memcpy(context->stack, top_of_stack, context->stack_size);
}
__attribute__((noinline)) static void phase2_save(stacky_context_t* context) {
	// We record the top of the stack in an inner function to make sure that we save any state that
	// save_context might have had. __builtin_frame_address(0) should capture an endpoint containing
	// all of save_context's state and a correct return pointer without running into issues of complete_save
	// trampling the stack it is trying to save.
	// Q? : Why is there an extra function call here? Is it necessary?
	complete_save(context, __builtin_frame_address(0));
}

__attribute__((noinline)) void save_context(stacky_context_t* context) {
	// First, we need to save the CPU state.
	if (!sigsetjmp(context->state, 1)) {
		// NOTO: This comments make it look like this save_context will be called multiple times, but only
		// first ever return 0. It returns 0 if it didn't just siglongjmp here.
		// If setjmp returns 0, this is the first time we are calling (i.e. we are saving, not restoring).
		// Continue doing the save by saving the stack, but do it in a different function so that we can
		// be more sure that we aren't messing around in the very part of the stack that we are trying to
		// save.
		phase2_save(context);
	}
}

__attribute__((noinline)) static void complete_restore(stacky_context_t* context, volatile void* _padding) {
	// Restore the stack
	memcpy(bottom_of_stack - context->stack_size, context->stack, context->stack_size);
	// And restore the rest of the machine state
	siglongjmp(context->state, 1);
}

__attribute__((noinline)) void restore_context(stacky_context_t* context) {
	volatile void* padding = NULL;
	// When we finally do a longjmp, we're going to rewind the stack pointer to where it was previously.
	// However, before we do that, we need to restore the actual stack. Therefore, for everything to work,
	// we can't be using that stack space for our local variables; the function that will do the actual
	// restore needs to be running with a stack frame beyond the recorded stack top. Therefore, use
	// alloca to push us past that point.
	if (bottom_of_stack - __builtin_frame_address(0) < context->stack_size) {
		padding = alloca(context->stack_size - (bottom_of_stack - __builtin_frame_address(0)));
	}

	// Pass the padding to complete restore so that it won't get optimized away.
	complete_restore(context, padding);

	// This is unreachable, but doing some stuff here should help prevent the call to complete_restore
	// from being optimized as a tail call
	((volatile char*)padding)[0] = 0;
}

void initialize_stack() {
	// Here we figure out the bottom of the stack, which should be completely stable across all of the program's
	// execution (unless there is some weird stack switching going on). To make sure we don't miss anything,
	// just look for the stack mapping and find where it ends.
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
				fprintf(stderr, "Found map from %p to %p!\n", beginning, end);
				break;
			}
		}
	}
	free(line);
	if (!found) {
		exit(3);
	}

	// Stacks grow down, so we want the end of the mapping
	bottom_of_stack = end;
}
