#include "../include/group.h"
#include "../include/group_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int find_group_by_name(const char *group_name) {
  FILE *file = fopen(GROUP_DB, "r");
  if (file == NULL) {
    perror("Cannot open groups.txt");
    return -1;
  }

  char line[512];
  char stored_group_name[256];

  while (fgets(line, sizeof(line), file)) {
    sscanf(line, "%s", stored_group_name);
    if (strcmp(stored_group_name, group_name) == 0) {
      fclose(file);
      return 1; // found
    }
  }

  fclose(file);
  return 0; // not found
}

char *group_list_all_by_user(const char *member_name) {
  char *res = (char *)malloc(1024 * sizeof(char));
  res[0] = '\0'; // Initialize as empty string

  pthread_mutex_lock(&group_db_mutex);
  FILE *file = fopen(GROUP_DB, "r");
  if (file == NULL) {
    perror("Cannot open groups.txt");
    pthread_mutex_unlock(&group_db_mutex);
    return res; // exit the function if the file cannot be opened
  }

  strcat(res, member_name);
  strcat(res, "'s groups:\n");
  char line[512];

  while (fgets(line, sizeof(line), file)) {
    Group group;
    char status_str[20];
    sscanf(line, "%s %s %d %s", group.group_name, group.member_name,
           &group.isAdmin, status_str);
    if (strcmp(group.member_name, member_name) == 0 &&
        strcmp(status_str, "member") == 0) {
      char group_info[512];
      sprintf(group_info, "Group: %s\n", group.group_name);
      strcat(res, group_info);
    }
  }

  if (strcmp(res, member_name) == 0) {
    strcat(res, "No groups found for this user.\n");
  }

  fclose(file);
  pthread_mutex_unlock(&group_db_mutex);
  return res;
}

// Check if a user is a member of a specific group
// Returns: 1 if user is a member with STATUS_MEMBER, 0 otherwise
int user_is_group_member(const char *username, const char *group_name) {
  if (username == NULL || group_name == NULL) {
    return 0;
  }

  pthread_mutex_lock(&group_db_mutex);
  FILE *file = fopen(GROUP_DB, "r");
  if (file == NULL) {
    perror("Cannot open groups.txt");
    pthread_mutex_unlock(&group_db_mutex);
    return 0;
  }

  char line[512];
  int is_member = 0;

  while (fgets(line, sizeof(line), file)) {
    Group group;
    char status_str[20];
    sscanf(line, "%s %s %d %s", group.group_name, group.member_name,
           &group.isAdmin, status_str);

    // Check if this line matches both username and group_name, and status is
    // "member"
    if (strcmp(group.member_name, username) == 0 &&
        strcmp(group.group_name, group_name) == 0 &&
        strcmp(status_str, "member") == 0) {
      is_member = 1;
      break;
    }
  }

  fclose(file);
  pthread_mutex_unlock(&group_db_mutex);
  return is_member;
}