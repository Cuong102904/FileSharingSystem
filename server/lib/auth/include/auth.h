#ifndef AUTH_H
#define AUTH_H

#include <pthread.h>
#include "group.h"

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define DB_FILE "database/users.txt"

// Shared mutex for database operations
extern pthread_mutex_t user_db_mutex;

// User structure
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int is_active;
} User;

// Auth result codes
typedef enum {
    AUTH_SUCCESS = 1,
    AUTH_USER_EXISTS = 0,
    AUTH_USER_NOT_FOUND = -1,
    AUTH_WRONG_PASSWORD = -2,
    AUTH_DB_ERROR = -3
} AuthResult;

// Initialize auth module
void auth_init(void);

// Cleanup auth module
void auth_cleanup(void);

// Register functions
int register_user(const char *username, const char *password);
int check_user_exists(const char *username);

// Login functions
int authenticate_user(const char *username, const char *password);

// User management functions
User* get_user_by_username(const char *username);
void free_user(User *user);

#endif // AUTH_H
