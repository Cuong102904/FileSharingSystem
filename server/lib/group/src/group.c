#include "group.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define GROUPS_FILE "database/groups.txt"

Group create_group(int group_id, int admin_id, const char* name){
    Group new_group;
    new_group.group_id = group_id;
    new_group.admin_id = admin_id;

    if(name == NULL){
        new_group.name[0] = '\0';
    }
    else{
        snprintf(new_group.name, sizeof(new_group.name), "%s", name);
    }
    return new_group;
}

int append_group_to_file(const Group group) {
    FILE* fp = fopen(GROUPS_FILE, "a");
    if (fp == NULL) {
        perror("Cannot open groups.txt");
        return -1;
    }

    if (fprintf(fp, "%d %d %s\n",
        group.group_id,
        group.admin_id,
        group.name) < 0) {
        perror("Failed to write to groups.txt");
        fclose(fp);
        return -1;
    }

    // Close to flush contents before returning
    fclose(fp);
    return 0;
}

int list_groups(void){
    FILE* fp = fopen(GROUPS_FILE, "r");
    if (fp == NULL) {
        perror("Cannot open groups.txt");
        return -1;
    }

    char line[512];
    int line_no = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_no++;
        // Expected format: "<group_id> <admin_id> <name>"
        Group g;
        char name_buf[sizeof(g.name)];

        int n = sscanf(line, "%d %d %255s", &g.group_id, &g.admin_id, name_buf);
        if (n != 3) {
            fprintf(stderr, "Malformed line %d in groups.txt\n", line_no);
            continue;
        }
        snprintf(g.name, sizeof(g.name), "%s", name_buf);

        printf("Group ID: %d, Admin ID: %d, Name: %s\n", g.group_id, g.admin_id, g.name);
    }

    fclose(fp);
    return 0;
}






