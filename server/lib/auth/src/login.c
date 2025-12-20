#include "../include/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int authenticate_user(const char *username, const char *password) {
    User *user = get_user_by_username(username);

    if (user == NULL) {
        return AUTH_USER_NOT_FOUND;
    }

    int result;
    if (strcmp(user->password, password) == 0) {
        result = AUTH_SUCCESS;
    } else {
        result = AUTH_WRONG_PASSWORD;
    }

    free_user(user);
    return result;
}
