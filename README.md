# G-THREAD

G-Thread is a safe multithreaded programming interface for C++
based on optimistic concurrency control, or more specifically, software transactional memory(STM). 
G-Thread reimplements a subset of the system called [Grace]
(https://dl.acm.org/citation.cfm?doid=1640089.1640096). 

G-Thread provides an interface close to `pthread` and avoids common currency
bugs entirely such as race condition, atomicity violation, deadlock, etc. By
changing all your `pthread_create` and `pthread_join` into `GThread::Create` and
`GThread::Join` and getting rid of all the locks, your program will be simply
free of all of those concurrency bugs and work correctly.

## Usage

To adapt your program to use G-Thread is straight forward. 

1. Include the library `#include "libgthread.hh"`

2. Use `GThread::Create()` and `GThread::Join()` instead of `pthread_create()`
   and `pthread_join()`

3. Link your program against the GThread library

[test.cc](src/test.cc) is an example program that illustrates the elimination of
race condition using G-Thread. To run this test:

```
make clean all
./src/gtest
```

To print out the logging information while G-Thread is running, uncomment this [line](//#define LOGPRINT)
to define `LOGPRINT` macro and recompile the program.

## Limitations

Those are some of the limitations that G-Thread currently has:

1. Although stack and heap memory will work without any concurrency issues, the
   global variable still need to be protected by locks or other synchronization control
to avoid concurrency bugs.

2. I/O (e.g. printing to the console or writing to a file) within each thread can happen 
multiple times if that particular thread is rolled back due to a conflict with
other thread.

3. No memory used by user's program or G-Thread will ever be reclaimed by OS even if
   user explicitly called `free`.




