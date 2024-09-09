# MyThread

This repository provides an implementation of a cooperative multithreading
library in C using the ucontext.h library. The library implements core
threading functionality, including thread creation, joining, locking, waiting,
and signaling. It also supports context switching and managing thread queues.

## Features
* Thread creation and execution with context switching
* Mutex locking and unlocking
* Conditional waiting and signaling
* Thread joining and result retrieval
* Cooperative multitasking using a round-robin scheduling mechanism

## Key Components

### Thread

Each thread is represented by a Thread structure:

``` c
Copy code
typedef struct Thread {
	int id;
	void *result;
	ucontext_t *context;
	struct Thread *next;
} Thread;
id: Thread ID.

```

Definition:
- result: Pointer to the result returned by the thread.
- context: Execution context of the thread (stack, register values, etc.).
- next: Pointer to the next thread in the queue.

### Queue

A Queue structure is used to manage the list of threads waiting to be scheduled:

``` c
Copy code
typedef struct Queue {
	Thread *head;
	Thread *tail;
} Queue;

```

Variables:
- head: The first thread in the queue.
- tail: The last thread in the queue.
- Threads can be in one of three states:
  - RUNNING: The thread is actively running or is scheduled to run.
  - FINISHED: The thread has completed execution but has not been cleaned up.
  - DEAD: The thread has completed execution and its resources have been cleaned.

## Function Overview

### Thread Management

* void threadInit(): Initializes the threading system, creating the first thread (main thread), allocating memory for thread state tracking, and setting up interrupt handling.
* int threadCreate(thFuncPtr funcPtr, void *argPtr): Creates a new thread and schedules it to run a function (funcPtr). The argument to the function is provided through argPtr.
* void threadYield(): Causes the current thread to yield the CPU, allowing other threads to run. It swaps the context to the next thread in the queue.
* void threadJoin(int id, void **result): Allows one thread to wait for another thread (identified by id) to finish. The result of the finished thread can be retrieved through the result pointer.
* void threadExit(void *result): Terminates the current thread and stores its result.

### Synchronization

* void threadLock(int lockNum): Attempts to acquire a mutex lock identified by lockNum. If the lock is already held, the thread is suspended until the lock is released.
* void threadUnlock(int lockNum): Releases the mutex lock identified by lockNum, and wakes up any threads waiting for this lock.
* void threadWait(int lockNum, int conditionNum): Releases the lock and suspends the thread on the condition identified by conditionNum. Once the condition is signaled, the thread will re-acquire the lock and resume execution.
* void threadSignal(int lockNum, int conditionNum): Wakes up one thread waiting on the condition identified by conditionNum.

### Cleanup

* void threadClean(): Cleans up resources used by the threading system, freeing memory allocated for thread states and waiting queues.

## Usage Example

``` c
#include "mythreads.h"

void *threadFunction(void *arg) {
    printf("Thread %d is running\n", *(int *)arg);
    return NULL;
}

int main() {
    threadInit();

    int arg1 = 1;
    int arg2 = 2;

    threadCreate(threadFunction, &arg1);
    threadCreate(threadFunction, &arg2);

    threadYield();

    threadClean();
    return 0;
}
```

```
   +------------------+       
   |  Main Thread      |       
   +------------------+       
            |                          
            v                          
   +------------------+       
   | Thread 1 Created  |       
   +------------------+       
            |                          
            v                          
   +------------------+      Thread 2 enters
   | Thread 1 Running  |----->+------------------+
   +------------------+       | Thread 2 Created |
            |                  +------------------+
            |                          
            v                          
   +------------------+      
   | Thread 1 Yields   |      
   +------------------+      
            |                          
            v                          
   +------------------+      Thread 2 starts running
   | Thread 2 Running  |      
   +------------------+      
```

## Build and Test Library

Build:
- run `make`

Build tests:
- `cd test`
- run `make`

