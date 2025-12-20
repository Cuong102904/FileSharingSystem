#ifndef GROUP_H
#define GROUP_H

typedef enum{
    STATUS_PENDING,
    STATUS_MEMBER
}MemberStatus;

typedef struct {
    char group_name[256];
    char member_name[256];
    int isAdmin;
    MemberStatus status;
} Group;

const char* status_to_string(MemberStatus);
Group create_group(const char* group_name, const char* member_name, int isAdmin, MemberStatus status);

#endif