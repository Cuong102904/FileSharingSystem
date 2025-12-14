#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/server.h"
#include "../lib/auth/include/auth.h"
#include "../lib/protocol/include/protocol.h"
#include "../lib/session/include/session.h"

void server_init(void) {
  auth_init();
  session_init();
  printf("Server modules initialized.\n");
}

void server_cleanup(void) {
  auth_cleanup();
  session_cleanup();
  printf("Server modules cleaned up.\n");
}

void *handle_client(void *arg) {
  int client_socket = *(int *)arg;
  free(arg);

  char buffer[BUFFER_SIZE];
  int bytes_received;
  ParsedCommand cmd;

  printf("Client connected: socket %d\n", client_socket);

  while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) >
         0) {
    buffer[bytes_received] = '\0';

    // printf("Received from client %d: %s\n", client_socket, buffer); // Debug
    // print

    // Parse and handle command
    CommandType cmd_type = protocol_parse_command(buffer, &cmd);

    switch (cmd_type) {
    case CMD_REGISTER:
      handle_register(client_socket, cmd.arg1, cmd.arg2);
      break;
    case CMD_LOGIN:
      handle_login(client_socket, cmd.arg1, cmd.arg2);
      break;
    case CMD_LOGOUT:
      handle_logout(client_socket, cmd.arg1);
      break;
    case CMD_UPLOAD:
      handle_upload(client_socket, cmd.arg1, cmd.arg2, cmd.arg3);
      break;
    default:
      send_response(client_socket, RESP_ERR_UNKNOWN_CMD);
      break;
    }

    memset(buffer, 0, BUFFER_SIZE);
  }

  printf("Client disconnected: socket %d\n", client_socket);
  close(client_socket);

  return NULL;
}

int main() {
  int server_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  // Initialize all modules
  server_init();

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Allow socket reuse
  int opt = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // Configure server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // Bind socket
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  // Listen
  if (listen(server_socket, MAX_CLIENTS) < 0) {
    perror("Listen failed");
    close(server_socket);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", PORT);

  // Accept connections
  while (1) {
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr,
                           &client_addr_len);

    if (client_socket < 0) {
      perror("Accept failed");
      continue;
    }

    // Create new thread for client
    pthread_t thread_id;
    int *pclient = malloc(sizeof(int));
    *pclient = client_socket;

    if (pthread_create(&thread_id, NULL, handle_client, pclient) != 0) {
      perror("Thread creation failed");
      close(client_socket);
      free(pclient);
      continue;
    }

    pthread_detach(thread_id);
  }

  server_cleanup();
  close(server_socket);
  return 0;
}
