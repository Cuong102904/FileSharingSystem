#include "../include/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int authenticate_user(const char *username, const char *password) {
    UserDetails *userDetails = get_user_by_username(username);

    if (userDetails == NULL) {
        return AUTH_USER_NOT_FOUND;
    }

    int result;
    if (strcmp(userDetails->password, password) == 0) {
        result = AUTH_SUCCESS;
    } else {
        result = AUTH_WRONG_PASSWORD;
    }

    free_user(userDetails);
    return result;
}
