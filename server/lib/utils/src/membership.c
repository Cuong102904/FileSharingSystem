#include "../include/membership.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static pthread_mutex_t groups_mutex = PTHREAD_MUTEX_INITIALIZER;

bool membership_check(const char *group_name, const char *username) {
  if (!group_name || !username)
    return false;

  pthread_mutex_lock(&groups_mutex);

  FILE *fp = fopen(GROUPS_DB, "r");
  if (!fp) {
    pthread_mutex_unlock(&groups_mutex);
    return false;
  }

  char line[512];
  char stored_group[256];
  int is_admin;
  char stored_user[256];
  char status[32];
  bool found = false;

  while (fgets(line, sizeof(line), fp)) {
    // Parse: group_name admin member_name status
    // e.g. "Group1 1 duongtt member"
    if (sscanf(line, "%255s %d %255s %31s", stored_group, &is_admin,
               stored_user, status) == 4) {
      if (strcmp(stored_group, group_name) == 0 &&
          strcmp(stored_user, username) == 0 && strcmp(status, "member") == 0) {
        found = true;
        break;
      }
    }
  }

  fclose(fp);
  pthread_mutex_unlock(&groups_mutex);

  return found;
}
