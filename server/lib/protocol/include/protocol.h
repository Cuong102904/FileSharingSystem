#ifndef PROTOCOL_H
#define PROTOCOL_H

#define BUFFER_SIZE 1024

// Command types
typedef enum {
    CMD_REGISTER,
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_UNKNOWN
} CommandType;

// Parsed command structure
typedef struct {
    CommandType type;
    char arg1[50];  // username or session_id
    char arg2[50];  // password (if applicable)
} ParsedCommand;

// Response codes
#define RESP_OK_REGISTER     "OK REGISTER"
#define RESP_OK_LOGIN        "OK LOGIN"
#define RESP_OK_LOGOUT       "OK LOGOUT"
#define RESP_ERR_ACCOUNT_EXISTS   "ERROR Account already exists"
#define RESP_ERR_WRONG_PASSWORD   "ERROR Wrong password"
#define RESP_ERR_USER_NOT_FOUND   "ERROR User not found"
#define RESP_ERR_INVALID_SESSION  "ERROR Invalid session"
#define RESP_ERR_SERVER_FULL      "ERROR Server full"
#define RESP_ERR_DB_ERROR         "ERROR Database error"
#define RESP_ERR_UNKNOWN_CMD      "ERROR Unknown command"

// Parser functions
CommandType protocol_parse_command(const char *buffer, ParsedCommand *cmd);

// Handler functions
void handle_register(int client_socket, const char *username, const char *password);
void handle_login(int client_socket, const char *username, const char *password);
void handle_logout(int client_socket, const char *session_id);

// Send response to client
void send_response(int client_socket, const char *response);

#endif // PROTOCOL_H
