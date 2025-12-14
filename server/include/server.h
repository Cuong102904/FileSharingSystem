#ifndef SERVER_H
#define SERVER_H

#define PORT 8080
#define MAX_CLIENTS 100

// Initialize all server modules
void server_init(void);

// Cleanup all server modules
void server_cleanup(void);

// Handle client connection
void* handle_client(void *arg);

#endif // SERVER_H
