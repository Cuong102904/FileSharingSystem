#include "../include/file_download.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define CHUNK_SIZE 4096

int file_download(int client_socket, const char *group_name,
                  const char *server_path, const char *local_path) {
  char buffer[BUFFER_SIZE];
  long filesize = 0;

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
  } else {
    printf("Server error: %s\n", buffer);
    return -1;
  }

  // Open local file for writing
  FILE *file = fopen(local_path, "wb");
  if (!file) {
    perror("Failed to open local file for writing");
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
         local_path);
  return 0;
}
