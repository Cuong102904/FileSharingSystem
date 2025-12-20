#ifndef USER_H
#define USER_H

#include "auth.h"   

typedef struct{
    int user_id;
    UserDetails user_details;
}User;

int get_user_id(char* username);

#endif