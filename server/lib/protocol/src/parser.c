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
    sscanf(buffer, "%*s %s %s %s", cmd->payload.upload.group,
           cmd->payload.upload.local_path, cmd->payload.upload.remote_path);
    return CMD_UPLOAD;
  }

  cmd->type = CMD_UNKNOWN;
  return CMD_UNKNOWN;
}
