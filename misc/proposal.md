# Proposal

## Introduction

Multithreaded programs are more popular than ever nowadays given the increase 
number of processors on a single machine. However, being able to write a 
concurrent program is hard and being able to write a bug-free concurrent program 
is even harder. Multithreaded applications often face hard-to-find errors such as 
atomicity violation, race condition and order violations. With synchronization 
primitives such as locks, condition variables, semaphores, etc., it’s also very 
easy to have issues such as deadlock and livelock. This project is trying to 
reimplement a subset of the system named Grace, a software-only runtime system 
that eliminates concurrency errors for a class of multithreaded programs: 
those based on fork-join parallelism. The goal for this project is to
reimplement as much Grace as possible. A likely section that will be discarded
is the support for transactional I/O due to its complexity.

## Approach

One of the major ideas that Grace adopts is software transaction memory (STM). 
However, typical STMs are optimized for short transactions, and incur substantial 
space and runtime overhead for fully-isolated memory updates inside transactions. 
Thus, Grace comes up with this novel but efficient way to implement software 
transactional memory by treating threads as processes. By turning threads into 
process, since now they have different address spaces, all of the problems we 
mentioned above are gone. The problems come with processes are now any change made 
within the process is not visible to other processes. Thus, Grace provides 
sequential semantics which enforces a particular commit order: a thread (a process 
in reality) is only allowed to commit after all of its logical predecessors have 
completed.	

The basic idea behind Grace is straightforward as demonstrated in the code
snippets below:

Suppose you have a multithreaded program:

```c
// Run f1 and f2 in parallel
pthread_t t1, t2;

t1 = pthread_create(&t1, NULL, f1, NULL);
t2 = pthread_create(&t2, NULL, f2, NULL);

pthread_join(t1, NULL);
pthread_join(t2, NULL);
```

And by using Grace, we will turn them into:

```c
// Run f1 to completion, then f2
f1();
f2();
```

It seems that `f1` and `f2` are running sequentially, they are really running in
parallel and after they are done, they commit sequentially according to the
sequential semantics.
 
## Evaluation

I will evaluate this system by answering the following questions:

1. Is the system really able to be free of the memory errors mentioned above?

   The whole purpose of this system is to turn a normal multithreaded program
into serial-like program free of concurrency bugs such as atomicity violation,
order violation and deadlock, etc. Thus, being able to run threads in parallel
but free of those bugs is the most important goal of this project.

2. How much overhead will this system impose on the multithread programs
   comparing to programs that just use threads and locks?

   With large numbers of rollbacks, there is no doubt that the system will be 
slower than normal multithreaded programs. However, how much performance
overhead will this impose on the program is a critical factor regarding how
useable this system is.

3. How easy is it to use this system?

   Grace is provided as a runtime library which can be linked against user's
program and requires minimal change of the source file (in some cases). Is this
reimplementation gonna be as user-friendly as Grace?


## Relevant Literatures


1. Emery D. Berger, Ting Yang, Tongping Liu, and Gene Novark. 2009. [Grace: safe 
multithreaded programming for C/C++](https://dl.acm.org/citation.cfm?doid=1640089.1640096). 
In Proceedings of the 24th ACM SIGPLAN conference on Object oriented programming 
systems languages and applications (OOPSLA '09). ACM, New York, NY, USA, 81-96. 

   This paper is the foundation of this project as it provides detailed insights
and implementations on how to implement this system.


2. G. Kestor et al., [STM2: A Parallel STM for High Performance Simultaneous 
Multithreading Systems]( http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=6113831&isnumber=6113772)
2011 International Conference on Parallel Architectures and Compilation 
Techniques, Galveston, TX, 2011, pp. 221-231.
  
   This paper provides a good starting point for me to understand how a
traditional STM works and how it is implemented. Even though Grace doesn't
really adopt the traditional approach for STM, it still provides me with some
insights into understanding STM.

3. E.D. Berger, K.S. McKinley, R. D. Blumofe, and P.R. Wilson. [Hoard: A
   scalable memory allocator for multithreaded applications](https://dl.acm.org/citation.cfm?id=379232).
In Proceedings of the International Conference on Architectural Support for Programming
Languages and Operating Systems(ASPLOS-IX), pages 117-128, New York, NY, USA,
Nov. 2000. ACM.

   Hoard, as a scalable "per-thread" memory allocator, serves as one of the 
most important building blocks for Grace. Even though I am not sure in the end,
I really need to implement a memory allocator myself, it will help me understand
why a custom memory allocator is needed.

4. Maurice Herlihy and J. Eliot B. Moss. 1993. [Transactional memory: architectural
support for lock-free data structures](https://dl.acm.org/citation.cfm?id=165164).
In Proceedings of the 20th annual international 
symposium on computer architecture (ISCA '93). ACM, New York, NY,
USA, 289-300. 

   Like STM2, this paper serves as the foundation stone for transactional memory,
which provides me with the basic idea of how a transactional memory is
first brought up to real life. It also helps me understand why traditional STM
is not suitable for a system like Grace.

## Starting Code Snippet

The first thing that I will implement is to have a drafted API for my user to
user, which turns thread creation into `fork`. It will be something along the
line of:

```cpp
// gthread.hh

#ifndef GTHREAD_HH
#define GTHREAD_HH

// Possibly change to template in the future
class GThread {
 public:
  // Possibly change to use Functional interface in the future
  GThread(void *(*start_routine)(void *), void *args);

  void Join();
};

#endif  // GTHREAD_HH
```

```cpp
// test.cc

#include <iostream>
#include <vector>

#include "gthread.hh"

#define THREAD_NUM 4

void *fn(void *arg) {
  int num = *(int *)arg;
  std::cout << "This is test function " << num;
  return NULL;
}

int main() {
  std::vector<GThread> ts;
  std::vector<int> args;
  for (int i = 0; i < THREAD_NUM; i++) {
    args.push_back(i);
    ts.emplace_back(fn, (void *)&args[i]);
  }

  for (int i = 0; i < THREAD_NUM; i++) {
    ts[i].Join();
  }
}
```
