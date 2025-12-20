#include "../include/group_repo.h"
#include "../include/group.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int group_find_by_id(const int group_id) {
    printf("Finding group by ID ...\n");
    pthread_mutex_lock(&group_db_mutex);
    // Critical section: perform database read operations here
    FILE *fp = fopen(GROUP_DB, "r");
    if (fp != NULL) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            int id, admin_id;
            char name[256];
            if (sscanf(line, "%d,%d,%255[^\n]", &id, &admin_id, name) == 3) {
                if (id == group_id) {
                    fclose(fp);
                    pthread_mutex_unlock(&group_db_mutex);
                    printf("Group found: ID=%d, AdminID=%d, Name=%s\n", id, admin_id, name);
                    return 1; // Group found
                }
            }
        }
        fclose(fp);
    } else {
        printf("Error opening group database file for reading.\n");
    }
    pthread_mutex_unlock(&group_db_mutex);
    printf("Group with ID %d not found.\n", group_id);
    return 0; // Group not found
}

int group_find_by_name(const char* group_name) {
    printf("Finding group by Name ...\n");
    pthread_mutex_lock(&group_db_mutex);
    // Critical section: perform database read operations here
    FILE *fp = fopen(GROUP_DB, "r");
    if (fp != NULL) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            int id, admin_id;
            char name[256];
            if (sscanf(line, "%d,%d,%255[^\n]", &id, &admin_id, name) == 3) {
                if (strcmp(name, group_name) == 0) {
                    fclose(fp);
                    pthread_mutex_unlock(&group_db_mutex);
                    printf("Group found: ID=%d, AdminID=%d, Name=%s\n", id, admin_id, name);
                    return 1; // Group found
                }
            }
        }
        fclose(fp);
    } else {
        printf("Error opening group database file for reading.\n");
    }
    pthread_mutex_unlock(&group_db_mutex);
    printf("Group with Name %s not found.\n", group_name);
    return 0; // Group not found
}

void group_list_all() {
    printf("Listing all groups ...\n");
    pthread_mutex_lock(&group_db_mutex);
    // Critical section: perform database read operations here
    FILE *fp = fopen(GROUP_DB, "r");
    if (fp != NULL) {
        char line[512];
        printf("Group List:\n");
        printf("ID\tAdminID\tName\n");
        printf("-------------------------\n");
        while (fgets(line, sizeof(line), fp)) {
            int id, admin_id;
            char name[256];
            if (sscanf(line, "%d,%d,%255[^\n]", &id, &admin_id, name) == 3) {
                printf("%d\t%d\t%s\n", id, admin_id, name);
            }
        }
        fclose(fp);
    } else {
        printf("Error opening group database file for reading.\n");
    }
    pthread_mutex_unlock(&group_db_mutex);
    return;
}