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
  } else if (strcmp(command, "CREATE_GROUP") == 0) {
    cmd->type = CMD_CREATE_GROUP;
    sscanf(buffer, "%*s %255s", cmd->payload.create_group.group_name);
    return CMD_CREATE_GROUP;
  } else if (strcmp(command, "LIST_GROUPS") == 0) {
    cmd->type = CMD_LIST_GROUPS;
    return CMD_LIST_GROUPS;
  } else if (strcmp(command, "LIST_MEMBERS") == 0) {
    cmd->type = CMD_LIST_MEMBERS;
    sscanf(buffer, "%*s %63s", cmd->payload.list_members.group_id);
    return CMD_LIST_MEMBERS;
  } else if (strcmp(command, "JOIN_REQ") == 0) {
    cmd->type = CMD_JOIN_REQ;
    sscanf(buffer, "%*s %63s", cmd->payload.join_req.group_id);
    return CMD_JOIN_REQ;
  } else if (strcmp(command, "APPROVE_JOIN") == 0) {
    cmd->type = CMD_APPROVE_JOIN;
    sscanf(buffer, "%*s %63s %63s", cmd->payload.approve_join.group_id,
           cmd->payload.approve_join.user_id);
    return CMD_APPROVE_JOIN;
  } else if (strcmp(command, "INVITE_USER") == 0) {
    cmd->type = CMD_INVITE_USER;
    sscanf(buffer, "%*s %63s %255s", cmd->payload.invite_user.group_id,
           cmd->payload.invite_user.username);
    return CMD_INVITE_USER;
  } else if (strcmp(command, "LEAVE_GROUP") == 0) {
    cmd->type = CMD_LEAVE_GROUP;
    sscanf(buffer, "%*s %63s", cmd->payload.leave_group.group_id);
    return CMD_LEAVE_GROUP;
  } else if (strcmp(command, "KICK_MEMBER") == 0) {
    cmd->type = CMD_KICK_MEMBER;
    sscanf(buffer, "%*s %255s %63s %63s", cmd->payload.kick_member.session_id,
           cmd->payload.kick_member.group_id, cmd->payload.kick_member.user_id);
    return CMD_KICK_MEMBER;
  }

  cmd->type = CMD_UNKNOWN;
  return CMD_UNKNOWN;
}
