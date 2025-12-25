#ifndef GROUP_REPO_H
#define GROUP_REPO_H
#include <pthread.h>
#include <stdlib.h>
#include "group.h"

#define GROUP_DB "database/groups.txt"

typedef enum {
    GROUP_REPO_OK = 0,          // operation successful
    GROUP_REPO_ERR_IO,          // file open/read/write error
    GROUP_REPO_ERR_EXISTS,      // group already exists
    GROUP_REPO_ERR_NOT_FOUND,   // group not found
    GROUP_REPO_ERR_INVALID,     // invalid input data
    GROUP_REPO_ERR_LOCK,        // mutex lock/unlock failure
    GROUP_REPO_ERR_UNKNOWN      // unexpected error
} GroupRepoStatus;

extern pthread_mutex_t group_db_mutex;

int group_create(const char *group_name, const char *user_name);
char* group_list_all_by_user(const char* member_name);
int find_group_by_name(const char *group_name);

#endif