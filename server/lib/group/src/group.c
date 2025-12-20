#include "../include/group.h"

const char* member_to_string(MemberStatus status){
    return status == STATUS_MEMBER ? "member" : "pending";
}