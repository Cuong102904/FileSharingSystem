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
        sscanf(buffer, "%*s %s %s", cmd->arg1, cmd->arg2);
        return CMD_REGISTER;
    }
    else if (strcmp(command, "LOGIN") == 0) {
        cmd->type = CMD_LOGIN;
        sscanf(buffer, "%*s %s %s", cmd->arg1, cmd->arg2);
        return CMD_LOGIN;
    }
    else if (strcmp(command, "LOGOUT") == 0) {
        cmd->type = CMD_LOGOUT;
        sscanf(buffer, "%*s %s", cmd->arg1);
        return CMD_LOGOUT;
    }

    cmd->type = CMD_UNKNOWN;
    return CMD_UNKNOWN;
}
