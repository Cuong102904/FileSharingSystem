#include "../include/thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Worker thread function
static void *worker_thread(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;

    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);

        // Wait for task or shutdown
        while (pool->queue_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_not_empty, &pool->queue_mutex);
        }

        // Check for shutdown
        if (pool->shutdown && pool->queue_count == 0) {
            pthread_mutex_unlock(&pool->queue_mutex);
            pthread_exit(NULL);
        }

        // Get task from queue
        Task task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % pool->queue_size;
        pool->queue_count--;

        // Signal that queue has space
        pthread_cond_signal(&pool->queue_not_full);
        pthread_mutex_unlock(&pool->queue_mutex);

        // Execute task
        if (task.function != NULL) {
            task.function(task.arg);
        }
    }

    return NULL;
}

ThreadPool *thread_pool_create(int thread_count) {
    if (thread_count <= 0) {
        thread_count = DEFAULT_THREAD_COUNT;
    }

    ThreadPool *pool = malloc(sizeof(ThreadPool));
    if (pool == NULL) {
        return NULL;
    }

    pool->thread_count = thread_count;
    pool->shutdown = 0;
    pool->queue_size = MAX_QUEUE_SIZE;
    pool->queue_front = 0;
    pool->queue_rear = 0;
    pool->queue_count = 0;

    // Allocate threads array
    pool->threads = malloc(sizeof(pthread_t) * thread_count);
    if (pool->threads == NULL) {
        free(pool);
        return NULL;
    }

    // Allocate task queue
    pool->task_queue = malloc(sizeof(Task) * pool->queue_size);
    if (pool->task_queue == NULL) {
        free(pool->threads);
        free(pool);
        return NULL;
    }

    // Initialize synchronization primitives
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_not_empty, NULL);
    pthread_cond_init(&pool->queue_not_full, NULL);

    // Create worker threads
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            // Cleanup on failure
            pool->shutdown = 1;
            pthread_cond_broadcast(&pool->queue_not_empty);
            for (int j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            pthread_mutex_destroy(&pool->queue_mutex);
            pthread_cond_destroy(&pool->queue_not_empty);
            pthread_cond_destroy(&pool->queue_not_full);
            free(pool->task_queue);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }

    printf("Thread pool created with %d workers\n", thread_count);
    return pool;
}

void thread_pool_destroy(ThreadPool *pool) {
    if (pool == NULL) {
        return;
    }

    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->queue_not_empty);
    pthread_mutex_unlock(&pool->queue_mutex);

    // Wait for all threads to finish
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    // Cleanup
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_not_empty);
    pthread_cond_destroy(&pool->queue_not_full);
    free(pool->task_queue);
    free(pool->threads);
    free(pool);

    printf("Thread pool destroyed\n");
}

int thread_pool_add_task(ThreadPool *pool, task_func_t function, void *arg) {
    if (pool == NULL || function == NULL) {
        return -1;
    }

    pthread_mutex_lock(&pool->queue_mutex);

    // Wait if queue is full
    while (pool->queue_count == pool->queue_size && !pool->shutdown) {
        pthread_cond_wait(&pool->queue_not_full, &pool->queue_mutex);
    }

    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->queue_mutex);
        return -1;
    }

    // Add task to queue
    pool->task_queue[pool->queue_rear].function = function;
    pool->task_queue[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_size;
    pool->queue_count++;

    // Signal that queue has tasks
    pthread_cond_signal(&pool->queue_not_empty);
    pthread_mutex_unlock(&pool->queue_mutex);

    return 0;
}

int thread_pool_pending_tasks(ThreadPool *pool) {
    if (pool == NULL) {
        return 0;
    }

    pthread_mutex_lock(&pool->queue_mutex);
    int count = pool->queue_count;
    pthread_mutex_unlock(&pool->queue_mutex);

    return count;
}
