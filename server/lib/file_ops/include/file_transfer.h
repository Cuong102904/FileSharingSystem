#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

// Handle Upload: Receive file from client
void handle_upload(int client_socket, const char *path,
                   const char *filesize_str);

#endif
