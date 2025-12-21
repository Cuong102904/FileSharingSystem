#include "../include/server.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../lib/protocol/include/protocol.h"
#include "../lib/session/include/session.h"
#include "../lib/utils/include/thread_pool.h"

#define MAX_EVENTS 64
#define PORT 8080
#define BUFFER_SIZE 1024

// Global Thread Pool
ThreadPool *thread_pool = NULL;

void set_nonblocking(int socket_fd) {
  int flags = fcntl(socket_fd, F_GETFL, 0);
  fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

int setup_server() {
  int server_fd;
  struct sockaddr_in address;
  int opt = 1;

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  return server_fd;
}

void process_command(int client_fd, char *buffer) {
  ParsedCommand cmd;
  CommandType type = protocol_parse_command(buffer, &cmd);

  if (type == CMD_REGISTER) {
    handle_register(client_fd, cmd.payload.auth.username,
                    cmd.payload.auth.password);
  } else if (type == CMD_LOGIN) {
    handle_login(client_fd, cmd.payload.auth.username,
                 cmd.payload.auth.password);
  } else if (type == CMD_LOGOUT) {
    handle_logout(client_fd, cmd.payload.session.session_id);
  } else if (type == CMD_UPLOAD) {
    // Find session by ID provided in command
    Session *auth_s = session_find_by_id(cmd.session_id);
    if (!auth_s) {
      send_response(client_fd, "ERROR Invalid session");
      return;
    }

    // This is a blocking operation, offload to Thread Pool
    WorkJob job;
    job.type = JOB_UPLOAD;
    job.session = auth_s;
    snprintf(job.arg1, sizeof(job.arg1), "%s", cmd.payload.upload.group);
    snprintf(job.arg2, sizeof(job.arg2), "%s", cmd.payload.upload.client_path);
    snprintf(job.arg3, sizeof(job.arg3), "%s", cmd.payload.upload.server_path);

    threadpool_add_job(thread_pool, job);
  } else {
    send_response(client_fd, RESP_ERR_UNKNOWN_CMD);
  }
}

int main() {
  int server_fd = setup_server();
  printf("Server listening on port %d with EPOLL\n", PORT);

  // Init Subsystems
  session_system_init();
  thread_pool = threadpool_create(4); // 4 workers

  // Epoll Init
  int epoll_fd = epoll_create1(0);
  if (epoll_fd == -1) {
    perror("epoll_create1");
    exit(1);
  }

  struct epoll_event ev, events[MAX_EVENTS];
  ev.events = EPOLLIN;
  ev.data.fd = server_fd;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
    perror("epoll_ctl: server_socket");
    exit(1);
  }

  while (1) {
    int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if (nfds == -1) {
      perror("epoll_wait");
      exit(1);
    }

    for (int i = 0; i < nfds; ++i) {
      if (events[i].data.fd == server_fd) {
        // Accept new connection
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd =
            accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd == -1) {
          perror("accept");
          continue;
        }

        set_nonblocking(client_fd);

        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = client_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
          perror("epoll_ctl: client_socket");
          close(client_fd);
        }

        // Create Session
        session_create(client_fd, &client_addr);
        printf("New connection: %d\n", client_fd);
      } else {
        // Handle client data
        int client_fd = events[i].data.fd;
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

        if (bytes_read <= 0) {
          // Closed connection
          if (bytes_read == 0) {
            printf("Client %d disconnected\n", client_fd);
          } else if (errno != EAGAIN) {
            perror("recv");
          }

          session_remove_by_socket(client_fd);
          close(client_fd);
          epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        } else {
          buffer[bytes_read] = '\0';
          process_command(client_fd, buffer);
        }
      }
    }
  }

  return 0;
}
