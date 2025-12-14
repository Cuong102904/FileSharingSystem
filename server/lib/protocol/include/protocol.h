#ifndef PROTOCOL_H
#define PROTOCOL_H

#define BUFFER_SIZE 1024

// Command types
typedef enum {
    CMD_REGISTER,
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_UPLOAD,
    CMD_DOWNLOAD,
    CMD_UNKNOWN
} CommandType;

// Parsed command structure
typedef struct {
    CommandType type;
    char arg1[256];  // username, session_id, or file path (increased size for paths)
    char arg2[50];   // password or filesize
} ParsedCommand;

// Response codes
#define RESP_OK_REGISTER     "OK REGISTER"
#define RESP_OK_LOGIN        "OK LOGIN"
#define RESP_OK_LOGOUT       "OK LOGOUT"
#define RESP_OK_UPLOAD_READY "OK UPLOAD_READY"
#define RESP_OK_UPLOAD_COMPLETE "OK UPLOAD_COMPLETE"
#define RESP_ERR_ACCOUNT_EXISTS   "ERROR Account already exists"
#define RESP_ERR_WRONG_PASSWORD   "ERROR Wrong password"
#define RESP_ERR_USER_NOT_FOUND   "ERROR User not found"
#define RESP_ERR_INVALID_SESSION  "ERROR Invalid session"
#define RESP_ERR_SERVER_FULL      "ERROR Server full"
#define RESP_ERR_DB_ERROR         "ERROR Database error"
#define RESP_ERR_UNKNOWN_CMD      "ERROR Unknown command"
#define RESP_ERR_FILE_NOT_FOUND   "ERROR File not found"
#define RESP_ERR_ACCESS_DENIED    "ERROR Access denied"


// Parser functions
CommandType protocol_parse_command(const char *buffer, ParsedCommand *cmd);

// Handler functions
void handle_register(int client_socket, const char *username, const char *password);
void handle_login(int client_socket, const char *username, const char *password);
void handle_logout(int client_socket, const char *session_id);

// Send response to client
void send_response(int client_socket, const char *response);

#endif // PROTOCOL_H
