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
  } else if (strcmp(command, "DOWNLOAD") == 0) {
    cmd->type = CMD_DOWNLOAD;
    sscanf(buffer, "%*s %s %s", cmd->payload.download.group,
           cmd->payload.download.path);
    return CMD_DOWNLOAD;
  } else if (strcmp(command, "MKDIR") == 0) {
    cmd->type = CMD_MKDIR;
    sscanf(buffer, "%*s %s %s", cmd->payload.mkdir.group,
           cmd->payload.mkdir.path);
    return CMD_MKDIR;
  } else if (strcmp(command, "COPYFILE") == 0) {
    cmd->type = CMD_COPYFILE;
    sscanf(buffer, "%*s %s %s %s", cmd->payload.copyfile.group,
           cmd->payload.copyfile.source, cmd->payload.copyfile.destination);
    return CMD_COPYFILE;
  } else if (strcmp(command, "COPYFOLDER") == 0) {
    cmd->type = CMD_COPYFOLDER;
    sscanf(buffer, "%*s %s %s %s", cmd->payload.copyfolder.group,
           cmd->payload.copyfolder.source, cmd->payload.copyfolder.destination);
    return CMD_COPYFOLDER;
  } else if (strcmp(command, "MOVEFILE") == 0) {
    cmd->type = CMD_MOVEFILE;
    sscanf(buffer, "%*s %s %s %s", cmd->payload.movefile.group,
           cmd->payload.movefile.source, cmd->payload.movefile.destination);
    return CMD_MOVEFILE;
  } else if (strcmp(command, "MOVEFOLDER") == 0) {
    cmd->type = CMD_MOVEFOLDER;
    sscanf(buffer, "%*s %s %s %s", cmd->payload.movefolder.group,
           cmd->payload.movefolder.source, cmd->payload.movefolder.destination);
    return CMD_MOVEFOLDER;
  } else if (strcmp(command, "CREATE_GROUP") == 0) {
    cmd->type = CMD_CREATE_GROUP;
    sscanf(buffer, "%*s %s %s", cmd->payload.group.group_name,
           cmd->payload.group.user_name);
    return CMD_CREATE_GROUP;
  } else if (strcmp(command, "LIST_GROUPS") == 0) {
    cmd->type = CMD_LIST_GROUPS;
    sscanf(buffer, "%*s %s", cmd->payload.group.user_name);
    return CMD_LIST_GROUPS;
  } else if (strcmp(command, "LIST_MEMBERS") == 0) {
    cmd->type = CMD_LIST_MEMBERS;
    sscanf(buffer, "%*s %s", cmd->payload.group.group_name);
    return CMD_LIST_MEMBERS;
  } else if (strcmp(command, "JOIN_REQ") == 0) {
    cmd->type = CMD_JOIN_REQ;
    sscanf(buffer, "%*s %s", cmd->payload.group.group_name);
    return CMD_JOIN_REQ;
  } else if (strcmp(command, "APPROVE_JOIN") == 0) {
    cmd->type = CMD_APPROVE_JOIN;
    sscanf(buffer, "%*s %s %s", cmd->payload.group.group_name,
           cmd->payload.group.user_name);
    return CMD_APPROVE_JOIN;
  } else if (strcmp(command, "INVITE_USER") == 0) {
    cmd->type = CMD_INVITE_USER;
    sscanf(buffer, "%*s %s %s", cmd->payload.group.group_name,
           cmd->payload.group.user_name);
    return CMD_INVITE_USER;
  } else if (strcmp(command, "ACCEPT_INVITE") == 0) {
    cmd->type = CMD_ACCEPT_INVITE;
    sscanf(buffer, "%*s %s %s", cmd->payload.group.group_name,
           cmd->payload.group.status);
    return CMD_ACCEPT_INVITE;
  }
  cmd->type = CMD_UNKNOWN;
  return CMD_UNKNOWN;
}
