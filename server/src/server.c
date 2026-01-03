#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef __APPLE__
#include <sys/event.h>
#else
#include <sys/epoll.h>
#endif

#include "../lib/auth/include/auth.h"
#include "../lib/client_session/include/client_session.h"
#include "../lib/group/include/group.h"
#include "../lib/group/include/group_repo.h"
#include "../lib/protocol/include/protocol.h"
#include "../lib/session/include/session.h"
#include "../lib/thread_pool/include/thread_pool.h"

#define PORT 8080
#define MAX_CLIENTS 100
#define THREAD_POOL_SIZE 10
#define MAX_EVENTS 64

// Global thread pool
static ThreadPool *g_thread_pool = NULL;

static void server_init(void) {
  auth_init();
  session_init();
  client_session_init();
  printf("Server modules initialized.\n");
}

static void server_cleanup(void) {
  client_session_cleanup();
  auth_cleanup();
  session_cleanup();
  printf("Server modules cleaned up.\n");
}

// Task argument structure
typedef struct {
  int client_socket;
  char buffer[BUFFER_SIZE];
  int buffer_len;
} ClientTask;

// Set socket to non-blocking mode
static int set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    return -1;
  }
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Process a single request (runs in thread pool)
static void process_request(void *arg) {
  ClientTask *task = (ClientTask *)arg;
  int client_socket = task->client_socket;
  char *buffer = task->buffer;
  ParsedCommand cmd;

  // Parse and handle command
  CommandType cmd_type = protocol_parse_command(buffer, &cmd);

  switch (cmd_type) {
  case CMD_REGISTER:
    handle_register(client_socket, cmd.payload.auth.username,
                    cmd.payload.auth.password);
    break;
  case CMD_LOGIN:
    handle_login(client_socket, cmd.payload.auth.username,
                 cmd.payload.auth.password);
    break;
  case CMD_LOGOUT:
    handle_logout(client_socket, cmd.payload.session.session_id);
    break;
  case CMD_UPLOAD:
    handle_upload(client_socket, cmd.payload.upload.group,
                  cmd.payload.upload.local_path,
                  cmd.payload.upload.remote_path);
    break;
  case CMD_DOWNLOAD:
    handle_download(client_socket, cmd.payload.download.group,
                    cmd.payload.download.path);
    break;
  case CMD_CREATE_GROUP:
    handle_create_group(client_socket, cmd.payload.group.group_name);
    break;
  case CMD_LIST_GROUPS:
    handle_list_groups_by_user(client_socket);
    break;
  default:
    // Only send error if buffer has actual content
    if (strlen(buffer) > 0 && buffer[0] != '\n' && buffer[0] != '\r') {
      send_response(client_socket, RESP_ERR_UNKNOWN_CMD);
    }
    break;
  }

  free(task);
}

#ifdef __APPLE__
// macOS uses kqueue
static int run_event_loop(int server_socket) {
  int kq = kqueue();
  if (kq == -1) {
    perror("kqueue creation failed");
    return -1;
  }

  // Register server socket for read events
  struct kevent change;
  EV_SET(&change, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
  if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
    perror("kevent register failed");
    close(kq);
    return -1;
  }

  struct kevent events[MAX_EVENTS];
  printf("Event loop started (kqueue)\n");

  while (1) {
    int nev = kevent(kq, NULL, 0, events, MAX_EVENTS, NULL);
    if (nev == -1) {
      if (errno == EINTR) {
        continue;
      }
      perror("kevent wait failed");
      break;
    }

    for (int i = 0; i < nev; i++) {
      int fd = (int)events[i].ident;

      if (events[i].flags & EV_EOF) {
        // Client disconnected - cleanup session
        client_session_logout(fd);
        printf("Client disconnected: socket %d\n", fd);
        close(fd);
        continue;
      }

      if (fd == server_socket) {
        // New connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket =
            accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket == -1) {
          if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Accept failed");
          }
          continue;
        }

        set_nonblocking(client_socket);

        // Register client socket for read events
        EV_SET(&change, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
               NULL);
        if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
          perror("kevent register client failed");
          close(client_socket);
          continue;
        }

        printf("Client connected: socket %d\n", client_socket);
      } else {
        // Data from client
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = recv(fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_read <= 0) {
          if (bytes_read == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
            client_session_logout(fd);
            printf("Client disconnected: socket %d\n", fd);
            close(fd);
          }
          continue;
        }

        buffer[bytes_read] = '\0';

        // Create task and add to thread pool
        ClientTask *task = malloc(sizeof(ClientTask));
        if (task == NULL) {
          send_response(fd, "ERROR Server out of memory");
          continue;
        }

        task->client_socket = fd;
        memcpy(task->buffer, buffer, bytes_read + 1);
        task->buffer_len = bytes_read;

        if (thread_pool_add_task(g_thread_pool, process_request, task) != 0) {
          send_response(fd, "ERROR Server busy");
          free(task);
        }
      }
    }
  }

  close(kq);
  return 0;
}

#else
// Linux uses epoll
static int run_event_loop(int server_socket) {
  int epfd = epoll_create1(0);
  if (epfd == -1) {
    perror("epoll_create1 failed");
    return -1;
  }

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = server_socket;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &ev) == -1) {
    perror("epoll_ctl add server failed");
    close(epfd);
    return -1;
  }

  struct epoll_event events[MAX_EVENTS];
  printf("Event loop started (epoll)\n");

  while (1) {
    int nev = epoll_wait(epfd, events, MAX_EVENTS, -1);
    if (nev == -1) {
      if (errno == EINTR) {
        continue;
      }
      perror("epoll_wait failed");
      break;
    }

    for (int i = 0; i < nev; i++) {
      int fd = events[i].data.fd;

      if (events[i].events & (EPOLLHUP | EPOLLERR)) {
        client_session_logout(fd);
        printf("Client disconnected: socket %d\n", fd);
        close(fd);
        continue;
      }

      if (fd == server_socket) {
        // New connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket =
            accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket == -1) {
          if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Accept failed");
          }
          continue;
        }

        set_nonblocking(client_socket);

        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = client_socket;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &ev) == -1) {
          perror("epoll_ctl add client failed");
          close(client_socket);
          continue;
        }

        printf("Client connected: socket %d\n", client_socket);
      } else {
        // Data from client
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = recv(fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_read <= 0) {
          if (bytes_read == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
            client_session_logout(fd);
            printf("Client disconnected: socket %d\n", fd);
            epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
          }
          continue;
        }

        buffer[bytes_read] = '\0';

        // Create task and add to thread pool
        ClientTask *task = malloc(sizeof(ClientTask));
        if (task == NULL) {
          send_response(fd, "ERROR Server out of memory");
          continue;
        }

        task->client_socket = fd;
        memcpy(task->buffer, buffer, bytes_read + 1);
        task->buffer_len = bytes_read;

        if (thread_pool_add_task(g_thread_pool, process_request, task) != 0) {
          send_response(fd, "ERROR Server busy");
          free(task);
        }
      }
    }
  }

  close(epfd);
  return 0;
}
#endif

int main() {
  int server_socket;
  struct sockaddr_in server_addr;

  // Initialize all modules
  server_init();

  // Create thread pool
  g_thread_pool = thread_pool_create(THREAD_POOL_SIZE);
  if (g_thread_pool == NULL) {
    fprintf(stderr, "Failed to create thread pool\n");
    exit(EXIT_FAILURE);
  }

  // Create socket
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    perror("Socket creation failed");
    thread_pool_destroy(g_thread_pool);
    exit(EXIT_FAILURE);
  }

  // Allow socket reuse
  int opt = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // Set non-blocking
  set_nonblocking(server_socket);

  // Configure server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // Bind socket
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    perror("Bind failed");
    close(server_socket);
    thread_pool_destroy(g_thread_pool);
    exit(EXIT_FAILURE);
  }

  // Listen
  if (listen(server_socket, MAX_CLIENTS) < 0) {
    perror("Listen failed");
    close(server_socket);
    thread_pool_destroy(g_thread_pool);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d (IO Multiplexing + Thread Pool)\n", PORT);

  // Run event loop
  run_event_loop(server_socket);

  // Cleanup
  thread_pool_destroy(g_thread_pool);
  server_cleanup();
  close(server_socket);

  return 0;
}
