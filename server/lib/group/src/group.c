#include "../include/group.h"

pthread_mutex_t group_db_mutex = PTHREAD_MUTEX_INITIALIZER;

void group_init(void) {
    pthread_mutex_init(&group_db_mutex, NULL);
}