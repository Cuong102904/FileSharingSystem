#include "../../auth/include/auth.h"
#include "../../client_session/include/client_session.h"
#include "../../file_ops/include/file_transfer.h"
#include "../../session/include/session.h"
#include "../include/protocol.h"
#include "../../group/include/group_repo.h"
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
  char response[BUFFER_SIZE];

  int result = authenticate_user(username, password);

  switch (result) {
  case AUTH_SUCCESS:
    // Register client socket -> username mapping
    if (client_session_login(client_socket, username) == 0) {
      snprintf(response, BUFFER_SIZE, "%s %s", RESP_OK_LOGIN, username);
    } else {
      strcpy(response, RESP_ERR_SERVER_FULL);
    }
    break;
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
}

void handle_logout(int client_socket) {
  char response[BUFFER_SIZE];

  // Get username from socket and logout
  const char *username = client_session_get_username(client_socket);
  if (username != NULL) {
    client_session_logout(client_socket);
    strcpy(response, RESP_OK_LOGOUT);
  } else {
    strcpy(response, RESP_ERR_NOT_LOGGED_IN);
  }

  send_response(client_socket, response);
}

void handle_create_group(int client_socket, const char *group_name) {
  char response[BUFFER_SIZE];

  // Get username from client session
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, "ERROR Not logged in");
    return;
  }

  int result = group_create(group_name, username);

  if (result == GROUP_REPO_OK) {
    strcpy(response, RESP_OK_CREATE_GROUP);
  } else {
    strcpy(response, RESP_ERR_GROUPNAME_EXISTS);
  }

  send_response(client_socket, response);
}

void handle_list_groups_by_user(int client_socket) {
  // Get username from client session
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, "ERROR Not logged in");
    return;
  }

  char *result = group_list_all_by_user(username);
  if (result != NULL) {
    // Combine list and status into single response
    size_t result_len = strlen(result);
    size_t status_len = strlen(RESP_OK_LIST_GROUP);
    char *combined = malloc(result_len + status_len + 1);
    if (combined != NULL) {
      memcpy(combined, result, result_len);
      memcpy(combined + result_len, RESP_OK_LIST_GROUP, status_len + 1);
      send(client_socket, combined, result_len + status_len, 0);
      free(combined);
    } else {
      // Fallback: send separately if malloc fails
      send(client_socket, result, result_len, 0);
      send_response(client_socket, RESP_OK_LIST_GROUP);
    }
    free(result);
  } else {
    send_response(client_socket, RESP_ERR_DB_ERROR);
  }
}

void handle_list_members(int client_socket, const char *group_name) {
  // Get username from client session
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, RESP_ERR_NOT_LOGGED_IN);
    return;
  }

  // Check if group exists
  if (find_group_by_name(group_name) != 1) {
    send_response(client_socket, RESP_ERR_GROUP_NOT_FOUND);
    return;
  }

  // Check if user is in the group
  if (!is_user_in_group(group_name, username)) {
    send_response(client_socket, RESP_ERR_NOT_IN_GROUP);
    return;
  }

  // Get list of members
  char *members = group_list_members(group_name);
  if (members != NULL && strlen(members) > 0) {
    // Combine list and status into single response
    size_t members_len = strlen(members);
    size_t status_len = strlen(RESP_OK_LIST_MEMBERS);
    char *combined = malloc(members_len + status_len + 1);
    if (combined != NULL) {
      memcpy(combined, members, members_len);
      memcpy(combined + members_len, RESP_OK_LIST_MEMBERS, status_len + 1);
      send(client_socket, combined, members_len + status_len, 0);
      free(combined);
    } else {
      send(client_socket, members, members_len, 0);
      send_response(client_socket, RESP_OK_LIST_MEMBERS);
    }
    free(members);
  } else {
    if (members) free(members);
    send_response(client_socket, RESP_ERR_GROUP_NOT_FOUND);
  }
}

void handle_join_request(int client_socket, const char *group_name) {
  // Get username from client session
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, RESP_ERR_NOT_LOGGED_IN);
    return;
  }

  // Check if group exists
  if (find_group_by_name(group_name) != 1) {
    send_response(client_socket, RESP_ERR_GROUP_NOT_FOUND);
    return;
  }

  // Check if user is already in the group (owner or member)
  if (is_user_in_group(group_name, username)) {
    send_response(client_socket, RESP_ERR_ALREADY_IN_GROUP);
    return;
  }

  // Check if user already has pending request
  if (is_user_pending(group_name, username)) {
    send_response(client_socket, RESP_ERR_ALREADY_PENDING);
    return;
  }

  // Add user as pending member
  int result = group_add_pending_member(group_name, username);
  if (result == GROUP_REPO_OK) {
    send_response(client_socket, RESP_OK_JOIN_REQ);
  } else {
    send_response(client_socket, RESP_ERR_DB_ERROR);
  }
}

void handle_approve_join(int client_socket, const char *group_name, const char *target_user) {
  // Get username from client session (the approver)
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, RESP_ERR_NOT_LOGGED_IN);
    return;
  }

  // Check if group exists
  if (find_group_by_name(group_name) != 1) {
    send_response(client_socket, RESP_ERR_GROUP_NOT_FOUND);
    return;
  }

  // Check if current user is owner of the group
  if (!is_user_owner(group_name, username)) {
    send_response(client_socket, RESP_ERR_NO_PERMISSION);
    return;
  }

  // Check if target user has pending request
  if (!is_user_pending(group_name, target_user)) {
    send_response(client_socket, RESP_ERR_NO_PENDING_REQUEST);
    return;
  }

  // Approve the member
  int result = group_approve_member(group_name, target_user);
  if (result == GROUP_REPO_OK) {
    send_response(client_socket, RESP_OK_APPROVE_JOIN);
  } else {
    send_response(client_socket, RESP_ERR_DB_ERROR);
  }
}

void handle_invite_user(int client_socket, const char *group_name, const char *target_user) {
  // Get username from client session (the inviter)
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, RESP_ERR_NOT_LOGGED_IN);
    return;
  }

  // Check if group exists
  if (find_group_by_name(group_name) != 1) {
    send_response(client_socket, RESP_ERR_GROUP_NOT_FOUND);
    return;
  }

  // Check if current user is owner of the group
  if (!is_user_owner(group_name, username)) {
    send_response(client_socket, RESP_ERR_NO_PERMISSION);
    return;
  }

  // Check if target user is already in the group
  if (is_user_in_group(group_name, target_user)) {
    char error_msg[BUFFER_SIZE];
    if (strcmp(username, target_user) == 0) {
      snprintf(error_msg, BUFFER_SIZE, "ERROR You are already in this group");
    } else {
      snprintf(error_msg, BUFFER_SIZE, "ERROR %s is already in this group", target_user);
    }
    send_response(client_socket, error_msg);
    return;
  }

  // Check if target user already has pending request
  if (is_user_pending(group_name, target_user)) {
    char error_msg[BUFFER_SIZE];
    if (strcmp(username, target_user) == 0) {
      snprintf(error_msg, BUFFER_SIZE, "ERROR You already have a pending request");
    } else {
      snprintf(error_msg, BUFFER_SIZE, "ERROR %s already has a pending request", target_user);
    }
    send_response(client_socket, error_msg);
    return;
  }

  // Check if target user already has been invited
  if (is_user_invited(group_name, target_user)) {
    char error_msg[BUFFER_SIZE];
    if (strcmp(username, target_user) == 0) {
      snprintf(error_msg, BUFFER_SIZE, "ERROR You have already been invited");
    } else {
      snprintf(error_msg, BUFFER_SIZE, "ERROR %s has already been invited", target_user);
    }
    send_response(client_socket, error_msg);
    return;
  }

  // Invite the user
  int result = group_invite_user(group_name, target_user);
  if (result == GROUP_REPO_OK) {
    send_response(client_socket, RESP_OK_INVITE_USER);
  } else {
    send_response(client_socket, RESP_ERR_DB_ERROR);
  }
}

void handle_accept_invite(int client_socket, const char *group_name, const char *status) {
  // Get username from client session
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, RESP_ERR_NOT_LOGGED_IN);
    return;
  }

  // Check if group exists
  if (find_group_by_name(group_name) != 1) {
    send_response(client_socket, RESP_ERR_GROUP_NOT_FOUND);
    return;
  }

  // Check if user has been invited
  if (!is_user_invited(group_name, username)) {
    send_response(client_socket, RESP_ERR_NOT_INVITED);
    return;
  }

  // Handle accept or reject
  if (strcmp(status, "accept") == 0) {
    int result = group_accept_invite(group_name, username);
    if (result == GROUP_REPO_OK) {
      send_response(client_socket, RESP_OK_ACCEPT_INVITE);
    } else {
      send_response(client_socket, RESP_ERR_DB_ERROR);
    }
  } else if (strcmp(status, "reject") == 0) {
    int result = group_reject_invite(group_name, username);
    if (result == GROUP_REPO_OK) {
      send_response(client_socket, RESP_OK_REJECT_INVITE);
    } else {
      send_response(client_socket, RESP_ERR_DB_ERROR);
    }
  } else {
    send_response(client_socket, RESP_ERR_INVALID_STATUS);
  }
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

