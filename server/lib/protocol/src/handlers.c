#include "../../auth/include/auth.h"
#include "../../file_ops/include/file_transfer.h"
#include "../../session/include/session.h"
#include "../include/protocol.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

void send_response(int client_socket, const char *response) {
  send(client_socket, response, strlen(response), 0);
}

void handle_register(int client_socket, const char *username,
                     const char *password) {
  char response[BUFFER_SIZE];

  int result = register_user(username, password);

  switch (result) {
  case AUTH_SUCCESS:
    strcpy(response, RESP_OK_REGISTER);
    break;
  case AUTH_USER_EXISTS:
    strcpy(response, RESP_ERR_ACCOUNT_EXISTS);
    break;
  default:
    strcpy(response, RESP_ERR_DB_ERROR);
    break;
  }

  send_response(client_socket, response);
}

void handle_login(int client_socket, const char *username,
                  const char *password) {
  char *session_id = handle_login_with_session(client_socket, username, password);
  if (session_id != NULL) {
    free(session_id);
  }
}

char *handle_login_with_session(int client_socket, const char *username,
                                const char *password) {
  char response[BUFFER_SIZE];
  char *session_id_to_return = NULL;

  int result = authenticate_user(username, password);

  switch (result) {
  case AUTH_SUCCESS: {
    char *session_id = session_create(username);
    if (session_id != NULL) {
      snprintf(response, BUFFER_SIZE, "%s %s", RESP_OK_LOGIN, session_id);
      session_id_to_return = session_id; // Return session_id (caller must free)
    } else {
      strcpy(response, RESP_ERR_SERVER_FULL);
    }
    break;
  }
  case AUTH_WRONG_PASSWORD:
    strcpy(response, RESP_ERR_WRONG_PASSWORD);
    break;
  case AUTH_USER_NOT_FOUND:
    strcpy(response, RESP_ERR_USER_NOT_FOUND);
    break;
  default:
    strcpy(response, RESP_ERR_DB_ERROR);
    break;
  }

  send_response(client_socket, response);
  return session_id_to_return;
}

void handle_logout(int client_socket, const char *session_id) {
  char response[BUFFER_SIZE];

  int result = session_destroy(session_id);

  if (result == SESSION_SUCCESS) {
    strcpy(response, RESP_OK_LOGOUT);
  } else {
    strcpy(response, RESP_ERR_INVALID_SESSION);
  }

  send_response(client_socket, response);
}

void handle_upload(int client_socket, const char *group_name,
                   const char *client_path, const char *server_path) {
  char full_path[512];
  char dir_path[512];

  // Security check: prevent directory traversal
  if (strstr(server_path, "..") || strstr(group_name, "..")) {
    send_response(client_socket, "ERROR Invalid path or group name");
    return;
  }

  // Extract filename (basename) from client path
  char client_path_copy[256];
  strncpy(client_path_copy, client_path, sizeof(client_path_copy) - 1);
  client_path_copy[sizeof(client_path_copy) - 1] = '\0';

  const char *filename = strrchr(client_path_copy, '/');
  if (filename) {
    filename++; // Skip the '/'
  } else {
    filename = client_path_copy;
  }

  // Construct full save path: storage/<group_name>/<server_path>/<filename>
  // e.g., "storage/group1/docs/README.md"
  snprintf(full_path, sizeof(full_path), "storage/%s/%s/%s", group_name,
           server_path, filename);

  // Create directory structure if needed
  strncpy(dir_path, full_path, sizeof(dir_path) - 1);
  dir_path[sizeof(dir_path) - 1] = '\0';

  // Get directory part of full_path
  char *last_slash = strrchr(dir_path, '/');
  if (last_slash) {
    *last_slash = '\0'; // Truncate to get directory path

    // Create directories recursively
    char temp_path[512] = "";
    char *token = strtok(dir_path, "/");
    while (token != NULL) {
      strcat(temp_path, token);
      strcat(temp_path, "/");

      struct stat st = {0};
      if (stat(temp_path, &st) == -1) {
        if (mkdir(temp_path, 0755) != 0) {
          perror("Directory creation error");
          send_response(client_socket, "ERROR Cannot create directory");
          return;
        }
      }
      token = strtok(NULL, "/");
    }
  }

  // Send ready signal
  send_response(client_socket, RESP_OK_UPLOAD_READY);

  // Receive file size
  long filesize = 0;
  int n = recv(client_socket, &filesize, sizeof(filesize), 0);
  if (n <= 0) {
    printf("Error receiving file size\n");
    return;
  }
  printf("Expecting file size: %ld\n", filesize);

  // Delegate file I/O to file_ops module
  long bytes_received = receive_file(client_socket, full_path, filesize);

  // Send completion status
  if (bytes_received == filesize) {
    send_response(client_socket, RESP_OK_UPLOAD_COMPLETE);
  } else if (bytes_received >= 0) {
    printf("Upload incomplete. Expected %ld, got %ld\n", filesize,
           bytes_received);
    send_response(client_socket, "ERROR Upload incomplete");
  } else {
    send_response(client_socket, "ERROR Cannot create file");
  }
}
