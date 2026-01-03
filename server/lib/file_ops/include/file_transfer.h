#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

/**
 * @brief Receive file data from socket and save to disk
 *
 * Pure file I/O function - no protocol handling.
 * Streams data from socket in chunks and writes to file.
 *
 * @param client_socket Socket to receive data from
 * @param save_path Full path where file should be saved (e.g.,
 * "storage/group1/file.txt")
 * @param filesize Expected file size in bytes
 * @return Total bytes received on success, -1 on error
 */
long receive_file(int client_socket, const char *save_path, long filesize);
long send_file(int client_socket, const char *file_path);

#endif // FILE_TRANSFER_H
