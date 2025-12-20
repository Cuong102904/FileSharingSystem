#include "../include/client_context.h"
#include "../lib/utils/testing.h"
#include <stdio.h>
#include <string.h>

// Global failed flag required by testing.h
int failed = 0;

TEST(test_context_init) {
  ClientContext ctx;
  int socket = 12345;

  // Test initialization
  client_context_init(&ctx, socket);

  ASSERT(ctx.client_socket == socket);
  ASSERT(ctx.is_authenticated == false);
  ASSERT_STR_EQ(ctx.username, "");
  ASSERT_STR_EQ(ctx.session_id, "");
}

TEST(test_context_update) {
  ClientContext ctx;
  client_context_init(&ctx, 10);

  // Simulate login update
  const char *username = "user1";
  const char *session = "sess123";

  strncpy(ctx.username, username, sizeof(ctx.username) - 1);
  strncpy(ctx.session_id, session, sizeof(ctx.session_id) - 1);
  ctx.is_authenticated = true;

  ASSERT_STR_EQ(ctx.username, "user1");
  ASSERT_STR_EQ(ctx.session_id, "sess123");
  ASSERT(ctx.is_authenticated == true);
}

int main() {
  printf("Running Client Context Unit Tests\n");
  printf("=================================\n");

  RUN_TEST(test_context_init);
  RUN_TEST(test_context_update);

  printf("\n=================================\n");

  if (failed) {
    printf("Some tests FAILED\n");
    return 1;
  } else {
    printf("All tests PASSED\n");
    return 0;
  }
}
