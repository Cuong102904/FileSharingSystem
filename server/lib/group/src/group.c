#include "../include/group.h"

const char* status_to_string(MemberStatus status){
    return status == STATUS_MEMBER ? "member" : "pending";
}