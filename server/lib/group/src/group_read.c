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

char* group_list_all_by_user(const char* member_name){
    char *res = (char*)malloc(1024 * sizeof(char));
    res[0] = '\0'; // Initialize as empty string

    pthread_mutex_lock(&group_db_mutex);
    FILE* file = fopen(GROUP_DB, "r");
    if(file == NULL){
        perror("Cannot open groups.txt");
        pthread_mutex_unlock(&group_db_mutex);
        return res;
    }

    strcat(res, member_name);
    strcat(res, "'s groups:\n");
    char line[512];
    int found_groups = 0;

    while(fgets(line, sizeof(line), file)){
        char group_name[256], username[256], role_str[20];
        // Format: group_name member_name role
        if(sscanf(line, "%s %s %s", group_name, username, role_str) == 3){
            // User is in group if role is owner or member (not pending)
            if(strcmp(username, member_name) == 0 &&
               (strcmp(role_str, "owner") == 0 || strcmp(role_str, "member") == 0)){
                char group_info[512];
                sprintf(group_info, "Group: %s\n", group_name);
                strcat(res, group_info);
                found_groups++;
            }
        }
    }

    if (found_groups == 0) {
        strcat(res, "No groups found for this user.\n");
    }

    fclose(file);
    pthread_mutex_unlock(&group_db_mutex);
    return res;
}

// Check if user is member of group (owner or member role)
int is_user_in_group(const char *group_name, const char *username) {
    FILE* file = fopen(GROUP_DB, "r");
    if(file == NULL){
        return 0;
    }

    char line[512];
    while(fgets(line, sizeof(line), file)){
        char g_name[256], u_name[256], role_str[20];
        if(sscanf(line, "%s %s %s", g_name, u_name, role_str) == 3){
            if(strcmp(g_name, group_name) == 0 && strcmp(u_name, username) == 0 &&
               (strcmp(role_str, "owner") == 0 || strcmp(role_str, "member") == 0)){
                fclose(file);
                return 1; // user is in group
            }
        }
    }

    fclose(file);
    return 0; // user not in group
}

// List all members of a group
char* group_list_members(const char *group_name) {
    char *res = (char*)malloc(2048 * sizeof(char));
    res[0] = '\0';

    pthread_mutex_lock(&group_db_mutex);
    FILE* file = fopen(GROUP_DB, "r");
    if(file == NULL){
        perror("Cannot open groups.txt");
        pthread_mutex_unlock(&group_db_mutex);
        strcpy(res, "ERROR Cannot read database");
        return res;
    }

    char line[512];
    int found = 0;

    while(fgets(line, sizeof(line), file)){
        char g_name[256], username[256], role_str[20];
        if(sscanf(line, "%s %s %s", g_name, username, role_str) == 3){
            if(strcmp(g_name, group_name) == 0){
                char member_info[512];
                sprintf(member_info, "%s <%s>\n", username, role_str);
                strcat(res, member_info);
                found++;
            }
        }
    }

    fclose(file);
    pthread_mutex_unlock(&group_db_mutex);

    if(found == 0) {
        strcpy(res, ""); // empty means group not found
    }

    return res;
}

// Check if user has pending request for group
int is_user_pending(const char *group_name, const char *username) {
    FILE* file = fopen(GROUP_DB, "r");
    if(file == NULL){
        return 0;
    }

    char line[512];
    while(fgets(line, sizeof(line), file)){
        char g_name[256], u_name[256], role_str[20];
        if(sscanf(line, "%s %s %s", g_name, u_name, role_str) == 3){
            if(strcmp(g_name, group_name) == 0 && strcmp(u_name, username) == 0 &&
               strcmp(role_str, "pending") == 0){
                fclose(file);
                return 1; // user has pending request
            }
        }
    }

    fclose(file);
    return 0;
}

// Check if user is owner of group
int is_user_owner(const char *group_name, const char *username) {
    FILE* file = fopen(GROUP_DB, "r");
    if(file == NULL){
        return 0;
    }

    char line[512];
    while(fgets(line, sizeof(line), file)){
        char g_name[256], u_name[256], role_str[20];
        if(sscanf(line, "%s %s %s", g_name, u_name, role_str) == 3){
            if(strcmp(g_name, group_name) == 0 && strcmp(u_name, username) == 0 &&
               strcmp(role_str, "owner") == 0){
                fclose(file);
                return 1; // user is owner
            }
        }
    }

    fclose(file);
    return 0;
}