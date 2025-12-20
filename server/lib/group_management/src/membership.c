#include "../include/membership.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

// Mutex for file access
static pthread_mutex_t membership_mutex = PTHREAD_MUTEX_INITIALIZER;

bool membership_check(const char *group_name, const char *username) {
  if (!group_name || !username) {
    return false;
  }

  pthread_mutex_lock(&membership_mutex);

  FILE *fp = fopen(MEMBERSHIP_DB, "r");
  if (!fp) {
    pthread_mutex_unlock(&membership_mutex);
    return false; // File doesn't exist = no members
  }

  char line[512];
  char stored_group[256];
  char stored_user[256];
  bool found = false;

  while (fgets(line, sizeof(line), fp)) {
    // Parse: "group_name username"
    if (sscanf(line, "%255s %255s", stored_group, stored_user) == 2) {
      if (strcmp(stored_group, group_name) == 0 &&
          strcmp(stored_user, username) == 0) {
        found = true;
        break;
      }
    }
  }

  fclose(fp);
  pthread_mutex_unlock(&membership_mutex);

  return found;
}
