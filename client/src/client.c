#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Import file upload library
#include "../lib/file_ops/include/file_download.h"
#include "../lib/file_ops/include/file_upload.h"

#define PORT 8080
#define BUFFER_SIZE 1024

const char menu[] =
    "Connected to server successfully!\n"
    "==================================\n"
    "Available commands:\n"
    "1. REGISTER <username> <password>\n"
    "2. LOGIN <username> <password>\n"
    "3. LOGOUT\n"
    "4. CREATE_GROUP <group_name>\n"
    "5. LIST_GROUPS\n"
    "6. LIST_MEMBERS <group_name>\n"
    "7. KICK_MEMBER <group_name> <user_name>\n"
    "8. RESPOND_INVITE <group_name> <status>\n"
    "9. JOIN_REQ <group_name>\n"
    "10. APPROVE_JOIN <group_name> <user_name>\n"
    "11. INVITE_USER <group_name> <user_name>\n"
    "12. UPLOAD <group_name> <local_path> <remote_path>\n"
    "13. LEAVE_GROUP <group_name>\n"
    "14. DOWNLOAD <group_name> <path_on_server> <local_save_path>\n"
    "15. MKDIR <group_name> <path>\n"
    "16. LS <group_name> <path>\n"
    "17. COPYFILE <group_name> <source_file> <dest_file>\n"
    "18. COPYFOLDER <group_name> <source_folder> <dest_folder>\n"
    "19. MOVEFILE <group_name> <source_file> <dest_folder>/\n"
    "20. MOVEFOLDER <group_name> <source_folder> <dest_parent_folder>/\n"
    "21. DELETEFILE <group_name> <file_path>\n"
    "22. RENAMEFILE <group_name> <old_name> <new_name>\n"
    "23. DELETEFOLDER <group_name> <folder_path>\n"
    "24. RENAMEFOLDER <group_name> <old_name> <new_name>\n"
    "*. QUIT (to exit)\n"
    "==================================\n\n";

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
  while (1) {
    printf("%s", menu);
    printf("Enter command: ");
    if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
      break;

    // Remove newline
    buffer[strcspn(buffer, "\n")] = 0;

    // Validate input
    if (strlen(buffer) == 0) {
      printf("Input cannot be empty. Please try again.\n");
      continue;
    }

    // Check QUIT command
    if (strcmp(buffer, "QUIT") == 0 || strcmp(buffer, "quit") == 0) {
      printf("Exiting...\n");
      break;
    }

    // Handle File Upload command
    else if (strncmp(buffer, "UPLOAD", 6) == 0) {
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
    } else if (strncmp(buffer, "DOWNLOAD", 8) == 0) {
      char group_name[256];
      char server_path[256];
      char local_path[256];

      int parsed = sscanf(buffer, "DOWNLOAD %s %s %s", group_name, server_path,
                          local_path);

      if (parsed == 3) {
        file_download(client_socket, group_name, server_path, local_path);
      } else {
        printf("Usage: DOWNLOAD <group_name> <path_on_server> "
               "<local_save_path>\n");
        printf(
            "Example: DOWNLOAD 123 docs/file.txt /home/user/downloaded.txt\n");
      }
      continue;
    }

    // MKDIR command
    if (strncmp(buffer, "MKDIR", 5) == 0) {
      send(client_socket, buffer, strlen(buffer), 0);
      int n = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
      if (n > 0) {
        buffer[n] = '\0';
        printf("Server response: %s\n", buffer);
      }
      continue;
    }

    // List content in a folder
    if (strncmp(buffer, "LS", 2) == 0) {
      send(client_socket, buffer, strlen(buffer), 0);
      int n = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
      if(n > 0){
        buffer[n] = '\0';
        printf("Server response: %s\n", buffer);
      }
      continue;
    }

    // COPYFILE command
    if (strncmp(buffer, "COPYFILE", 8) == 0) {
      send(client_socket, buffer, strlen(buffer), 0);
      int n = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
      if (n > 0) {
        buffer[n] = '\0';
        printf("Server response: %s\n", buffer);
      }
      continue;
    }

    // COPYFOLDER command
    if (strncmp(buffer, "COPYFOLDER", 10) == 0) {
      send(client_socket, buffer, strlen(buffer), 0);
      int n = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
      if (n > 0) {
        buffer[n] = '\0';
        printf("Server response: %s\n", buffer);
      }
      continue;
    }

    // MOVEFILE command
    if (strncmp(buffer, "MOVEFILE", 8) == 0) {
      send(client_socket, buffer, strlen(buffer), 0);
      int n = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
      if (n > 0) {
        buffer[n] = '\0';
        printf("Server response: %s\n", buffer);
      }
      continue;
    }

    // MOVEFOLDER command
    if (strncmp(buffer, "MOVEFOLDER", 10) == 0) {
      send(client_socket, buffer, strlen(buffer), 0);
      int n = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
      if (n > 0) {
        buffer[n] = '\0';
        printf("Server response: %s\n", buffer);
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
