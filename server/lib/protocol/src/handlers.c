#include "../../auth/include/auth.h"
#include "../../client_session/include/client_session.h"
#include "../../file_ops/include/directory_ops.h"
#include "../../file_ops/include/file_transfer.h"
#include "../../group/include/group_repo.h"
#include "../../session/include/session.h"
#include "../include/protocol.h"
#include <errno.h>
#include <fcntl.h>
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
    if (members)
      free(members);
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

void handle_approve_join(int client_socket, const char *group_name,
                         const char *target_user) {
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

void handle_invite_user(int client_socket, const char *group_name,
                        const char *target_user) {
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
      snprintf(error_msg, BUFFER_SIZE, "ERROR %s is already in this group",
               target_user);
    }
    send_response(client_socket, error_msg);
    return;
  }

  // Check if target user already has pending request
  if (is_user_pending(group_name, target_user)) {
    char error_msg[BUFFER_SIZE];
    if (strcmp(username, target_user) == 0) {
      snprintf(error_msg, BUFFER_SIZE,
               "ERROR You already have a pending request");
    } else {
      snprintf(error_msg, BUFFER_SIZE, "ERROR %s already has a pending request",
               target_user);
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
      snprintf(error_msg, BUFFER_SIZE, "ERROR %s has already been invited",
               target_user);
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

void handle_respond_invite(int client_socket, const char *group_name, const char *status) {
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

void handle_leave_group(int client_socket, const char *group_name){
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

  int result = group_remove_member(group_name, username);
  if(result == GROUP_REPO_OK){
    send_response(client_socket, RESP_OK_LEAVE_GROUP);
  }
  else if(result == GROUP_REPO_ERR_NOT_FOUND){
    send_response(client_socket, RESP_ERR_NOT_IN_GROUP);
  }
  else{
    send_response(client_socket, RESP_ERR_DB_ERROR);
  }
}

void handle_kick_member(int client_socket, const char *group_name, const char *member_name){
  // Get username from client session
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, RESP_ERR_NOT_LOGGED_IN);
    return;
  }

  // Check if current user is owner of the group
  if (!is_user_owner(group_name, username)) {
    send_response(client_socket, RESP_ERR_NO_PERMISSION);
    return;
  }

  // Check if group exists
  if (find_group_by_name(group_name) != 1) {
    send_response(client_socket, RESP_ERR_GROUP_NOT_FOUND);
    return;
  }

  int result = group_remove_member(group_name, member_name);
  if(result == GROUP_REPO_OK){
    send_response(client_socket, RESP_OK_KICK_MEMBER);
  }
  else if(result == GROUP_REPO_ERR_NOT_FOUND){
    send_response(client_socket, RESP_ERR_NOT_IN_GROUP);
  }
  else{
    send_response(client_socket, RESP_ERR_DB_ERROR);
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

  // Permission check: verify user is a member of the group
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, "ERROR Not logged in");
    return;
  }

  if (!is_user_in_group(group_name, username)) {
    send_response(client_socket, "ERROR You are not a member of this group");
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

  snprintf(full_path, sizeof(full_path), "storage/%s/%s/%s", group_name,
           server_path, filename);

  strncpy(dir_path, full_path, sizeof(dir_path) - 1);
  dir_path[sizeof(dir_path) - 1] = '\0';

  char *last_slash = strrchr(dir_path, '/');
  if (last_slash) {
    *last_slash = '\0';
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

  // Save current flags
  int flags = fcntl(client_socket, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl F_GETFL failed");
    send_response(client_socket, "ERROR Server error");
    return;
  }

  // Set to blocking mode
  if (fcntl(client_socket, F_SETFL, flags & ~O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL failed");
    send_response(client_socket, "ERROR Server error");
    return;
  }

  // Receive file size
  long filesize = 0;
  int n = recv(client_socket, &filesize, sizeof(filesize), 0);
  if (n <= 0) {
    // Restore non-blocking mode before returning
    fcntl(client_socket, F_SETFL, flags);
    return;
  }

  long bytes_received = receive_file(client_socket, full_path, filesize);

  if (fcntl(client_socket, F_SETFL, flags) == -1) {
    perror("fcntl restore failed");
  }

  // Send completion status
  if (bytes_received == filesize) {
    send_response(client_socket, RESP_OK_UPLOAD_COMPLETE);
  } else if (bytes_received >= 0) {
    send_response(client_socket, "ERROR Upload incomplete");
  } else {
    send_response(client_socket, "ERROR Cannot create file");
  }
}

void handle_download(int client_socket, const char *group_name,
                     const char *server_path) {
  char full_path[512];

  // Security check: prevent directory traversal
  if (strstr(server_path, "..") || strstr(group_name, "..")) {
    send_response(client_socket, "ERROR Invalid path or group name");
    return;
  }

  // Permission check: verify user is a member of the group
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, "ERROR Not logged in");
    return;
  }

  if (!is_user_in_group(group_name, username)) {
    send_response(client_socket, "ERROR You are not a member of this group");
    return;
  }

  // Construct full path: storage/<group_name>/<server_path>
  snprintf(full_path, sizeof(full_path), "storage/%s/%s", group_name,
           server_path);

  // Check file existence
  struct stat st;
  if (stat(full_path, &st) == -1) {
    send_response(client_socket, RESP_ERR_FILE_NOT_FOUND);
    return;
  }

  // Check if it's a regular file
  if (!S_ISREG(st.st_mode)) {
    send_response(client_socket, "ERROR Not a file");
    return;
  }

  long filesize = st.st_size;

  // Send OK response with filesize
  char response[256];
  snprintf(response, sizeof(response), "%s %ld", RESP_OK_DOWNLOAD, filesize);
  send_response(client_socket, response);

  // Set socket to BLOCKING mode for handshake AND file transfer
  int flags = fcntl(client_socket, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl F_GETFL failed");
    return;
  }

  if (fcntl(client_socket, F_SETFL, flags & ~O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL failed");
    return;
  }

  char ack[64];
  int ack_len = recv(client_socket, ack, sizeof(ack), 0);
  if (ack_len <= 0) {
    printf("Error waiting for client ACK (recv returned %d)\n", ack_len);
    fcntl(client_socket, F_SETFL, flags);
    return;
  }

  long bytes_sent = send_file(client_socket, full_path);

  fcntl(client_socket, F_SETFL, flags);

  if (bytes_sent != filesize) {
    printf("WARNING: Download incomplete. Sent %ld/%ld bytes\n", bytes_sent,
           filesize);
  }
}

void handle_mkdir(int client_socket, const char *group_name, const char *path) {
  char full_path[512];

  // Security: directory traversal check
  if (strstr(path, "..") || strstr(group_name, "..")) {
    send_response(client_socket, "ERROR Invalid path");
    return;
  }

  // Authentication & Authorization
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, "ERROR Not logged in");
    return;
  }

  if (!is_user_in_group(group_name, username)) {
    send_response(client_socket, RESP_ERR_PERMISSION_DENIED);
    return;
  }

  // Construct full path
  snprintf(full_path, sizeof(full_path), "storage/%s/%s", group_name, path);

  // Check if already exists
  struct stat st;
  if (stat(full_path, &st) == 0) {
    send_response(client_socket, RESP_ERR_FOLDER_EXISTS);
    return;
  }

  // Create directory using directory_ops
  if (create_directory_recursive(full_path) != 0) {
    send_response(client_socket, "ERROR Cannot create directory");
    return;
  }

  send_response(client_socket, RESP_OK_MKDIR);
}

void handle_copyfile(int client_socket, const char *group_name,
                     const char *source, const char *destination) {
  char src_path[512], dst_path[512];

  // Security checks
  if (strstr(source, "..") || strstr(destination, "..") ||
      strstr(group_name, "..")) {
    send_response(client_socket, "ERROR Invalid path");
    return;
  }

  if (!is_file_path(source)) {
    send_response(client_socket,
                  "ERROR Source must be a file (use COPYFOLDER for "
                  "directories)");
    return;
  }

  if (!is_file_path(destination)) {
    send_response(client_socket, "ERROR Destination must be a file path");
    return;
  }

  // Authentication & Authorization
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, "ERROR Not logged in");
    return;
  }

  if (!is_user_in_group(group_name, username)) {
    send_response(client_socket, RESP_ERR_PERMISSION_DENIED);
    return;
  }

  snprintf(src_path, sizeof(src_path), "storage/%s/%s", group_name, source);
  snprintf(dst_path, sizeof(dst_path), "storage/%s/%s", group_name,
           destination);

  // Check source exists AND is a file
  if (validate_path_type(src_path, 1) != 0) {
    send_response(client_socket,
                  "ERROR Source file not found or is not a file");
    return;
  }

  // Check destination parent directory exists
  if (check_parent_directory_exists(dst_path) != 0) {
    send_response(client_socket,
                  "ERROR Destination folder not found (use MKDIR first)");
    return;
  }

  // Perform copy (reuse existing logic)
  if (copy_path(src_path, dst_path) != 0) {
    send_response(client_socket, "ERROR Copy file operation failed");
    return;
  }

  send_response(client_socket, RESP_OK_COPYFILE);
}

void handle_copyfolder(int client_socket, const char *group_name,
                       const char *source, const char *destination) {
  char src_path[512], dst_path[512];

  // Security checks
  if (strstr(source, "..") || strstr(destination, "..") ||
      strstr(group_name, "..")) {
    send_response(client_socket, "ERROR Invalid path");
    return;
  }

  // Validate source is a folder path (no extension)
  if (!is_folder_path(source)) {
    send_response(client_socket,
                  "ERROR Source must be a folder (use COPYFILE for files)");
    return;
  }

  // Validate destination is a folder path
  if (!is_folder_path(destination)) {
    send_response(client_socket, "ERROR Destination must be a folder path");
    return;
  }

  // Authentication & Authorization
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, "ERROR Not logged in");
    return;
  }

  if (!is_user_in_group(group_name, username)) {
    send_response(client_socket, RESP_ERR_PERMISSION_DENIED);
    return;
  }

  // Construct paths
  snprintf(src_path, sizeof(src_path), "storage/%s/%s", group_name, source);
  snprintf(dst_path, sizeof(dst_path), "storage/%s/%s", group_name,
           destination);

  // Check source exists AND is a directory
  if (validate_path_type(src_path, 0) != 0) {
    send_response(client_socket,
                  "ERROR Source folder not found or is not a folder");
    return;
  }

  // Check destination parent directory exists
  if (check_parent_directory_exists(dst_path) != 0) {
    send_response(client_socket,
                  "ERROR Destination parent folder not found (use MKDIR "
                  "first)");
    return;
  }

  // Perform copy (reuse existing logic)
  if (copy_path(src_path, dst_path) != 0) {
    send_response(client_socket, "ERROR Copy folder operation failed");
    return;
  }

  send_response(client_socket, RESP_OK_COPYFOLDER);
}

void handle_movefile(int client_socket, const char *group_name,
                     const char *source, const char *destination) {
  char src_path[1024], dst_folder[512], dst_path[1024];

  // Security checks
  if (strstr(source, "..") || strstr(destination, "..") ||
      strstr(group_name, "..")) {
    send_response(client_socket, "ERROR Invalid path");
    return;
  }

  // Validate source is a file path
  if (!is_file_path(source)) {
    send_response(client_socket,
                  "ERROR Source must be a file (use MOVEFOLDER for "
                  "directories)");
    return;
  }

  // Validate destination is a FOLDER (must not have extension)
  if (is_file_path(destination)) {
    send_response(client_socket,
                  "ERROR Destination must be a folder (e.g. archive/)");
    return;
  }

  // Authentication & Authorization
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, "ERROR Not logged in");
    return;
  }

  if (!is_user_in_group(group_name, username)) {
    send_response(client_socket, RESP_ERR_PERMISSION_DENIED);
    return;
  }

  // Construct source path
  snprintf(src_path, sizeof(src_path), "storage/%s/%s", group_name, source);

  // Extract filename from source
  char source_copy[512];
  strncpy(source_copy, source, sizeof(source_copy) - 1);
  source_copy[sizeof(source_copy) - 1] = '\0';
  char *filename = basename(source_copy);

  // Construct destination folder path and full destination path
  snprintf(dst_folder, sizeof(dst_folder), "storage/%s/%s", group_name,
           destination);
  snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_folder, filename);

  // Check source exists AND is a file
  if (validate_path_type(src_path, 1) != 0) {
    send_response(client_socket,
                  "ERROR Source file not found or is not a file");
    return;
  }

  // Check destination folder exists
  if (validate_path_type(dst_folder, 0) != 0) {
    send_response(client_socket,
                  "ERROR Destination folder not found (use MKDIR first)");
    return;
  }

  // Perform move (reuse existing logic)
  if (move_path(src_path, dst_path) != 0) {
    send_response(client_socket, "ERROR Move file failed");
    return;
  }

  send_response(client_socket, RESP_OK_MOVEFILE);
}

void handle_movefolder(int client_socket, const char *group_name,
                       const char *source, const char *destination) {
  char src_path[1024], dst_folder[512], dst_path[1024];

  // Security checks
  if (strstr(source, "..") || strstr(destination, "..") ||
      strstr(group_name, "..")) {
    send_response(client_socket, "ERROR Invalid path");
    return;
  }

  // Validate source is a folder path
  if (!is_folder_path(source)) {
    send_response(client_socket,
                  "ERROR Source must be a folder (use MOVEFILE for files)");
    return;
  }

  // Validate destination is a FOLDER
  if (is_file_path(destination)) {
    send_response(client_socket, "ERROR Destination must be a folder");
    return;
  }

  // Authentication & Authorization
  const char *username = client_session_get_username(client_socket);
  if (username == NULL) {
    send_response(client_socket, "ERROR Not logged in");
    return;
  }

  if (!is_user_in_group(group_name, username)) {
    send_response(client_socket, RESP_ERR_PERMISSION_DENIED);
    return;
  }

  // Construct source path
  snprintf(src_path, sizeof(src_path), "storage/%s/%s", group_name, source);

  // Extract folder name from source
  char source_copy[512];
  strncpy(source_copy, source, sizeof(source_copy) - 1);
  source_copy[sizeof(source_copy) - 1] = '\0';
  char *foldername = basename(source_copy);

  // Construct destination folder path and full destination path
  snprintf(dst_folder, sizeof(dst_folder), "storage/%s/%s", group_name,
           destination);
  snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_folder, foldername);

  // Check source exists AND is a directory
  if (validate_path_type(src_path, 0) != 0) {
    send_response(client_socket,
                  "ERROR Source folder not found or is not a folder");
    return;
  }

  // Check destination folder exists
  if (validate_path_type(dst_folder, 0) != 0) {
    send_response(client_socket, "ERROR Destination folder not found");
    return;
  }

  // Perform move (reuse existing logic)
  if (move_path(src_path, dst_path) != 0) {
    send_response(client_socket, "ERROR Move folder failed");
    return;
  }

  send_response(client_socket, RESP_OK_MOVEFOLDER);
}
