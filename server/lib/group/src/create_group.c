#include "group.h"
#include "User.h"
#include <stdio.h>
#include <string.h>

pthread_mutex_t group_db_mutex = PTHREAD_MUTEX_INITIALIZER;

Group create_group(int group_id, int admin_id, const char* name) {
    Group group;
    group.group_id = group_id;
    group.admin_id = admin_id;
    snprintf(group.name, sizeof(group.name), "%s", name);
    return group;
}

void group_init(void) {
    pthread_mutex_init(&group_db_mutex, NULL);



    Group group = new_group();
}