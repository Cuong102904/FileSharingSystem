#include "../include/client_context.h"
#include <string.h>

void client_context_init(ClientContext *ctx, int socket) {
  if (ctx) {
    ctx->client_socket = socket;
    memset(ctx->username, 0, sizeof(ctx->username));
    memset(ctx->session_id, 0, sizeof(ctx->session_id));
    ctx->is_authenticated = false;
  }
}
