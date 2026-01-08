#include "../include/group.h"
#include "../include/group_repo.h"
#include <stdio.h>
#include <string.h>

int group_create(const char *group_name, const char *user_name) {
  pthread_rwlock_wrlock(&group_db_rwlock);

  FILE *file = fopen(GROUP_DB, "a");
  if (file == NULL) {
    perror("Cannot opened groups.txt");
    pthread_rwlock_unlock(&group_db_rwlock);
    return -1;
  }
  if (find_group_by_name(group_name) == 1) {
    fclose(file);
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_EXISTS;
  }
  // Format: group_name member_name role
  // Creator is owner
  fprintf(file, "%s %s %s\n", group_name, user_name,
          role_to_string(ROLE_OWNER));
  fclose(file);
  pthread_rwlock_unlock(&group_db_rwlock);
  return GROUP_REPO_OK;
}

// Add user as pending member to group
int group_add_pending_member(const char *group_name, const char *username) {
  pthread_rwlock_wrlock(&group_db_rwlock);

  FILE *file = fopen(GROUP_DB, "a");
  if (file == NULL) {
    perror("Cannot open groups.txt");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  // Format: group_name member_name role
  fprintf(file, "%s %s %s\n", group_name, username,
          role_to_string(ROLE_PENDING));
  fclose(file);
  pthread_rwlock_unlock(&group_db_rwlock);
  return GROUP_REPO_OK;
}

// Approve pending member (change role from pending to member)
int group_approve_member(const char *group_name, const char *username) {
  pthread_rwlock_wrlock(&group_db_rwlock);

  FILE *file = fopen(GROUP_DB, "r");
  if (file == NULL) {
    perror("Cannot open groups.txt");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  // Read all lines into memory
  char lines[1024][512];
  int line_count = 0;
  int found = 0;

  while (fgets(lines[line_count], sizeof(lines[line_count]), file) &&
         line_count < 1024) {
    char g_name[256], u_name[256], role_str[20];
    if (sscanf(lines[line_count], "%s %s %s", g_name, u_name, role_str) == 3) {
      // If this is the pending user we want to approve
      if (strcmp(g_name, group_name) == 0 && strcmp(u_name, username) == 0 &&
          strcmp(role_str, "pending") == 0) {
        // Replace the line with member role
        snprintf(lines[line_count], sizeof(lines[line_count]), "%s %s %s\n",
                 group_name, username, role_to_string(ROLE_MEMBER));
        found = 1;
      }
    }
    line_count++;
  }
  fclose(file);

  if (!found) {
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_NOT_FOUND;
  }

  // Write all lines back
  file = fopen(GROUP_DB, "w");
  if (file == NULL) {
    perror("Cannot open groups.txt for writing");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  for (int i = 0; i < line_count; i++) {
    fputs(lines[i], file);
  }

  fclose(file);
  pthread_rwlock_unlock(&group_db_rwlock);
  return GROUP_REPO_OK;
}

// Invite user to group (add with invited role)
int group_invite_user(const char *group_name, const char *username) {
  pthread_rwlock_wrlock(&group_db_rwlock);

  FILE *file = fopen(GROUP_DB, "a");
  if (file == NULL) {
    perror("Cannot open groups.txt");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  // Format: group_name member_name role
  fprintf(file, "%s %s %s\n", group_name, username,
          role_to_string(ROLE_INVITED));
  fclose(file);
  pthread_rwlock_unlock(&group_db_rwlock);
  return GROUP_REPO_OK;
}

// Accept invite (change role from invited to member)
int group_accept_invite(const char *group_name, const char *username) {
  pthread_rwlock_wrlock(&group_db_rwlock);

  FILE *file = fopen(GROUP_DB, "r");
  if (file == NULL) {
    perror("Cannot open groups.txt");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  // Read all lines into memory
  char lines[1024][512];
  int line_count = 0;
  int found = 0;

  while (fgets(lines[line_count], sizeof(lines[line_count]), file) &&
         line_count < 1024) {
    char g_name[256], u_name[256], role_str[20];
    if (sscanf(lines[line_count], "%s %s %s", g_name, u_name, role_str) == 3) {
      // If this is the invited user we want to accept
      if (strcmp(g_name, group_name) == 0 && strcmp(u_name, username) == 0 &&
          strcmp(role_str, "invited") == 0) {
        // Replace the line with member role
        snprintf(lines[line_count], sizeof(lines[line_count]), "%s %s %s\n",
                 group_name, username, role_to_string(ROLE_MEMBER));
        found = 1;
      }
    }
    line_count++;
  }
  fclose(file);

  if (!found) {
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_NOT_FOUND;
  }

  // Write all lines back
  file = fopen(GROUP_DB, "w");
  if (file == NULL) {
    perror("Cannot open groups.txt for writing");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  for (int i = 0; i < line_count; i++) {
    fputs(lines[i], file);
  }

  fclose(file);
  pthread_rwlock_unlock(&group_db_rwlock);
  return GROUP_REPO_OK;
}

// Reject invite (remove invited entry)
int group_reject_invite(const char *group_name, const char *username) {
  pthread_rwlock_wrlock(&group_db_rwlock);

  FILE *file = fopen(GROUP_DB, "r");
  if (file == NULL) {
    perror("Cannot open groups.txt");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  // Read all lines into memory, skip the one to remove
  char lines[1024][512];
  int line_count = 0;
  int found = 0;

  while (fgets(lines[line_count], sizeof(lines[line_count]), file) &&
         line_count < 1024) {
    char g_name[256], u_name[256], role_str[20];
    if (sscanf(lines[line_count], "%s %s %s", g_name, u_name, role_str) == 3) {
      // If this is the invited entry to remove, skip it
      if (strcmp(g_name, group_name) == 0 && strcmp(u_name, username) == 0 &&
          strcmp(role_str, "invited") == 0) {
        found = 1;
        continue; // Don't increment line_count, effectively removing this line
      }
    }
    line_count++;
  }
  fclose(file);

  if (!found) {
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_NOT_FOUND;
  }

  // Write all lines back (without the removed entry)
  file = fopen(GROUP_DB, "w");
  if (file == NULL) {
    perror("Cannot open groups.txt for writing");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  for (int i = 0; i < line_count; i++) {
    fputs(lines[i], file);
  }

  fclose(file);
  pthread_rwlock_unlock(&group_db_rwlock);
  return GROUP_REPO_OK;
}

int group_remove_member(const char *group_name, const char *username) {
  pthread_rwlock_wrlock(&group_db_rwlock);

  FILE *file = fopen(GROUP_DB, "r");
  if (file == NULL) {
    perror("Cannot open groups.txt");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  char lines[1024][512];
  int line_count = 0;
  int found = 0;

  while (fgets(lines[line_count], sizeof(lines[line_count]), file) &&
         line_count < 1024) {
    char g_name[256], u_name[256], role_str[20];
    if (sscanf(lines[line_count], "%s %s %s", g_name, u_name, role_str) == 3) {
      // If this is the user that needs to be remove, skip
      if (strcmp(g_name, group_name) == 0 && strcmp(u_name, username) == 0) {
        found = 1;
        continue; // Don't increment line_count, effectively removing this line
      }
    }
    line_count++;
  }
  fclose(file);

  if (!found) {
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_NOT_FOUND;
  }

  // Write all lines back (without the removed entry)
  file = fopen(GROUP_DB, "w");
  if (file == NULL) {
    perror("Cannot open groups.txt for writing");
    pthread_rwlock_unlock(&group_db_rwlock);
    return GROUP_REPO_ERR_IO;
  }

  for (int i = 0; i < line_count; i++) {
    fputs(lines[i], file);
  }

  fclose(file);
  pthread_rwlock_unlock(&group_db_rwlock);
  return GROUP_REPO_OK;
}
