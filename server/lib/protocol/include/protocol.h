#ifndef PROTOCOL_H
#define PROTOCOL_H

#define BUFFER_SIZE 1024

// Command types
typedef enum {
  CMD_REGISTER,
  CMD_LOGIN,
  CMD_LOGOUT,
  CMD_UPLOAD,
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
      char path[256];
    } download; // DOWNLOAD
  } payload;
} ParsedCommand;

// Response codes
#define RESP_OK_REGISTER "OK REGISTER"
#define RESP_OK_LOGIN "OK LOGIN"
#define RESP_OK_LOGOUT "OK LOGOUT"
#define RESP_OK_UPLOAD_READY "OK UPLOAD_READY"
#define RESP_OK_UPLOAD_COMPLETE "OK UPLOAD_COMPLETE"
#define RESP_ERR_ACCOUNT_EXISTS "ERROR Account already exists"
#define RESP_ERR_WRONG_PASSWORD "ERROR Wrong password"
#define RESP_ERR_USER_NOT_FOUND "ERROR User not found"
#define RESP_ERR_INVALID_SESSION "ERROR Invalid session"
#define RESP_ERR_SERVER_FULL "ERROR Server full"
#define RESP_ERR_DB_ERROR "ERROR Database error"
#define RESP_ERR_UNKNOWN_CMD "ERROR Unknown command"
#define RESP_ERR_FILE_NOT_FOUND "ERROR File not found"
#define RESP_ERR_ACCESS_DENIED "ERROR Access denied"

// Parser functions
CommandType protocol_parse_command(const char *buffer, ParsedCommand *cmd);

// Handler functions
void handle_register(int client_socket, const char *username,
                     const char *password);
void handle_login(int client_socket, const char *username,
                  const char *password);
void handle_logout(int client_socket, const char *session_id);
void handle_upload(int client_socket, const char *group_name,
                   const char *client_path, const char *server_path);

// Send response to client
void send_response(int client_socket, const char *response);

#endif // PROTOCOL_H
