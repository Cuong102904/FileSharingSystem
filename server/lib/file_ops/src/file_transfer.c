#include "../include/file_transfer.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define CHUNK_SIZE 4096

long receive_file(int client_socket, const char *save_path, long filesize) {
  // Open file for writing
  FILE *file = fopen(save_path, "wb");
  if (!file) {
    perror("File open error");
    return -1;
  }

  // Receive chunks
  char buffer[CHUNK_SIZE];
  long total_received = 0;
  int bytes_received;

  while (total_received < filesize) {
    long remaining = filesize - total_received;
    int to_read = (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE;

    bytes_received = recv(client_socket, buffer, to_read, 0);
    if (bytes_received <= 0) {
      fclose(file);
      return -1;
    }

    size_t written = fwrite(buffer, 1, bytes_received, file);
    if (written != (size_t)bytes_received) {
      fclose(file);
      return -1;
    }

    total_received += bytes_received;
  }

  fclose(file);
  return total_received;
}

long send_file(int client_socket, const char *file_path) {
  FILE *file = fopen(file_path, "rb");
  if (!file) {
    perror("File open error");
    return -1;
  }

  char buffer[CHUNK_SIZE];
  long total_sent = 0;
  size_t bytes_read;

  while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, file)) > 0) {
    // Handle partial sends with retry loop
    size_t total_sent_this_chunk = 0;
    while (total_sent_this_chunk < bytes_read) {
      ssize_t sent = send(client_socket, buffer + total_sent_this_chunk,
                          bytes_read - total_sent_this_chunk, 0);
      if (sent < 0) {
        if (errno == EINTR) {
          continue; // Interrupted system call, retry
        }
        perror("Send error");
        fclose(file);
        return -1;
      }
      if (sent == 0) {
        // Connection closed by peer
        perror("Connection closed");
        fclose(file);
        return -1;
      }
      total_sent_this_chunk += (size_t)sent;
      total_sent += (size_t)sent;
    }
  }

  fclose(file);
  return total_sent;
}
