#ifndef GROUP_REPO_H
#define GROUP_REPO_H
#include <pthread.h>
#include "group.h"

#define GROUP_DB "../../../../database/groups.txt"

extern pthread_mutex_t group_db_mutex;

extern int start_group_id;

int group_save(const Group *group);
int group_delete(int group_id);
int group_find_by_id(const int group_id);
int group_find_by_name(const char* group_name);
void group_list_all();

#endif