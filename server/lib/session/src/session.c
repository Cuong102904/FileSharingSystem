#include "../include/session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static Session sessions[MAX_SESSIONS];
static pthread_mutex_t session_mutex = PTHREAD_MUTEX_INITIALIZER;

void session_init(void) {
    pthread_mutex_init(&session_mutex, NULL);
    memset(sessions, 0, sizeof(sessions));
}

void session_cleanup(void) {
    pthread_mutex_destroy(&session_mutex);
}

void session_generate_id(char *session_id) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand(time(NULL) + rand());

    for (int i = 0; i < SESSION_ID_LEN; i++) {
        session_id[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    session_id[SESSION_ID_LEN] = '\0';
}

char* session_create(const char *username) {
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!sessions[i].active) {
            strcpy(sessions[i].username, username);
            session_generate_id(sessions[i].session_id);
            sessions[i].active = 1;
            sessions[i].created_at = time(NULL);

            char *session_id = malloc(SESSION_ID_LEN + 1);
            if (session_id == NULL) {
                pthread_mutex_unlock(&session_mutex);
                return NULL;
            }
            strcpy(session_id, sessions[i].session_id);

            pthread_mutex_unlock(&session_mutex);
            return session_id;
        }
    }

    pthread_mutex_unlock(&session_mutex);
    return NULL;
}

int session_destroy(const char *session_id) {
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && strcmp(sessions[i].session_id, session_id) == 0) {
            sessions[i].active = 0;
            memset(sessions[i].username, 0, sizeof(sessions[i].username));
            memset(sessions[i].session_id, 0, sizeof(sessions[i].session_id));

            pthread_mutex_unlock(&session_mutex);
            return SESSION_SUCCESS;
        }
    }

    pthread_mutex_unlock(&session_mutex);
    return SESSION_NOT_FOUND;
}

int session_validate(const char *session_id) {
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && strcmp(sessions[i].session_id, session_id) == 0) {
            pthread_mutex_unlock(&session_mutex);
            return SESSION_SUCCESS;
        }
    }

    pthread_mutex_unlock(&session_mutex);
    return SESSION_NOT_FOUND;
}

char* session_get_username(const char *session_id) {
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && strcmp(sessions[i].session_id, session_id) == 0) {
            char *username = malloc(50);
            if (username == NULL) {
                pthread_mutex_unlock(&session_mutex);
                return NULL;
            }
            strcpy(username, sessions[i].username);

            pthread_mutex_unlock(&session_mutex);
            return username;
        }
    }

    pthread_mutex_unlock(&session_mutex);
    return NULL;
}
