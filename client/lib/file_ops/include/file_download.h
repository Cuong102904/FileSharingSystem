#ifndef FILE_DOWNLOAD_H
#define FILE_DOWNLOAD_H

/**
 * Download file from server
 *
 * Protocol:
 * 1. Sends DOWNLOAD command with group and server path
 * 2. Waits for server's OK DOWNLOAD response with filesize
 * 3. Receives file data in chunks
 * 4. Saves to local path
 *
 * Returns: 0 on success, -1 on error
 */
int file_download(int client_socket, const char *group_name,
                  const char *server_path, const char *local_path);

#endif // FILE_DOWNLOAD_H
