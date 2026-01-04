#include "../include/directory_ops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int create_directory_recursive(const char *full_path) {
  char temp_path[512] = "";
  char path_copy[512];
  struct stat st;

  strncpy(path_copy, full_path, sizeof(path_copy) - 1);
  path_copy[sizeof(path_copy) - 1] = '\0';

  char *token = strtok(path_copy, "/");
  while (token != NULL) {
    strcat(temp_path, token);
    strcat(temp_path, "/");

    if (stat(temp_path, &st) == -1) {
      if (mkdir(temp_path, 0755) != 0) {
        perror("Directory creation error");
        return -1;
      }
    }
    token = strtok(NULL, "/");
  }

  return 0;
}

int copy_path(const char *src_path, const char *dst_path) {
  char cmd[2048];
  snprintf(cmd, sizeof(cmd), "cp -r \"%s\" \"%s\"", src_path, dst_path);

  if (system(cmd) != 0) {
    return -1;
  }

  return 0;
}

int move_path(const char *src_path, const char *dst_path) {
  // Try atomic rename first
  if (rename(src_path, dst_path) == 0) {
    return 0;
  }

  // If rename fails (cross-filesystem), do copy + delete
  char cmd[2048];
  snprintf(cmd, sizeof(cmd), "cp -r \"%s\" \"%s\" && rm -rf \"%s\"", src_path,
           dst_path, src_path);

  if (system(cmd) != 0) {
    return -1;
  }

  return 0;
}

int check_parent_directory_exists(const char *dst_path) {
  char dst_dir[512];
  struct stat st;

  strncpy(dst_dir, dst_path, sizeof(dst_dir) - 1);
  dst_dir[sizeof(dst_dir) - 1] = '\0';

  char *last_slash = strrchr(dst_dir, '/');
  if (last_slash == NULL) {
    return 0; // No parent directory needed
  }

  *last_slash = '\0';

  // Check if parent directory exists
  if (stat(dst_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
    return -1;
  }

  return 0;
}
