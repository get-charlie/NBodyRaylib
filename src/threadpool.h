#ifndef _THREADPOOL_H
#define _THREADPOOL_H

// Minimal POSIX threads worker pool used to parallelize the simulation step.
//
// The pool is persistent: the worker threads are created once and then parked
// on a condition variable, so a simulation step does not pay for thread
// creation. Work is split as a range of body indexes, every worker writes to
// its own slice of the bodies array, so no locking is needed inside the tasks.

typedef void (*ThreadTask)(void* arg, unsigned start, unsigned end, unsigned worker);

typedef struct ThreadPool ThreadPool;

// Number of online processors (1 if it cannot be determined)
unsigned cpu_count(void);

// threads == 0 selects cpu_count(). Returns NULL on failure, callers can keep
// working with a NULL pool: threadpool_run() falls back to serial execution.
ThreadPool* threadpool_create(unsigned threads);
void threadpool_destroy(ThreadPool* pool);
unsigned threadpool_size(const ThreadPool* pool);

// Runs task over [0, items) split between the calling thread and threads - 1
// workers, and returns once every chunk is done. threads is clamped to the
// size of the pool, and a value of 1 runs the task on the calling thread
// without waking anyone up.
void threadpool_run(ThreadPool* pool, ThreadTask task, void* arg, unsigned items, unsigned threads);

#endif
