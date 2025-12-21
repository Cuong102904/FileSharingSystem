#include "../include/session.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SESSIONS 1024

static Session *session_pool[MAX_SESSIONS];
static pthread_rwlock_t pool_lock = PTHREAD_RWLOCK_INITIALIZER;

// Helper to generate random session ID
static void generate_session_id(char *buffer, size_t length) {
  const char charset[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  for (size_t i = 0; i < length - 1; i++) {
    int key = rand() % (int)(sizeof(charset) - 1);
    buffer[i] = charset[key];
  }
  buffer[length - 1] = '\0';
}

void session_system_init(void) {
  srand(time(NULL));
  for (int i = 0; i < MAX_SESSIONS; i++) {
    session_pool[i] = NULL;
  }
}

Session *session_create(int socket_fd, struct sockaddr_in *addr) {
  Session *s = (Session *)malloc(sizeof(Session));
  if (!s)
    return NULL;

  memset(s, 0, sizeof(Session));
  s->socket_fd = socket_fd;
  if (addr)
    s->client_addr = *addr;
  s->created_at = time(NULL);
  s->last_active = time(NULL);
  s->is_logged_in = false;
  pthread_rwlock_init(&s->lock, NULL);

  // Generate temp session ID (will be finalized on login usually, but useful
  // for tracking)
  generate_session_id(s->session_id, SESSION_ID_LEN);

  // Add to pool
  pthread_rwlock_wrlock(&pool_lock);
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (session_pool[i] == NULL) {
      session_pool[i] = s;
      pthread_rwlock_unlock(&pool_lock);
      return s;
    }
  }
  pthread_rwlock_unlock(&pool_lock);

  // Pool full
  free(s);
  return NULL;
}

Session *session_find_by_socket(int socket_fd) {
  pthread_rwlock_rdlock(&pool_lock);
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (session_pool[i] && session_pool[i]->socket_fd == socket_fd) {
      Session *s = session_pool[i];
      pthread_rwlock_unlock(&pool_lock);
      return s;
    }
  }
  pthread_rwlock_unlock(&pool_lock);
  return NULL;
}

Session *session_find_by_id(const char *session_id) {
  if (!session_id)
    return NULL;
  pthread_rwlock_rdlock(&pool_lock);
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (session_pool[i] &&
        strcmp(session_pool[i]->session_id, session_id) == 0) {
      Session *s = session_pool[i];
      pthread_rwlock_unlock(&pool_lock);
      return s;
    }
  }
  pthread_rwlock_unlock(&pool_lock);
  return NULL;
}

void session_login(Session *s, const char *username) {
  if (!s || !username)
    return;

  pthread_rwlock_wrlock(&s->lock);
  strncpy(s->username, username, USERNAME_LEN - 1);
  // Regenerate session ID on login for security
  generate_session_id(s->session_id, SESSION_ID_LEN);
  s->is_logged_in = true;
  s->last_active = time(NULL);
  pthread_rwlock_unlock(&s->lock);
}

void session_remove_by_socket(int socket_fd) {
  pthread_rwlock_wrlock(&pool_lock);
  for (int i = 0; i < MAX_SESSIONS; i++) {
    if (session_pool[i] && session_pool[i]->socket_fd == socket_fd) {
      Session *s = session_pool[i];
      session_pool[i] = NULL; // Remove from pool
      pthread_rwlock_unlock(&pool_lock);

      pthread_rwlock_destroy(&s->lock);
      free(s);
      return;
    }
  }
  pthread_rwlock_unlock(&pool_lock);
}

void session_destroy(Session *s) {
  if (!s)
    return;
  session_remove_by_socket(s->socket_fd);
}
