#include "../include/auth.h"
#include "../include/user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t user_db_mutex = PTHREAD_MUTEX_INITIALIZER;

void auth_init(void) {
    pthread_mutex_init(&user_db_mutex, NULL);
}

void auth_cleanup(void) {
    pthread_mutex_destroy(&user_db_mutex);
}

int check_user_exists(const char *username) {
    pthread_mutex_lock(&user_db_mutex);

    FILE *file = fopen(DB_FILE, "r");
    if (file == NULL) {
        pthread_mutex_unlock(&user_db_mutex);
        return 0;
    }

    char line[256];
    char stored_username[MAX_USERNAME_LEN];

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s", stored_username);
        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            pthread_mutex_unlock(&user_db_mutex);
            return 1;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&user_db_mutex);
    return 0;
}

User* get_user_by_username(const char *username) {
    pthread_mutex_lock(&user_db_mutex);

    FILE *file = fopen(DB_FILE, "r");
    if (file == NULL) {
        pthread_mutex_unlock(&user_db_mutex);
        return NULL;
    }

    char line[256];
    int stored_user_id;
    char stored_username[MAX_USERNAME_LEN];
    char stored_password[MAX_PASSWORD_LEN];

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%d %s %s", &stored_user_id, stored_username, stored_password);
        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            pthread_mutex_unlock(&user_db_mutex);

            User *user = malloc(sizeof(User));
            if (user == NULL) return NULL;


            user->user_id = stored_user_id;
            strcpy(user->user_details.username, stored_username);
            strcpy(user->user_details.password, stored_password);
            user->user_details.is_active = 1;
            return user;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&user_db_mutex);
    return NULL;
}

void free_user(UserDetails *userDetails) {
    if (userDetails != NULL) {
        free(userDetails);
    }
}
