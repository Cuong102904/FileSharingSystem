#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

#define DEFAULT_THREAD_COUNT 10
#define MAX_QUEUE_SIZE 1024

// Task function type
typedef void (*task_func_t)(void *arg);

// Task structure
typedef struct {
    task_func_t function;
    void *arg;
} Task;

// Thread pool structure
typedef struct {
    pthread_t *threads;
    int thread_count;

    Task *task_queue;
    int queue_size;
    int queue_front;
    int queue_rear;
    int queue_count;

    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;

    int shutdown;
} ThreadPool;

// Initialize thread pool with specified number of threads
ThreadPool *thread_pool_create(int thread_count);

// Destroy thread pool
void thread_pool_destroy(ThreadPool *pool);

// Add task to the pool
int thread_pool_add_task(ThreadPool *pool, task_func_t function, void *arg);

// Get number of pending tasks
int thread_pool_pending_tasks(ThreadPool *pool);

#endif // THREAD_POOL_H
