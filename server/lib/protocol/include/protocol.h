#ifndef PROTOCOL_H
#define PROTOCOL_H

#define BUFFER_SIZE 1024

// Command types
typedef enum {
  CMD_REGISTER,
  CMD_LOGIN,
  CMD_LOGOUT,
  CMD_CREATE_GROUP,
  CMD_LIST_GROUPS,
  CMD_LIST_MEMBERS,
  CMD_JOIN_REQ,
  CMD_APPROVE_JOIN,
  CMD_INVITE_USER,
  CMD_RESPOND_INVITE,
  CMD_UPLOAD,
  CMD_LEAVE_GROUP,
  CMD_KICK_MEMBER,
  CMD_DOWNLOAD,
  CMD_MKDIR,
  CMD_COPYFILE,
  CMD_COPYFOLDER,
  CMD_MOVEFILE,
  CMD_MOVEFOLDER,
  CMD_UNKNOWN
} CommandType;

// Parsed command structure
typedef struct {
  CommandType type;
  union {
    struct {
      char username[256];
      char password[256];
    } auth; // REGISTER, LOGIN
    struct {
      char session_id[256];
    } session; // LOGOUT
    struct {
      char group[256];
      char local_path[256];
      char remote_path[256];
    } upload; // UPLOAD
    struct {
      char group[256];
      char path[256];
    } download; // DOWNLOAD
    struct {
      char group[256];
      char path[256];
    } mkdir; // MKDIR
    struct {
      char group[256];
      char source[256];
      char destination[256];
    } copyfile; // COPYFILE
    struct {
      char group[256];
      char source[256];
      char destination[256];
    } copyfolder; // COPYFOLDER
    struct {
      char group[256];
      char source[256];
      char destination[256];
    } movefile; // MOVEFILE
    struct {
      char group[256];
      char source[256];
      char destination[256];
    } movefolder; // MOVEFOLDER
    struct {
      char group_name[256];
      char user_name[256];
      char status[16]; // for ACCEPT_INVITE/REJECT_INVITE: accept/reject
    } group; // CREATE_GROUP, LIST_GROUPS, ACCEPT_INVITE, REJECT_INVITE, LEAVE_GROUP, KICK_MEMBER
  } payload;
} ParsedCommand;

// Response codes
#define RESP_OK_REGISTER "OK REGISTER"
#define RESP_OK_LOGIN "OK LOGIN"
#define RESP_OK_LOGOUT "OK LOGOUT"
#define RESP_OK_UPLOAD_READY "OK UPLOAD_READY"
#define RESP_OK_UPLOAD_COMPLETE "OK UPLOAD_COMPLETE"
#define RESP_OK_DOWNLOAD "OK DOWNLOAD" // New response
#define RESP_OK_MKDIR "OK MKDIR"
#define RESP_OK_COPYFILE "OK COPYFILE"
#define RESP_OK_COPYFOLDER "OK COPYFOLDER"
#define RESP_OK_MOVEFILE "OK MOVEFILE"
#define RESP_OK_MOVEFOLDER "OK MOVEFOLDER"
#define RESP_OK_CREATE_GROUP "OK CREATE_GROUP"
#define RESP_ERR_GROUPNAME_EXISTS "ERROR Group name already exists"
#define RESP_OK_LIST_GROUP "OK LIST_GROUP"
#define RESP_OK_LIST_MEMBERS "OK LIST_MEMBERS"
#define RESP_ERR_NOT_IN_GROUP "ERROR User is not in this group"
#define RESP_ERR_GROUP_NOT_FOUND "ERROR Group not found"
#define RESP_OK_JOIN_REQ "OK JOIN_REQ"
#define RESP_ERR_ALREADY_IN_GROUP "ERROR You are already in this group"
#define RESP_ERR_ALREADY_PENDING "ERROR You already have a pending request"
#define RESP_OK_APPROVE_JOIN "OK APPROVE_JOIN"
#define RESP_ERR_NO_PERMISSION "ERROR No permission"
#define RESP_ERR_NO_PENDING_REQUEST "ERROR User has no pending request"
#define RESP_OK_INVITE_USER "OK INVITE_USER"
#define RESP_OK_ACCEPT_INVITE "OK ACCEPT_INVITE"
#define RESP_OK_REJECT_INVITE "OK REJECT_INVITE"
#define RESP_ERR_NOT_INVITED "ERROR You have not been invited to this group"
#define RESP_ERR_INVALID_STATUS "ERROR Invalid status (use accept or reject)"
#define RESP_ERR_USER_ALREADY_INVITED "ERROR User already invited"
#define RESP_ERR_ACCOUNT_EXISTS "ERROR Account already exists"
#define RESP_ERR_WRONG_PASSWORD "ERROR Wrong password"
#define RESP_ERR_USER_NOT_FOUND "ERROR User not found"
#define RESP_ERR_INVALID_SESSION "ERROR Invalid session"
#define RESP_ERR_SERVER_FULL "ERROR Server full"
#define RESP_ERR_DB_ERROR "ERROR Database error"
#define RESP_ERR_UNKNOWN_CMD "ERROR Unknown command"
#define RESP_ERR_FILE_NOT_FOUND "ERROR File not found"
#define RESP_ERR_ACCESS_DENIED "ERROR Access denied"
#define RESP_ERR_FOLDER_EXISTS "ERROR Folder exists"
#define RESP_ERR_PERMISSION_DENIED "ERROR Permission denied"
#define RESP_ERR_ALREADY_LOGGED_IN "ERROR Already logged in"
#define RESP_ERR_NOT_LOGGED_IN "ERROR Not logged in"
#define RESP_OK_LEAVE_GROUP "OK LEAVE_GROUP"
#define RESP_OK_KICK_MEMBER "OK KICK_MEMBER"

// Parser functions
CommandType protocol_parse_command(const char *buffer, ParsedCommand *cmd);

// Handler functions
void handle_register(int client_socket, const char *username,
                     const char *password);
void handle_login(int client_socket, const char *username,
                  const char *password);
void handle_create_group(int client_socket, const char *group_name);
void handle_list_groups_by_user(int client_socket);
void handle_list_members(int client_socket, const char *group_name);
void handle_join_request(int client_socket, const char *group_name);
void handle_approve_join(int client_socket, const char *group_name, const char *target_user);
void handle_invite_user(int client_socket, const char *group_name, const char *target_user);
void handle_respond_invite(int client_socket, const char *group_name, const char *status);
void handle_leave_group(int client_socket, const char *group_name);
void handle_kick_member(int client_socket, const char *group_name, const char *member_name);
void handle_logout(int client_socket);
void handle_upload(int client_socket, const char *group_name,
                   const char *client_path, const char *server_path);
void handle_download(int client_socket, const char *group_name,
                     const char *server_path);
void handle_mkdir(int client_socket, const char *group_name, const char *path);
void handle_copyfile(int client_socket, const char *group_name,
                     const char *source, const char *destination);
void handle_copyfolder(int client_socket, const char *group_name,
                       const char *source, const char *destination);
void handle_movefile(int client_socket, const char *group_name,
                     const char *source, const char *destination);
void handle_movefolder(int client_socket, const char *group_name,
                       const char *source, const char *destination);

// Send response to client
void send_response(int client_socket, const char *response);

#endif // PROTOCOL_H
