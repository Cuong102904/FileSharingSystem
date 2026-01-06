#include "../include/directory_ops.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int create_directory_recursive(const char *full_path) {
  char temp_path[512] = "";
  char path_copy[512];
  struct stat st;
  char *saveptr; // For thread-safe strtok_r

  strncpy(path_copy, full_path, sizeof(path_copy) - 1);
  path_copy[sizeof(path_copy) - 1] = '\0';

  char *token = strtok_r(path_copy, "/", &saveptr);
  while (token != NULL) {
    strcat(temp_path, token);
    strcat(temp_path, "/");

    if (stat(temp_path, &st) == -1) {
      if (mkdir(temp_path, 0755) != 0) {
        perror("Directory creation error");
        return -1;
      }
    }
    token = strtok_r(NULL, "/", &saveptr);
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

int is_file_path(const char *path) {
  // Extract basename (filename)
  char path_copy[512];
  strncpy(path_copy, path, sizeof(path_copy) - 1);
  path_copy[sizeof(path_copy) - 1] = '\0';

  char *base = basename(path_copy);

  // Check if contains '.' (extension)
  char *dot = strrchr(base, '.');

  // Must have extension and it's not just a dot at start (.hidden)
  return (dot != NULL && dot != base && *(dot + 1) != '\0');
}

int is_folder_path(const char *path) { return !is_file_path(path); }

int validate_path_type(const char *full_path, int expect_file) {
  struct stat st;

  if (stat(full_path, &st) != 0) {
    return -1; // Path doesn't exist
  }

  int is_file = S_ISREG(st.st_mode);
  int is_dir = S_ISDIR(st.st_mode);

  if (expect_file && !is_file) {
    return -1; // Expected file, got directory
  }

  if (!expect_file && !is_dir) {
    return -1; // Expected directory, got file
  }

  return 0; // Matches expectation
}
