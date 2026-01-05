#include "../include/group.h"
#include <string.h>

const char* role_to_string(MemberRole role) {
    switch (role) {
        case ROLE_OWNER:   return "owner";
        case ROLE_MEMBER:  return "member";
        case ROLE_PENDING: return "pending";
        default:           return "unknown";
    }
}

MemberRole string_to_role(const char* str) {
    if (strcmp(str, "owner") == 0)   return ROLE_OWNER;
    if (strcmp(str, "member") == 0)  return ROLE_MEMBER;
    if (strcmp(str, "pending") == 0) return ROLE_PENDING;
    return ROLE_PENDING; // default
}