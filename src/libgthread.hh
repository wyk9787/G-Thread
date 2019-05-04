#ifndef LIBGTHREAD_HH_
#define LIBGTHREAD_HH_

#include <dlfcn.h>

#include "color_log.hh"
#include "gstm.hh"
#include "gthread.hh"

typedef int (*main_fn_t)(int argc, char** argv, char** env);
extern "C" int __libc_start_main(main_fn_t main_fn, int argc, char** argv,
                                 void (*init)(), void (*fini)(),
                                 void (*rtld_fini)(), void* stack_end);

static decltype(__libc_start_main)* real_libc_start_main;

struct LibcStartMainArgs {
  main_fn_t main_fn;
  int argc;
  char** argv;
  void (*init)();
  void (*fini)();
  void (*rtld_fini)();
  void* stack_end;
};

void* program(void* arg) {
  LibcStartMainArgs* a = static_cast<LibcStartMainArgs*>(arg);
  int result = real_libc_start_main(a->main_fn, a->argc, a->argv, a->init,
                                    a->fini, a->rtld_fini, a->stack_end);
  return nullptr;
}

extern "C" int __libc_start_main(main_fn_t main_fn, int argc, char** argv,
                                 void (*init)(), void (*fini)(),
                                 void (*rtld_fini)(), void* stack_end) {
  Gstm::Initialize();
  real_libc_start_main = reinterpret_cast<decltype(__libc_start_main)*>(
      dlsym(RTLD_NEXT, "__libc_start_main"));
  LibcStartMainArgs a = {.main_fn = main_fn,
                         .argc = argc,
                         .argv = argv,
                         .init = init,
                         .fini = fini,
                         .rtld_fini = rtld_fini,
                         .stack_end = stack_end};
  GThread t(true);
  t.AtomicBegin();
  t.Create(program, &a);
  t.Join();
  return 0;
}

__attribute__((destructor)) void fini() {
  // ColorLog("END");
  Gstm::Finalize();
}

#endif  // LIBGTHREAD_HH_
