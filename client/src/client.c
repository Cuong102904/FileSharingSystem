#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Import file upload library
#include "../lib/file_ops/include/file_upload.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
  int client_socket;
  struct sockaddr_in server_addr;
  char buffer[BUFFER_SIZE];

  // Init socket
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Config IP address of server
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  // Change IP address
  if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    close(client_socket);
    exit(EXIT_FAILURE);
  }

  // Connect to server
  if (connect(client_socket, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) < 0) {
    perror("Connection failed");
    close(client_socket);
    exit(EXIT_FAILURE);
  }

  printf("Connected to server successfully!\n");
  printf("==================================\n");
  printf("Available commands:\n");
  printf("1. REGISTER <username> <password>\n");
  printf("2. LOGIN <username> <password>\n");
  printf("3. LOGOUT\n");
  printf("4. CREATE_GROUP <group_name>\n");
  printf("5. LIST_GROUPS\n");
  printf("6. UPLOAD <group_name> <local_path> <remote_path>\n");
  printf("7. QUIT (to exit)\n");
  printf("==================================\n\n");

  while (1) {
    printf("Enter command: ");
    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
      break;

    // Remove newline
    buffer[strcspn(buffer, "\n")] = 0;

    // Check QUIT command
    if (strcmp(buffer, "QUIT") == 0 || strcmp(buffer, "quit") == 0) {
      printf("Exiting...\n");
      break;
    }

    // Handle File Upload command
    if (strncmp(buffer, "UPLOAD", 6) == 0) {
      char group_name[256];
      char local_path[256];
      char remote_path[256];

      // Try to parse group, local_path, and remote_path
      int parsed = sscanf(buffer, "UPLOAD %s %s %s", group_name, local_path,
                          remote_path);

      if (parsed == 3) {
        file_upload(client_socket, group_name, local_path, remote_path);
      } else {
        printf("Usage: UPLOAD <group_name> <local_path> <remote_path>\n");
        printf("Example: UPLOAD group1 file.txt docs/\n");
      }
      continue;
    }

    // Send regular command to server
    send(client_socket, buffer, strlen(buffer), 0);

    // Receive the response form server
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_received <= 0) {
      printf("Server disconnected\n");
      break;
    }

    buffer[bytes_received] = '\0';
    printf("Server response: %s\n\n", buffer);

    // Check login response
    if (strncmp(buffer, "OK LOGIN", 8) == 0) {
      char username[256];
      sscanf(buffer, "OK LOGIN %s", username);
      printf("Logged in as: %s\n\n", username);
    }

    if (strncmp(buffer, "OK LOGOUT", 9) == 0) {
      printf("Logged out successfully.\n\n");
    }
  }

  close(client_socket);
  return 0;
}
