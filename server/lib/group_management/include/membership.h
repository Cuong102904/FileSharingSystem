#ifndef MEMBERSHIP_H
#define MEMBERSHIP_H

#include <stdbool.h>

#define MEMBERSHIP_DB "database/group_members.txt"

/**
 * @brief Check if user is member of group
 * @param group_name Group name
 * @param username Username
 * @return true if member, false otherwise
 */
bool membership_check(const char *group_name, const char *username);

#endif // MEMBERSHIP_H
