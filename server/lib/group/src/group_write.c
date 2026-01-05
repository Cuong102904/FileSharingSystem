#include "../include/group_repo.h"
#include "../include/group.h"
#include <stdio.h>

int group_create(const char *group_name, const char *user_name){
    pthread_mutex_lock(&group_db_mutex);

    FILE* file = fopen(GROUP_DB, "a");
    if(file == NULL){
        perror("Cannot opened groups.txt");
        pthread_mutex_unlock(&group_db_mutex);
        return -1;
    }
    if(find_group_by_name(group_name) == 1){
        fclose(file);
        pthread_mutex_unlock(&group_db_mutex);
        return GROUP_REPO_ERR_EXISTS;
    }
    // Format: group_name member_name role
    // Creator is owner
    fprintf(file, "%s %s %s\n", group_name, user_name, role_to_string(ROLE_OWNER));
    fclose(file);
    pthread_mutex_unlock(&group_db_mutex);
    return GROUP_REPO_OK;
}

// Add user as pending member to group
int group_add_pending_member(const char *group_name, const char *username) {
    pthread_mutex_lock(&group_db_mutex);

    FILE* file = fopen(GROUP_DB, "a");
    if(file == NULL){
        perror("Cannot open groups.txt");
        pthread_mutex_unlock(&group_db_mutex);
        return GROUP_REPO_ERR_IO;
    }

    // Format: group_name member_name role
    fprintf(file, "%s %s %s\n", group_name, username, role_to_string(ROLE_PENDING));
    fclose(file);
    pthread_mutex_unlock(&group_db_mutex);
    return GROUP_REPO_OK;
}

