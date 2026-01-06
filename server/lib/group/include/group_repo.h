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

// Check if user is member of group (owner or member role)
int is_user_in_group(const char *group_name, const char *username);

// List all members of a group (returns allocated string, caller must free)
char* group_list_members(const char *group_name);

// Check if user has pending request for group
int is_user_pending(const char *group_name, const char *username);

// Add user as pending member to group
int group_add_pending_member(const char *group_name, const char *username);

// Check if user is owner of group
int is_user_owner(const char *group_name, const char *username);

// Approve pending member (change role from pending to member)
int group_approve_member(const char *group_name, const char *username);

// Check if user has been invited (invited role)
int is_user_invited(const char *group_name, const char *username);

// Invite user to group (add with invited role)
int group_invite_user(const char *group_name, const char *username);

// Accept invite (change role from invited to member)
int group_accept_invite(const char *group_name, const char *username);

// Reject invite (remove invited entry)
int group_reject_invite(const char *group_name, const char *username);

// Leave group (remove member from a group)
int group_leave(const char *group_name, const char *username);

#endif