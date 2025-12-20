#include "../include/group_repo.h"
#include "../include/group.h"
#include <stdio.h>

int group_save(const Group* group) {
    printf("Saving group ...\n");
    pthread_mutex_lock(&group_db_mutex);
    // Critical section: perform database write operations here
    FILE *fp = fopen(GROUP_DB, "a");
    if (fp != NULL) {
        fprintf(fp, "%d,%d,%s\n", group->group_id, group->admin_id, group->name);
        fclose(fp);
        printf("Group saved: ID=%d, AdminID=%d, Name=%s\n", group->group_id, group->admin_id, group->name);
    } else {
        printf("Error opening group database file for writing.\n");
        return -1; // Error
    }
    pthread_mutex_unlock(&group_db_mutex);
    return 0; // Success
}

int group_delete(int group_id) {
    printf("Deleting group ...\n");
    pthread_mutex_lock(&group_db_mutex);
    // Critical section: perform database delete operations here
    FILE *fp = fopen(GROUP_DB, "r");
    FILE *temp_fp = fopen("temp_groups.txt", "w");
    int found = 0;
    if (fp != NULL && temp_fp != NULL) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            int id, admin_id;
            char name[256];
            if (sscanf(line, "%d,%d,%255[^\n]", &id, &admin_id, name) == 3) {
                if (id == group_id) {
                    found = 1; // Group found and will be deleted
                    continue; // Skip writing this line to temp file
                }
            }
            fputs(line, temp_fp); // Write to temp file
        }
        fclose(fp);
        fclose(temp_fp);
        remove(GROUP_DB);
        rename("temp_groups.txt", GROUP_DB);
        if (found) {
            printf("Group with ID %d deleted.\n", group_id);
        } else {
            printf("Group with ID %d not found.\n", group_id);
        }
    } else {
        printf("Error opening group database file for deletion.\n");
        return -1; // Error
    }
    pthread_mutex_unlock(&group_db_mutex);
    return found; // Return 1 if deleted, 0 if not found
}
