#include "../include/logger.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static FILE *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static char log_file_path_buffer[256];

void logger_init(const char *log_file_path) {
  pthread_mutex_lock(&log_mutex);

  // Store log file path
  strncpy(log_file_path_buffer, log_file_path,
          sizeof(log_file_path_buffer) - 1);
  log_file_path_buffer[sizeof(log_file_path_buffer) - 1] = '\0';

  // Open log file in append mode
  log_file = fopen(log_file_path, "a");
  if (log_file == NULL) {
    perror("Failed to open log file");
    pthread_mutex_unlock(&log_mutex);
    return;
  }

  // Write initialization message
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  char time_buffer[64];
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

  fprintf(log_file, "\n=== Server started at %s ===\n", time_buffer);
  fflush(log_file);

  pthread_mutex_unlock(&log_mutex);
}

void logger_cleanup(void) {
  pthread_mutex_lock(&log_mutex);

  if (log_file != NULL) {
    // Write shutdown message
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buffer[64];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log_file, "=== Server stopped at %s ===\n\n", time_buffer);
    fflush(log_file);

    fclose(log_file);
    log_file = NULL;
  }

  pthread_mutex_unlock(&log_mutex);
}

void log_command(const char *username, const char *command) {
  if (log_file == NULL) {
    return; // Logger not initialized
  }

  pthread_mutex_lock(&log_mutex);

  // Get current time
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);
  char time_buffer[64];
  strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

  // Write log entry: <time> <username> <command>
  fprintf(log_file, "%s %s %s\n", time_buffer, username ? username : "ANONYMOUS",
          command ? command : "UNKNOWN");
  fflush(log_file); // Ensure log is written immediately

  pthread_mutex_unlock(&log_mutex);
}
