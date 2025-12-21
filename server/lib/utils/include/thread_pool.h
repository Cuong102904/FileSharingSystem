#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "../../session/include/session.h"
#include <pthread.h>
#include <stdbool.h>

#define MAX_QUEUE_SIZE 256

typedef enum { JOB_UPLOAD, JOB_DOWNLOAD } JobType;

typedef struct {
  JobType type;
  Session *session;
  char arg1[256]; // e.g., group_name
  char arg2[512]; // e.g., file_path
  char arg3[512]; // e.g., server_path
} WorkJob;

typedef struct {
  WorkJob jobs[MAX_QUEUE_SIZE];
  int head;
  int tail;
  int count;
  pthread_mutex_t lock;
  pthread_cond_t not_empty;
  pthread_cond_t not_full;
  bool shutdown;
} WorkQueue;

typedef struct {
  pthread_t *threads;
  int num_threads;
  WorkQueue queue;
} ThreadPool;

// Initialize thread pool with num_threads
ThreadPool *threadpool_create(int num_threads);

// Add job to the queue
void threadpool_add_job(ThreadPool *pool, WorkJob job);

// Destroy thread pool
void threadpool_destroy(ThreadPool *pool);

#endif // THREAD_POOL_H
