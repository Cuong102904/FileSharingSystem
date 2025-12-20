#include "../include/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int register_user(const char *username, const char *password) {
    pthread_mutex_lock(&db_mutex);

    // Check if username already exists
    FILE *file = fopen(db_path, "r");
    if (file != NULL) {
        char line[256];
        char stored_username[MAX_USERNAME_LEN];

        while (fgets(line, sizeof(line), file)) {
            sscanf(line, "%s", stored_username);
            if (strcmp(stored_username, username) == 0) {
                fclose(file);
                pthread_mutex_unlock(&db_mutex);
                return AUTH_USER_EXISTS;
            }
        }
        fclose(file);
    }

    // Register new user
    file = fopen(db_path, "a");
    if (file == NULL) {
        pthread_mutex_unlock(&db_mutex);
        return AUTH_DB_ERROR;
    }

    fprintf(file, "%s %s\n", username, password);
    fclose(file);

    pthread_mutex_unlock(&db_mutex);
    return AUTH_SUCCESS;
}
