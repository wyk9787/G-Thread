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

static main_fn_t real_main;

struct MainArgs {
  int argc;
  char** argv;
  char** env;
};

void* program(void* arg) {
  MainArgs* a = (MainArgs*)arg;
  real_main(a->argc, a->argv, a->env);
  GThread::AtomicEnd();
  _exit(0);
}

int wrapped_main(int argc, char** argv, char** env) {
  MainArgs arg = {.argc = argc, .argv = argv, .env = env};
  GThread::InitGThread();
  gthread_t t;
  GThread::AtomicBegin();
  GThread::Create(&t, program, &arg);
  GThread::Join(t);
  GThread::AtomicEnd();
  _exit(0);
}

extern "C" int __libc_start_main(main_fn_t main_fn, int argc, char** argv,
                                 void (*init)(), void (*fini)(),
                                 void (*rtld_fini)(), void* stack_end) {
  GlobalHeapInit();
  auto real_libc_start_main = reinterpret_cast<decltype(__libc_start_main)*>(
      dlsym(RTLD_NEXT, "__libc_start_main"));
  real_main = main_fn;
  int result = real_libc_start_main(wrapped_main, argc, argv, init, fini,
                                    rtld_fini, stack_end);
  return result;
}

__attribute__((destructor)) void fini() {
  // ColorLog("END");
  Gstm::Finalize();
}

#endif  // LIBGTHREAD_HH_
