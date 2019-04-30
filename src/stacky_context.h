#include <setjmp.h>
#include <stdlib.h>

typedef struct {
	sigjmp_buf state;
	void* stack;
	size_t stack_size;
} stacky_context_t;

void save_context(stacky_context_t* context);
void destroy_context(stacky_context_t* context);
void restore_context(stacky_context_t* context);
void initialize_stack();
