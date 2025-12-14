#include "group.h"
#include <string.h>

// Create a new group struct
Group create_group(int group_id, const char *name, int user_count, int admin_id) {
    Group g;
    g.group_id = group_id;
    strcpy(g.name, name);
    g.user_count = user_count;
    g.admin_id = admin_id;
    return g;
}

void save_group(FILE *fp, Group g){
    fprintf(fp, "%d %s %d %d", g.group_id, g.name, g.user_count, g.admin_id);
}

// Adds a group line to the bottom of groups.txt
int append_group(const char *filename, Group g) {
    FILE *fp = fopen(filename, "a");
    if (!fp) return 0; // could not open file
    save_group(fp, g);
    fclose(fp);
    return 1;
}