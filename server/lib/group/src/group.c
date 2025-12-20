#include "../include/group.h"
#include <pthread.h>
#include "../include/group_repo.h"

#include <stdio.h>


pthread_mutex_t group_db_mutex = PTHREAD_MUTEX_INITIALIZER;
int start_group_id = 0;

void init_mutex() {
    pthread_mutex_init(&group_db_mutex, NULL);
}

// Constructor
Group new_group(int group_id, int admin_id, const char* name) {
    Group group;
    group.group_id = group_id;
    group.admin_id = admin_id;
    snprintf(group.name, sizeof(group.name), "%s", name);
    return group;
}


