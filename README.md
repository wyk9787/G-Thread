# G-Thread

G-Thread is a lock-free software-only runtime system for C++ that eliminates concurrency
errors for fork-join parallel programs. G-Thread reimplements a subset of the system called [Grace](https://dl.acm.org/citation.cfm?doid=1640089.1640096). 

By including the header file and linking against the G-Thread library, your
pthread programs are easily free of concurrency errors such as atomicity
violation, order violation, race conditions, etc.

## Usage

To adapt your pthread program to use G-Thread is simple:

1. Include the library `#include "libgthread.hh"`

2. Copy over your program into [src](src/) directory and run `make`

3. Inside `src` directory, run `./gtest`

[test.cc](example/test.cc) is an example program that illustrates the elimination of
race condition using G-Thread. Other examples can be found in
[example](example/) directory or you can easily adapt problematic programs in [pthread](pthread/) directory to use gthread by including `libgthread.hh`.

To run this test:

```
cd src/
cp ../example/test.cc ./
make clean all
./src/gtest
```

To print out the logging information while G-Thread is running, uncomment this [line](https://github.com/wyk9787/G-STM/blob/540907da27075547b1303332e2a1e33cd84b39d4/src/util.hh#L25)
to define `LOGPRINT` macro and recompile the program.

## Limitations

Those are some of the limitations that G-Thread currently has:

0. G-Thread only supports fork-join parallelism and does not support programs
   with concurrency control through synchronization primitives such as condition
variables.

1. Although stack and heap memory will work without any concurrency issues, the
   global variable still need to be protected by locks or other synchronization control
to avoid concurrency bugs.

2. I/O (e.g. printing to the console or writing to a file) within each thread can happen 
multiple times if that particular thread is rolled back due to a conflict with
other thread.

3. No memory used by user's program or G-Thread will ever be reclaimed by OS even if
   user explicitly called `free`.

4. Return value from the thread isn't supported yet.

5. Only heap-allocated memory can be passed into threads without concurrency issues. Stack-allocated variables passed into threads are not supported at this moment.

## Order violation

G-Thread is able to sacrifice some performance to guarantee sequential semantics
of the program. You can comment out this [line](https://github.com/wyk9787/G-Thread/blob/f98cf68f28cbb66e7640216508d71dbb02e96cd2/src/util.hh#L27)
to not define `NO_ORDER` macro and recompile the program to let the program be free of
order violation. See [order_violation.cc](pthread/order_violation.cc) for illustration of order violation.
