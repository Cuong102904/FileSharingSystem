#include "../include/thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Forward declarations of task handlers (to be linked later)
// For now we will just print basic info
void perform_upload_task(WorkJob *job);
void perform_download_task(WorkJob *job);

static void *worker_routine(void *arg) {
  ThreadPool *pool = (ThreadPool *)arg;

  while (1) {
    pthread_mutex_lock(&pool->queue.lock);

    while (pool->queue.count == 0 && !pool->queue.shutdown) {
      pthread_cond_wait(&pool->queue.not_empty, &pool->queue.lock);
    }

    if (pool->queue.shutdown) {
      pthread_mutex_unlock(&pool->queue.lock);
      pthread_exit(NULL);
    }

    // Get job
    WorkJob job = pool->queue.jobs[pool->queue.head];
    pool->queue.head = (pool->queue.head + 1) % MAX_QUEUE_SIZE;
    pool->queue.count--;

    pthread_cond_signal(&pool->queue.not_full);
    pthread_mutex_unlock(&pool->queue.lock);

    // Process job
    if (job.session) {
      printf("[Worker] Processing job for socket %d\n", job.session->socket_fd);
      switch (job.type) {
      case JOB_UPLOAD:
        perform_upload_task(&job);
        break;
      case JOB_DOWNLOAD:
        perform_download_task(&job);
        break;
      }
    }
  }
  return NULL;
}

ThreadPool *threadpool_create(int num_threads) {
  ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
  if (!pool)
    return NULL;

  pool->num_threads = num_threads;
  pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);

  // Init queue
  pool->queue.head = 0;
  pool->queue.tail = 0;
  pool->queue.count = 0;
  pool->queue.shutdown = false;
  pthread_mutex_init(&pool->queue.lock, NULL);
  pthread_cond_init(&pool->queue.not_empty, NULL);
  pthread_cond_init(&pool->queue.not_full, NULL);

  // Start workers
  for (int i = 0; i < num_threads; i++) {
    pthread_create(&pool->threads[i], NULL, worker_routine, pool);
  }

  return pool;
}

void threadpool_add_job(ThreadPool *pool, WorkJob job) {
  pthread_mutex_lock(&pool->queue.lock);

  while (pool->queue.count == MAX_QUEUE_SIZE && !pool->queue.shutdown) {
    pthread_cond_wait(&pool->queue.not_full, &pool->queue.lock);
  }

  if (pool->queue.shutdown) {
    pthread_mutex_unlock(&pool->queue.lock);
    return;
  }

  pool->queue.jobs[pool->queue.tail] = job;
  pool->queue.tail = (pool->queue.tail + 1) % MAX_QUEUE_SIZE;
  pool->queue.count++;

  pthread_cond_signal(&pool->queue.not_empty);
  pthread_mutex_unlock(&pool->queue.lock);
}

void threadpool_destroy(ThreadPool *pool) {
  if (!pool)
    return;

  pthread_mutex_lock(&pool->queue.lock);
  pool->queue.shutdown = true;
  pthread_cond_broadcast(&pool->queue.not_empty);
  pthread_mutex_unlock(&pool->queue.lock);

  for (int i = 0; i < pool->num_threads; i++) {
    pthread_join(pool->threads[i], NULL);
  }

  free(pool->threads);
  pthread_mutex_destroy(&pool->queue.lock);
  pthread_cond_destroy(&pool->queue.not_empty);
  pthread_cond_destroy(&pool->queue.not_full);
  free(pool);
}

// Temporary stubs - will move to handlers logic later
void perform_upload_task(WorkJob *job) {
  printf("STUB: Performing upload %s -> %s\n", job->arg2, job->arg1);
  // TODO: Call actual handle_upload_blocking code here
  // For now simulate work
  sleep(1);
}

void perform_download_task(WorkJob *job) {
  (void)job;
  printf("STUB: Performing download\n");
}
