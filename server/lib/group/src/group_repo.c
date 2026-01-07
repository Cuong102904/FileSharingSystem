#include "../include/group_repo.h"
#include "../include/group.h"
#include <pthread.h>

#include <stdio.h>

// Define the shared rwlock (single definition)
pthread_rwlock_t group_db_rwlock = PTHREAD_RWLOCK_INITIALIZER;

void init_group_rwlock() { pthread_rwlock_init(&group_db_rwlock, NULL); }

void destroy_group_rwlock() { pthread_rwlock_destroy(&group_db_rwlock); }
