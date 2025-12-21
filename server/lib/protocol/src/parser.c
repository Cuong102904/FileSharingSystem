#include "../include/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CommandType protocol_parse_command(const char *buffer, ParsedCommand *cmd) {
  char command[20];

  // Initialize command structure
  memset(cmd, 0, sizeof(ParsedCommand));

  // Parse command type
  sscanf(buffer, "%s", command);

  if (strcmp(command, "REGISTER") == 0) {
    cmd->type = CMD_REGISTER;
    sscanf(buffer, "%*s %s %s", cmd->payload.auth.username,
           cmd->payload.auth.password);
    return CMD_REGISTER;
  } else if (strcmp(command, "LOGIN") == 0) {
    cmd->type = CMD_LOGIN;
    sscanf(buffer, "%*s %s %s", cmd->payload.auth.username,
           cmd->payload.auth.password);
    return CMD_LOGIN;
  } else if (strcmp(command, "LOGOUT") == 0) {
    cmd->type = CMD_LOGOUT;
    sscanf(buffer, "%*s %s", cmd->payload.session.session_id);
    return CMD_LOGOUT;
  } else if (strcmp(command, "UPLOAD") == 0) {
    cmd->type = CMD_UPLOAD;
    // Format: UPLOAD <session_id> <group> <client_path> <server_path>
    if (sscanf(buffer, "UPLOAD %64s %255s %255s %255s", cmd->session_id,
               cmd->payload.upload.group, cmd->payload.upload.client_path,
               cmd->payload.upload.server_path) == 4) {
      return CMD_UPLOAD;
    }
  }

  cmd->type = CMD_UNKNOWN;
  return CMD_UNKNOWN;
}
