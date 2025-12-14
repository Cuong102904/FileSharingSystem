/*
 * File: group.h
 * ----------------
 * This header file defines the data structure and function prototypes
 * used for managing groups in the File Sharing System.
 *
 * A group represents a collection of users with exactly one administrator
 * who is responsible for managing the group. Each group is uniquely
 * identified by a group ID and has a human-readable name.
 *
 * The Group structure stores basic group information, while the provided
 * function prototypes allow the creation and initialization of group
 * objects.
 */

#ifndef GROUP_H
#define GROUP_H

/*
 * Struct: Group
 * ----------------
 * Represents a user group in the system.
 *
 * group_id : unique identifier for the group
 * admin_id : user ID of the group administrator
 * name     : name of the group
 */
typedef struct {
    int group_id;
    int admin_id;
    char name[256];
} Group;

/*
 * Function: create_group
 * ----------------------
 * Creates and initializes a new Group instance.
 *
 * group_id : unique identifier for the group
 * admin_id : user ID of the group administrator
 * name     : name of the group
 *
 * returns  : a Group structure with the provided information
 */
Group create_group(int group_id, int admin_id, const char* name);


int append_group_to_file(Group group);
int list_groups(void);

#endif