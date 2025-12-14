#ifndef GROUP_H
#define GROUP_H
#include <stdio.h>
#include <stdlib.h>

#define MAX_USERS_PER_GROUP 32

typedef struct {
    int group_id; //group id
    char name[256]; //group name
    int user_count; // store number of users
    int admin_id; // store admin id
}Group;

Group create_group(int group_id, const char *name, int user_count, int admin_id);

#endif