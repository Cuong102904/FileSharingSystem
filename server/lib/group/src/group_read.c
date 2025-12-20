#include "../include/group_repo.h"
#include "../include/group.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int find_group_by_name(const char *group_name){
    FILE* file = fopen(GROUP_DB, "r");
    if(file == NULL){
        perror("Cannot open groups.txt");
        return -1;
    }

    char line[512];
    char stored_group_name[256];

    while(fgets(line, sizeof(line), file)){
        sscanf(line, "%s", stored_group_name);
        if(strcmp(stored_group_name, group_name) == 0){
            fclose(file);
            return 1; // found
        }
    }

    fclose(file);
    return 0; // not found
}

int group_list_all_by_user(const char* user_name){
    pthread_mutex_lock(&group_db_mutex);
    FILE* file = fopen(GROUP_DB, "r");
    if(file == NULL){
        perror("Cannot open groups.txt");
        pthread_mutex_unlock(&group_db_mutex);
        return -1; // exit the function if the file cannot be opened
    }

    char line[512];
    char user_name_in_file[256];
    char stored_group_name[256];

    while(fgets(line, sizeof(line), file)){
        sscanf(line, "%s %s", stored_group_name, user_name_in_file);
        if(strcmp(user_name_in_file, user_name) == 0) // read both group name and user name
            printf("%s\n", stored_group_name); // print each group name
    }

    fclose(file);
    pthread_mutex_unlock(&group_db_mutex);
    return GROUP_REPO_OK;
}