#ifndef SESSION_H
#define SESSION_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define SESSION_ID_LEN 64
#define USERNAME_LEN 256

typedef struct {
  char session_id[SESSION_ID_LEN];
  char username[USERNAME_LEN];
  int socket_fd;
  struct sockaddr_in client_addr;
  time_t created_at;
  time_t last_active;
  pthread_rwlock_t lock; // Protect individual session data
  bool is_logged_in;
} Session;

// Initialize session system
void session_system_init(void);

// Create a new session for a socket
Session *session_create(int socket_fd, struct sockaddr_in *addr);

// Find session by socket (O(1) lookup map or simple array scan for now)
Session *session_find_by_socket(int socket_fd);

// Find session by ID
Session *session_find_by_id(const char *session_id);

// Associate username with session (Login)
void session_login(Session *s, const char *username);

// Destroy session
void session_destroy(Session *s);

// Remove session by socket
void session_remove_by_socket(int socket_fd);

// Cleanup idle sessions
void session_cleanup_idle(int timeout_seconds);

#endif // SESSION_H
