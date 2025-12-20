#include "../include/group.h"
#include <pthread.h>
#include "../include/group_repo.h"

#include <stdio.h>

// Define the shared mutex (single definition)
pthread_mutex_t group_db_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_mutex() {
    pthread_mutex_init(&group_db_mutex, NULL);
}


