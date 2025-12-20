#include "../include/auth.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
char db_path[MAX_PATH_LEN] = "";

void auth_init(void) {
    pthread_mutex_init(&db_mutex, NULL);

    // Get executable path and compute absolute path to database
    char exe_path[MAX_PATH_LEN];

#ifdef __APPLE__
    uint32_t size = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &size) == 0) {
        // Get directory of executable (bin/)
        char *dir = dirname(exe_path);
        // Go up one level to project root, then append database path
        snprintf(db_path, MAX_PATH_LEN, "%s/../%s", dir, DB_FILE_RELATIVE);
    } else {
        // Fallback to relative path
        strncpy(db_path, DB_FILE_RELATIVE, MAX_PATH_LEN);
    }
#else
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
        char *dir = dirname(exe_path);
        snprintf(db_path, MAX_PATH_LEN, "%s/../%s", dir, DB_FILE_RELATIVE);
    } else {
        strncpy(db_path, DB_FILE_RELATIVE, MAX_PATH_LEN);
    }
#endif

    printf("Database path: %s\n", db_path);
}

void auth_cleanup(void) {
    pthread_mutex_destroy(&db_mutex);
}

int check_user_exists(const char *username) {
    pthread_mutex_lock(&db_mutex);

    FILE *file = fopen(db_path, "r");
    if (file == NULL) {
        pthread_mutex_unlock(&db_mutex);
        return 0;
    }

    char line[256];
    char stored_username[MAX_USERNAME_LEN];

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s", stored_username);
        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            pthread_mutex_unlock(&db_mutex);
            return 1;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&db_mutex);
    return 0;
}

User* get_user_by_username(const char *username) {
    pthread_mutex_lock(&db_mutex);

    FILE *file = fopen(db_path, "r");
    if (file == NULL) {
        pthread_mutex_unlock(&db_mutex);
        return NULL;
    }

    char line[256];
    char stored_username[MAX_USERNAME_LEN];
    char stored_password[MAX_PASSWORD_LEN];

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %s", stored_username, stored_password);
        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            pthread_mutex_unlock(&db_mutex);

            User *user = malloc(sizeof(User));
            if (user == NULL) return NULL;

            strcpy(user->username, stored_username);
            strcpy(user->password, stored_password);
            user->is_active = 1;
            return user;
        }
    }

    fclose(file);
    pthread_mutex_unlock(&db_mutex);
    return NULL;
}

void free_user(User *user) {
    if (user != NULL) {
        free(user);
    }
}
