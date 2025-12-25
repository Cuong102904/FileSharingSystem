#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include <pthread.h>

#define MAX_CLIENT_SESSIONS 1024
#define MAX_USERNAME_LEN 50

// Initialize client session module
void client_session_init(void);

// Cleanup client session module
void client_session_cleanup(void);

// Register a client socket with username after login
// Returns 0 on success, -1 on failure
int client_session_login(int client_socket, const char *username);

// Remove client session on logout or disconnect
void client_session_logout(int client_socket);

// Get username by client socket fd
// Returns pointer to username (do NOT free), or NULL if not found
const char *client_session_get_username(int client_socket);

// Check if client is logged in
int client_session_is_logged_in(int client_socket);

#endif // CLIENT_SESSION_H
