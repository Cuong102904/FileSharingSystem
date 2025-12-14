#ifndef SESSION_H
#define SESSION_H

#include <pthread.h>

#define MAX_SESSIONS 100
#define SESSION_ID_LEN 64

// Session structure
typedef struct {
    char username[50];
    char session_id[SESSION_ID_LEN + 1];
    int active;
    long created_at;
} Session;

// Session result codes
typedef enum {
    SESSION_SUCCESS = 1,
    SESSION_NOT_FOUND = 0,
    SESSION_FULL = -1,
    SESSION_ERROR = -2
} SessionResult;

// Initialize session module
void session_init(void);

// Cleanup session module
void session_cleanup(void);

// Create a new session for user
char* session_create(const char *username);

// Destroy session by session_id
int session_destroy(const char *session_id);

// Validate session
int session_validate(const char *session_id);

// Get username from session_id
char* session_get_username(const char *session_id);

// Generate random session ID
void session_generate_id(char *session_id);

#endif // SESSION_H
