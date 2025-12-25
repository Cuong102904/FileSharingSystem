#include "../include/client_session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Client session entry
typedef struct {
    int client_socket;
    char username[MAX_USERNAME_LEN];
    int active;
} ClientSessionEntry;

// Session storage
static ClientSessionEntry sessions[MAX_CLIENT_SESSIONS];
static pthread_mutex_t session_mutex = PTHREAD_MUTEX_INITIALIZER;

void client_session_init(void) {
    pthread_mutex_lock(&session_mutex);
    for (int i = 0; i < MAX_CLIENT_SESSIONS; i++) {
        sessions[i].client_socket = -1;
        sessions[i].username[0] = '\0';
        sessions[i].active = 0;
    }
    pthread_mutex_unlock(&session_mutex);
    printf("Client session module initialized.\n");
}

void client_session_cleanup(void) {
    pthread_mutex_lock(&session_mutex);
    for (int i = 0; i < MAX_CLIENT_SESSIONS; i++) {
        sessions[i].active = 0;
    }
    pthread_mutex_unlock(&session_mutex);
    printf("Client session module cleaned up.\n");
}

int client_session_login(int client_socket, const char *username) {
    if (username == NULL || client_socket < 0) {
        return -1;
    }

    pthread_mutex_lock(&session_mutex);

    // Check if already logged in
    for (int i = 0; i < MAX_CLIENT_SESSIONS; i++) {
        if (sessions[i].active && sessions[i].client_socket == client_socket) {
            pthread_mutex_unlock(&session_mutex);
            return -1; // Already logged in
        }
    }

    // Find empty slot
    for (int i = 0; i < MAX_CLIENT_SESSIONS; i++) {
        if (!sessions[i].active) {
            sessions[i].client_socket = client_socket;
            strncpy(sessions[i].username, username, MAX_USERNAME_LEN - 1);
            sessions[i].username[MAX_USERNAME_LEN - 1] = '\0';
            sessions[i].active = 1;
            pthread_mutex_unlock(&session_mutex);
            printf("Client session created: socket %d -> user '%s'\n", client_socket, username);
            return 0;
        }
    }

    pthread_mutex_unlock(&session_mutex);
    return -1; // No empty slot
}

void client_session_logout(int client_socket) {
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_CLIENT_SESSIONS; i++) {
        if (sessions[i].active && sessions[i].client_socket == client_socket) {
            printf("Client session removed: socket %d -> user '%s'\n",
                   client_socket, sessions[i].username);
            sessions[i].active = 0;
            sessions[i].client_socket = -1;
            sessions[i].username[0] = '\0';
            break;
        }
    }

    pthread_mutex_unlock(&session_mutex);
}

const char *client_session_get_username(int client_socket) {
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_CLIENT_SESSIONS; i++) {
        if (sessions[i].active && sessions[i].client_socket == client_socket) {
            pthread_mutex_unlock(&session_mutex);
            return sessions[i].username;
        }
    }

    pthread_mutex_unlock(&session_mutex);
    return NULL;
}

int client_session_is_logged_in(int client_socket) {
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_CLIENT_SESSIONS; i++) {
        if (sessions[i].active && sessions[i].client_socket == client_socket) {
            pthread_mutex_unlock(&session_mutex);
            return 1;
        }
    }

    pthread_mutex_unlock(&session_mutex);
    return 0;
}
