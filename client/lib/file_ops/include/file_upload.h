#ifndef FILE_UPLOAD_H
#define FILE_UPLOAD_H

/**
 * @brief Upload a file to the server
 *
 * This function handles the complete upload workflow:
 * 1. Opens the local file and gets its size
 * 2. Sends UPLOAD command with client path, server path, and size
 * 3. Waits for server's UPLOAD_READY response
 * 4. Streams file data in chunks
 * 5. Waits for server's UPLOAD_COMPLETE confirmation
 *
 * @param client_socket Socket connected to server
 * @param filepath Path to the local file to upload
 * @param server_path Server-side path where file should be stored (e.g.,
 * "storage/", "storage/group1/")
 * @return 0 on success, -1 on error
 */
int file_upload(int client_socket, const char *filepath,
                const char *server_path);

#endif // FILE_UPLOAD_H
