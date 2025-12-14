#include "../include/protocol.h"
#include "../../auth/include/auth.h"
#include "../../session/include/session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

void send_response(int client_socket, const char *response) {
    send(client_socket, response, strlen(response), 0);
}

void handle_register(int client_socket, const char *username, const char *password) {
    char response[BUFFER_SIZE];

    int result = register_user(username, password);

    switch (result) {
        case AUTH_SUCCESS:
            strcpy(response, RESP_OK_REGISTER);
            break;
        case AUTH_USER_EXISTS:
            strcpy(response, RESP_ERR_ACCOUNT_EXISTS);
            break;
        default:
            strcpy(response, RESP_ERR_DB_ERROR);
            break;
    }

    send_response(client_socket, response);
}

void handle_login(int client_socket, const char *username, const char *password) {
    char response[BUFFER_SIZE];

    int result = authenticate_user(username, password);

    switch (result) {
        case AUTH_SUCCESS: {
            char *session_id = session_create(username);
            if (session_id != NULL) {
                snprintf(response, BUFFER_SIZE, "%s %s", RESP_OK_LOGIN, session_id);
                free(session_id);
            } else {
                strcpy(response, RESP_ERR_SERVER_FULL);
            }
            break;
        }
        case AUTH_WRONG_PASSWORD:
            strcpy(response, RESP_ERR_WRONG_PASSWORD);
            break;
        case AUTH_USER_NOT_FOUND:
            strcpy(response, RESP_ERR_USER_NOT_FOUND);
            break;
        default:
            strcpy(response, RESP_ERR_DB_ERROR);
            break;
    }

    send_response(client_socket, response);
}

void handle_logout(int client_socket, const char *session_id) {
    char response[BUFFER_SIZE];

    int result = session_destroy(session_id);

    if (result == SESSION_SUCCESS) {
        strcpy(response, RESP_OK_LOGOUT);
    } else {
        strcpy(response, RESP_ERR_INVALID_SESSION);
    }

    send_response(client_socket, response);
}
