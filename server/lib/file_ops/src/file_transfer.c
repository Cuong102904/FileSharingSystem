#include "../include/file_transfer.h"
#include "../../protocol/include/protocol.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define CHUNK_SIZE 4096
#define STORAGE_DIR "storage"

void handle_upload(int client_socket, const char *path,
                   const char *filesize_str) {
  long filesize = atol(filesize_str);
  char full_path[512];

  // Ensure storage directory exists
  struct stat st = {0};
  if (stat(STORAGE_DIR, &st) == -1) {
    mkdir(STORAGE_DIR, 0700);
  }

  // Construct full path
  snprintf(full_path, sizeof(full_path), "%s/%s", STORAGE_DIR, path);
  // Simple security check (prevent directory traversal)
  if (strstr(path, "..")) {
    send_response(client_socket, "ERROR Invalid filename");
    return;
  }

  FILE *file = fopen(full_path, "wb");
  if (!file) {
    perror("File open error");
    send_response(client_socket, "ERROR Cannot create file");
    return;
  }

  // 1. Send Ready signal
  send_response(client_socket, RESP_OK_UPLOAD_READY);

  // 2. Receive chunks
  char buffer[CHUNK_SIZE];
  long total_received = 0;
  int bytes_received;

  while (total_received < filesize) {
    long remaining = filesize - total_received;
    int to_read = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;

    bytes_received = recv(client_socket, buffer, to_read, 0);
    if (bytes_received <= 0) {
      printf("Error or disconnect during upload\n");
      break;
    }

    fwrite(buffer, 1, bytes_received, file);
    total_received += bytes_received;
  }

  fclose(file);

  // 3. Send Complete signal
  if (total_received == filesize) {
    printf("File received successfully: %s (%ld bytes)\n", full_path,
           total_received);
    send_response(client_socket, RESP_OK_UPLOAD_COMPLETE);
  } else {
    printf("Upload incomplete/error. Expected %ld, got %ld\n", filesize,
           total_received);
    // Might not be able to send response if socket closed, but try
    send_response(client_socket, "ERROR Upload incomplete");
  }
}
