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
      printf("Error or disconnect during file receive\n");
      fclose(file);
      return -1;
    }

    fwrite(buffer, 1, bytes_received, file);
    total_received += bytes_received;
  }

  fclose(file);

  printf("File received successfully: %s (%ld bytes)\n", save_path,
         total_received);
  return total_received;
}
