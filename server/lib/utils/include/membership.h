#ifndef MEMBERSHIP_H
#define MEMBERSHIP_H

#include <stdbool.h>

#define GROUPS_DB "server/database/groups.txt"

// Check if user is an approved member of the group
bool membership_check(const char *group_name, const char *username);

#endif // MEMBERSHIP_H
