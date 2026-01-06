#ifndef GROUP_H
#define GROUP_H

// Member roles: owner (người tạo group), member (thành viên), pending (chờ duyệt), invited (được mời)
typedef enum {
    ROLE_OWNER,
    ROLE_MEMBER,
    ROLE_PENDING,
    ROLE_INVITED
} MemberRole;

// Group membership record (1 row in database)
// Format: group_name member_name role
typedef struct {
    char group_name[256];
    char member_name[256];
    MemberRole role;
} GroupMembership;

// Convert role to string
const char* role_to_string(MemberRole role);

// Parse role from string
MemberRole string_to_role(const char* str);

#endif