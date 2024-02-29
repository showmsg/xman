#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <pthread.h>

typedef struct XThreadPool XThreadPool;

// Creates a thread pool and returns a pointer to it.
XThreadPool *XThreadPoolInit(int max_threads);

// Return maximum number of threads.
int XThreadPoolGetMaxThrds(XThreadPool *);

// Insert task into thread pool. The pool will call work_routine and pass arg
// as an argument to it. This is a similar interface to pthread_create.
void XThreadPoolAddTask(XThreadPool *, void *(*work_routine)(void *), void *arg);

// Blocks until the thread pool is done executing its tasks.
void XThreadPoolWait(XThreadPool *);

// Cleans up the thread pool, frees memory. Waits until work is done.
void XThreadPoolDestroy(XThreadPool *);

#endif

