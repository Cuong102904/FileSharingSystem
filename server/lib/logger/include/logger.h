#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>

// Initialize logger system
void logger_init(const char *log_file_path);

// Cleanup logger system
void logger_cleanup(void);

// Log a command execution
// Format: <time> <username> <command>
// Example: 2024-01-09 08:00:15 duongtt LIST_GROUPS
void log_command(const char *username, const char *command);

#endif // LOGGER_H
