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
    fprintf(file, "%s %s %d %s\n", group_name, user_name, 1, status_to_string(STATUS_MEMBER));
    fclose(file);
    pthread_mutex_unlock(&group_db_mutex);
    return GROUP_REPO_OK;
}

