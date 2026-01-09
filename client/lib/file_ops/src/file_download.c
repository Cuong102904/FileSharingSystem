#include "../include/file_download.h"
#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define CHUNK_SIZE 4096

int file_download(int client_socket, const char *group_name,
                  const char *server_path, const char *local_path) {
  char buffer[BUFFER_SIZE];
  char expanded_path[512];
  char final_path[512];
  long filesize = 0;

  // Expand ~ to home directory
  if (local_path[0] == '~') {
    const char *home = getenv("HOME");
    if (home) {
      snprintf(expanded_path, sizeof(expanded_path), "%s%s", home, local_path + 1);
    } else {
      strncpy(expanded_path, local_path, sizeof(expanded_path) - 1);
      expanded_path[sizeof(expanded_path) - 1] = '\0';
    }
  } else {
    strncpy(expanded_path, local_path, sizeof(expanded_path) - 1);
    expanded_path[sizeof(expanded_path) - 1] = '\0';
  }

  // Extract filename from server_path
  char server_path_copy[256];
  strncpy(server_path_copy, server_path, sizeof(server_path_copy) - 1);
  server_path_copy[sizeof(server_path_copy) - 1] = '\0';

  char *filename = basename(server_path_copy);

  // Check if expanded_path is a directory or file
  struct stat path_stat;
  int is_directory = 0;

  if (stat(expanded_path, &path_stat) == 0) {
    // Path exists, check if it's a directory
    is_directory = S_ISDIR(path_stat.st_mode);
  } else {
    // Path doesn't exist, check if it ends with '/'
    size_t len = strlen(expanded_path);
    if (len > 0 && expanded_path[len - 1] == '/') {
      is_directory = 1;
    }
  }

  // Construct final path
  if (is_directory) {
    // Remove trailing slash if present
    size_t len = strlen(expanded_path);
    if (len > 0 && expanded_path[len - 1] == '/') {
      snprintf(final_path, sizeof(final_path), "%.*s/%s", (int)(len - 1), expanded_path, filename);
    } else {
      snprintf(final_path, sizeof(final_path), "%s/%s", expanded_path, filename);
    }
  } else {
    // Use expanded_path as-is (assume it's a full file path)
    strncpy(final_path, expanded_path, sizeof(final_path) - 1);
    final_path[sizeof(final_path) - 1] = '\0';
  }

  // Send DOWNLOAD command
  snprintf(buffer, sizeof(buffer), "DOWNLOAD %s %s", group_name, server_path);
  if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
    perror("Failed to send DOWNLOAD command");
    return -1;
  }

  // Wait for server response
  int n = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
  if (n <= 0) {
    printf("Server disconnected or error\n");
    return -1;
  }
  buffer[n] = '\0';

  // Check response
  if (strncmp(buffer, "OK DOWNLOAD", 11) == 0) {
    // Parse filesize from response: "OK DOWNLOAD <filesize>"
    if (sscanf(buffer, "OK DOWNLOAD %ld", &filesize) != 1) {
      printf("Error parsing file size from server response\n");
      return -1;
    }
    printf("Downloading file... Size: %ld bytes\n", filesize);

    // Send READY ack to server
    if (send(client_socket, "READY", 5, 0) < 0) {
      perror("Failed to send READY ack");
      return -1;
    }
  } else if (strncmp(buffer, "ERROR", 5) == 0) {
    // Only print if it's a text error message
    // Sanitize buffer to only print printable characters
    for (int i = 0; i < n; i++) {
      if (buffer[i] == '\0' || !isprint((unsigned char)buffer[i])) {
        buffer[i] = ' ';
      }
    }
    buffer[n] = '\0';
    printf("Server error: %s\n", buffer);
    return -1;
  } else {
    printf("Unexpected server response (first 50 chars): %.50s\n", buffer);
    return -1;
  }

  // Open local file for writing using final_path
  FILE *file = fopen(final_path, "wb");
  if (!file) {
    perror("Failed to open local file for writing");
    printf("Attempted to save to: %s\n", final_path);
    return -1;
  }

  // Receive file data
  char chunk[CHUNK_SIZE];
  long total_received = 0;
  int bytes_received;

  while (total_received < filesize) {
    long remaining = filesize - total_received;
    int to_read = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;

    bytes_received = recv(client_socket, chunk, to_read, 0);
    if (bytes_received <= 0) {
      printf("Error receiving file data\n");
      fclose(file);
      return -1;
    }

    fwrite(chunk, 1, bytes_received, file);
    total_received += bytes_received;

    // Optional: Progress bar could go here
  }

  fclose(file);
  printf("Download complete: %ld bytes saved to %s\n", total_received,
         final_path);
  return 0;
}
