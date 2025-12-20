#include "../include/file_upload.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define CHUNK_SIZE 4096

int file_upload(int client_socket, const char *group_name, const char *filepath,
                const char *server_path) {
  // Open file for reading
  FILE *file = fopen(filepath, "rb");
  if (!file) {
    printf("Error: File '%s' not found.\n", filepath);
    return -1;
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  long filesize = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Send UPLOAD command with group, client path, and server path (no size)
  char command[BUFFER_SIZE];
  snprintf(command, sizeof(command), "UPLOAD %s %s %s", group_name, filepath,
           server_path);
  if (send(client_socket, command, strlen(command), 0) < 0) {
    printf("Error: Failed to send UPLOAD command\n");
    fclose(file);
    return -1;
  }

  // Wait for server's READY response
  char buffer[BUFFER_SIZE];
  int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
  if (bytes_received <= 0) {
    printf("Error: No response from server\n");
    fclose(file);
    return -1;
  }
  buffer[bytes_received] = 0;

  // Check if server is ready
  if (strncmp(buffer, "OK UPLOAD_READY", 15) != 0) {
    printf("Server rejected upload: %s\n", buffer);
    fclose(file);
    return -1;
  }

  // Send file size as binary long
  if (send(client_socket, &filesize, sizeof(filesize), 0) < 0) {
    printf("Error: Failed to send file size\n");
    fclose(file);
    return -1;
  }

  printf("Uploading %ld bytes...\n", filesize);

  // Stream file data in chunks
  char chunk[CHUNK_SIZE];
  int bytes_read;
  long total_sent = 0;

  while ((bytes_read = fread(chunk, 1, CHUNK_SIZE, file)) > 0) {
    if (send(client_socket, chunk, bytes_read, 0) < 0) {
      printf("Error: Failed to send chunk\n");
      fclose(file);
      return -1;
    }
    total_sent += bytes_read;
  }

  fclose(file);
  printf("Upload complete: %ld bytes sent\n", total_sent);

  // Wait for server's COMPLETE confirmation
  bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
  if (bytes_received > 0) {
    buffer[bytes_received] = 0;
    printf("Server response: %s\n", buffer);
  }

  return 0;
}
