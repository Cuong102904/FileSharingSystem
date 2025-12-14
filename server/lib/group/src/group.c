#include "group.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define GROUPS_FILE "groups.txt"

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

int append_group_to_file(Group group){
    FILE* fp = fopen(GROUPS_FILE, "a");
    if(fp == NULL){
        perror("Cannot open groups.txt");
        return 0;
    }

    fprintf(fp, "%d %d %s\n", group.group_id, group.admin_id, group.name);
    fclose(fp);

    return 1; // Append successful
}



