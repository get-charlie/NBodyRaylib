#include "threadpool.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <unistd.h>
#endif

typedef struct {
    ThreadPool* pool;
    unsigned    id;
} Worker;

struct ThreadPool {
    pthread_t*      threads;
    Worker*         workers;
    unsigned        size;           // workers + the calling thread
    unsigned        spawned;

    pthread_mutex_t mutex;
    pthread_cond_t  ready;          // signals a new job to the workers
    pthread_cond_t  done;           // signals the caller that a job finished

    ThreadTask      task;
    void*           arg;
    unsigned        items;
    unsigned long   generation;     // bumped once per job
    unsigned        pending;        // workers still running the current job
    bool            stop;
};

unsigned cpu_count(void)
{
#if defined(_WIN32)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors > 0 ? (unsigned)info.dwNumberOfProcessors : 1;
#elif defined(_SC_NPROCESSORS_ONLN)
    long count = sysconf(_SC_NPROCESSORS_ONLN);
    return count > 0 ? (unsigned)count : 1;
#else
    return 1;
#endif
}

// Every thread derives its own slice from its id, the remainder is spread
// over the first chunks so the split stays balanced for small body counts.
static void run_chunk(ThreadTask task, void* arg, unsigned items, unsigned id, unsigned size)
{
    unsigned chunk = items / size;
    unsigned rest  = items % size;
    unsigned start = id * chunk + (id < rest ? id : rest);
    unsigned end   = start + chunk + (id < rest ? 1 : 0);
    if(start < end){
        task(arg, start, end, id);
    }
}

static void* worker_main(void* data)
{
    Worker* worker = data;
    ThreadPool* pool = worker->pool;
    unsigned long seen = 0;

    for(;;){
        pthread_mutex_lock(&pool->mutex);
        while(!pool->stop && pool->generation == seen){
            pthread_cond_wait(&pool->ready, &pool->mutex);
        }
        if(pool->stop){
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        seen = pool->generation;
        ThreadTask task = pool->task;
        void* arg       = pool->arg;
        unsigned items  = pool->items;
        unsigned size   = pool->size;
        pthread_mutex_unlock(&pool->mutex);

        run_chunk(task, arg, items, worker->id, size);

        pthread_mutex_lock(&pool->mutex);
        pool->pending--;
        if(pool->pending == 0){
            pthread_cond_signal(&pool->done);
        }
        pthread_mutex_unlock(&pool->mutex);
    }
    return NULL;
}

ThreadPool* threadpool_create(unsigned threads)
{
    if(threads == 0){
        threads = cpu_count();
    }
    if(threads > 256){
        threads = 256;
    }

    ThreadPool* pool = calloc(1, sizeof(ThreadPool));
    if(pool == NULL){
        return NULL;
    }
    pool->size = threads;

    if(pthread_mutex_init(&pool->mutex, NULL) != 0){
        free(pool);
        return NULL;
    }
    if(pthread_cond_init(&pool->ready, NULL) != 0){
        pthread_mutex_destroy(&pool->mutex);
        free(pool);
        return NULL;
    }
    if(pthread_cond_init(&pool->done, NULL) != 0){
        pthread_cond_destroy(&pool->ready);
        pthread_mutex_destroy(&pool->mutex);
        free(pool);
        return NULL;
    }

    // The calling thread takes chunk 0, so only size - 1 workers are spawned
    if(threads > 1){
        pool->threads = calloc(threads - 1, sizeof(pthread_t));
        pool->workers = calloc(threads - 1, sizeof(Worker));
        if(pool->threads == NULL || pool->workers == NULL){
            threadpool_destroy(pool);
            return NULL;
        }
        for(unsigned i = 0; i < threads - 1; i++){
            pool->workers[i].pool = pool;
            pool->workers[i].id   = i + 1;
            if(pthread_create(&pool->threads[i], NULL, worker_main, &pool->workers[i]) != 0){
                // Keep going with the threads that could be created
                pool->size = pool->spawned + 1;
                break;
            }
            pool->spawned++;
        }
    }
    return pool;
}

void threadpool_destroy(ThreadPool* pool)
{
    if(pool == NULL){
        return;
    }
    pthread_mutex_lock(&pool->mutex);
    pool->stop = true;
    pthread_cond_broadcast(&pool->ready);
    pthread_mutex_unlock(&pool->mutex);

    for(unsigned i = 0; i < pool->spawned; i++){
        pthread_join(pool->threads[i], NULL);
    }

    pthread_cond_destroy(&pool->done);
    pthread_cond_destroy(&pool->ready);
    pthread_mutex_destroy(&pool->mutex);
    free(pool->threads);
    free(pool->workers);
    free(pool);
}

unsigned threadpool_size(const ThreadPool* pool)
{
    return pool == NULL ? 1 : pool->size;
}

void threadpool_run(ThreadPool* pool, ThreadTask task, void* arg, unsigned items, unsigned min_items)
{
    if(items == 0){
        return;
    }
    if(pool == NULL || pool->size <= 1 || items < min_items){
        task(arg, 0, items, 0);
        return;
    }

    pthread_mutex_lock(&pool->mutex);
    pool->task    = task;
    pool->arg     = arg;
    pool->items   = items;
    pool->pending = pool->size - 1;
    pool->generation++;
    pthread_cond_broadcast(&pool->ready);
    pthread_mutex_unlock(&pool->mutex);

    run_chunk(task, arg, items, 0, pool->size);

    pthread_mutex_lock(&pool->mutex);
    while(pool->pending > 0){
        pthread_cond_wait(&pool->done, &pool->mutex);
    }
    pthread_mutex_unlock(&pool->mutex);
}
