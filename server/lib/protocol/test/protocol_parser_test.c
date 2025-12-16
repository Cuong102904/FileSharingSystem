#include "../../utils/testing.h"
#include "../include/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define global failed flag
int failed = 0;

TEST(test_parse_upload_command) {
  ParsedCommand cmd;
  // Simulate input: UPLOAD <client_path> <server_path>
  const char *input = "UPLOAD g1 /home/user/docs/report.pdf storage/docs/";

  // Parse command
  CommandType type = protocol_parse_command(input, &cmd);

  // Check command type
  ASSERT(type == CMD_UPLOAD);
  ASSERT(cmd.type == CMD_UPLOAD);

  // Check arguments
  ASSERT_STR_EQ(cmd.payload.upload.group, "g1"); // group
  ASSERT_STR_EQ(cmd.payload.upload.local_path,
                "/home/user/docs/report.pdf");                    // Client path
  ASSERT_STR_EQ(cmd.payload.upload.remote_path, "storage/docs/"); // Server path
}

int main() {
  printf("Running Protocol Parser Unit Tests\n");
  printf("==================================\n");

  RUN_TEST(test_parse_upload_command);

  printf("\n==================================\n");

  if (failed) {
    printf("Some tests FAILED\n");
    return 1;
  } else {
    printf("All tests PASSED\n");
    return 0;
  }
}
