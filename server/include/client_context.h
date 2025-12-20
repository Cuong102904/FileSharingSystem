#ifndef CLIENT_CONTEXT_H
#define CLIENT_CONTEXT_H

#include <stdbool.h>

/**
 * @brief Structure to track client state per connection thread
 */
typedef struct {
  int client_socket;
  char username[50];
  char session_id[65]; // SESSION_ID_LEN (64) + 1
  bool is_authenticated;
} ClientContext;

void client_context_init(ClientContext *ctx, int socket);

#endif // CLIENT_CONTEXT_H
