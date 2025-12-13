#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100
#define DB_FILE "database/users.txt"

// Data structure store session's information
typedef struct {
    char username[50];
    char session_id[65];
    int active;
} Session;

Session sessions[MAX_CLIENTS];
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t session_mutex = PTHREAD_MUTEX_INITIALIZER;

// Create session id randomlly
void generate_session_id(char *session_id) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand(time(NULL) + rand());

    for (int i = 0; i < 64; i++) {
        session_id[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    session_id[64] = '\0';
}

// Check username exists or not
int check_user_exists(const char *username) {
    pthread_mutex_lock(&db_mutex);

    FILE *file = fopen(DB_FILE, "r");
    if (file == NULL) {
        pthread_mutex_unlock(&db_mutex);
        return 0; // Database not exists, return error
    }

    char line[256];
    char stored_username[50];

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s", stored_username);
        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            pthread_mutex_unlock(&db_mutex);
            return 1; // Username existed
        }
    }

    fclose(file);
    pthread_mutex_unlock(&db_mutex);
    return 0; // Username not existed
}

// Register new user
int register_user(const char *username, const char *password) {
    if (check_user_exists(username)) {
        return 0; // Username existed
    }

    pthread_mutex_lock(&db_mutex);

    FILE *file = fopen(DB_FILE, "a");
    if (file == NULL) {
        pthread_mutex_unlock(&db_mutex);
        return -1; // Error in opening file
    }

    fprintf(file, "%s %s\n", username, password);
    fclose(file);

    pthread_mutex_unlock(&db_mutex);
    return 1; // Register successfully
}

// Authentication for login
int authenticate_user(const char *username, const char *password) {
    pthread_mutex_lock(&db_mutex);

    FILE *file = fopen(DB_FILE, "r");
    if (file == NULL) {
        pthread_mutex_unlock(&db_mutex);
        return 0; // File doesn't exist
    }

    char line[256];
    char stored_username[50];
    char stored_password[50];

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %s", stored_username, stored_password);
        if (strcmp(stored_username, username) == 0) {
            fclose(file);
            pthread_mutex_unlock(&db_mutex);
            if (strcmp(stored_password, password) == 0) {
                return 1; // Login successfully
            } else {
                return -1; // Wrong password
            }
        }
    }

    fclose(file);
    pthread_mutex_unlock(&db_mutex);
    return 0; // User doesn't exist
}

// Create new session id
char* create_session(const char *username) {
    pthread_mutex_lock(&session_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!sessions[i].active) {
            strcpy(sessions[i].username, username);
            generate_session_id(sessions[i].session_id);
            sessions[i].active = 1;

            char *session_id = malloc(65);
            strcpy(session_id, sessions[i].session_id);

            pthread_mutex_unlock(&session_mutex);
            return session_id;
        }
    }

    pthread_mutex_unlock(&session_mutex);
    return NULL;
}

// Handle REGISTER command
void handle_register(int client_socket, const char *username, const char *password) {
    char response[BUFFER_SIZE];

    int result = register_user(username, password);

    if (result == 1) {
        strcpy(response, "OK REGISTER");
    } else if (result == 0) {
        strcpy(response, "ERROR Account already exists");
    } else {
        strcpy(response, "ERROR Database error");
    }

    send(client_socket, response, strlen(response), 0);
}

// Handle LOGIN command
void handle_login(int client_socket, const char *username, const char *password) {
    char response[BUFFER_SIZE];

    int result = authenticate_user(username, password);

    if (result == 1) {
        char *session_id = create_session(username);
        if (session_id != NULL) {
            snprintf(response, BUFFER_SIZE, "OK LOGIN %s", session_id);
            free(session_id);
        } else {
            strcpy(response, "ERROR Server full");
        }
    } else if (result == -1) {
        strcpy(response, "ERROR Wrong password");
    } else {
        strcpy(response, "ERROR User not found");
    }

    send(client_socket, response, strlen(response), 0);
}

// Handle LOGOUT command
void handle_logout(int client_socket, const char *session_id) {
    char response[BUFFER_SIZE];

    pthread_mutex_lock(&session_mutex);

    int found = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (sessions[i].active && strcmp(sessions[i].session_id, session_id) == 0) {
            sessions[i].active = 0;
            memset(sessions[i].username, 0, sizeof(sessions[i].username));
            memset(sessions[i].session_id, 0, sizeof(sessions[i].session_id));
            found = 1;
            break;
        }
    }

    pthread_mutex_unlock(&session_mutex);

    if (found) {
        strcpy(response, "OK LOGOUT");
    } else {
        strcpy(response, "ERROR Invalid session");
    }

    send(client_socket, response, strlen(response), 0);
}

// Handle client
void* handle_client(void *arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    int bytes_received;

    printf("Client connected: socket %d\n", client_socket);

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';

        printf("Received from client %d: %s\n", client_socket, buffer);

        // Parse command
        char command[20];
        char username[50];
        char password[50];
        char session_id[65];

        sscanf(buffer, "%s", command);

        if (strcmp(command, "REGISTER") == 0) {
            sscanf(buffer, "%s %s %s", command, username, password);
            handle_register(client_socket, username, password);
        }
        else if (strcmp(command, "LOGIN") == 0) {
            sscanf(buffer, "%s %s %s", command, username, password);
            handle_login(client_socket, username, password);
        }
        else if (strcmp(command, "LOGOUT") == 0) {
            sscanf(buffer, "%s %s", command, session_id);
            handle_logout(client_socket, session_id);
        }
        else {
            char response[] = "ERROR Unknown command";
            send(client_socket, response, strlen(response), 0);
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

    printf("Client disconnected: socket %d\n", client_socket);
    close(client_socket);

    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Init sessions
    memset(sessions, 0, sizeof(sessions));

    // Init socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Config IP address of server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Create new thread for client
        pthread_t thread_id;
        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;

        if (pthread_create(&thread_id, NULL, handle_client, pclient) != 0) {
            perror("Thread creation failed");
            close(client_socket);
            free(pclient);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}
